/*
 *  Driver for NDS sound
 *  Copyright (C) 2006 Malcolm Parsons <malcolm.parsons@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License.
 *
 */
#include <sound/driver.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <sound/core.h>
#include <sound/initval.h>
#include <sound/pcm.h>
#include <asm/mach-types.h>

#include <asm/arch/fifo.h>

#define TIMER1_DATA	(*(volatile u16*)0x04000104)
#define TIMER2_DATA	(*(volatile u16*)0x04000108)
#define TIMER1_CR	(*(volatile u16*)0x04000106)
#define TIMER2_CR	(*(volatile u16*)0x0400010A)
#define TIMER_ENABLE	(1<<7)
#define TIMER_IRQ_REQ	(1<<6)
#define TIMER_CASCADE	(TIMER_ENABLE|(1<<2))

#define DMA_BUFFERSIZE	(128*1024)
#define DMA_BUFFERSIZE_CAPTURE (64*1024)

/* module parameters (see "Module Parameters") */
static int index[SNDRV_CARDS] = SNDRV_DEFAULT_IDX;
static char *id[SNDRV_CARDS] = SNDRV_DEFAULT_STR;
static int enable[SNDRV_CARDS] = SNDRV_DEFAULT_ENABLE_PNP;

/* DMA-capable sample buffer */
/* This buffer is aligned to the ARM9 cache lines */
static char samplebuf[DMA_BUFFERSIZE] __attribute__ ((aligned (32)));
static char capturebuf[DMA_BUFFERSIZE_CAPTURE] __attribute__ ((aligned (32)));


MODULE_AUTHOR("Malcolm Parsons <malcolm.parsons@gmail.com>");
MODULE_LICENSE("GPL");

/* definition of the chip-specific record */
struct nds {
	snd_card_t *card;
	spinlock_t lock;
	snd_pcm_t *pcm;
	snd_pcm_substream_t *substream;
	size_t buffer_size;
	size_t period_size;
	u8 period;
};

/* hardware definition */
static snd_pcm_hardware_t snd_nds_playback_hw = {
	.info = (SNDRV_PCM_INFO_MMAP |
		 SNDRV_PCM_INFO_NONINTERLEAVED |
		 SNDRV_PCM_INFO_BLOCK_TRANSFER | SNDRV_PCM_INFO_MMAP_VALID),
	.formats = SNDRV_PCM_FMTBIT_S8 |
	    SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_IMA_ADPCM,
	.rates = SNDRV_PCM_RATE_8000_48000 | SNDRV_PCM_RATE_CONTINUOUS,
	.rate_min = 8000,
	.rate_max = 48000,
	.channels_min = 1,
	.channels_max = 2,
	.buffer_bytes_max = DMA_BUFFERSIZE,
	.period_bytes_min = DMA_BUFFERSIZE/32,
	.period_bytes_max = DMA_BUFFERSIZE/8,
	.periods_min = 8,
	.periods_max = 32,
};

/* hardware definition */
static snd_pcm_hardware_t snd_nds_capture_hw = {
	.info = (SNDRV_PCM_INFO_MMAP |
		 SNDRV_PCM_INFO_NONINTERLEAVED |
		 SNDRV_PCM_INFO_BLOCK_TRANSFER | SNDRV_PCM_INFO_MMAP_VALID),
	.formats =  SNDRV_PCM_FMTBIT_U8 | 
				SNDRV_PCM_FMTBIT_S8 | 
				SNDRV_PCM_FMTBIT_U16_LE |
				SNDRV_PCM_FMTBIT_S16_LE,
	.rates = SNDRV_PCM_RATE_8000_48000,
	.rate_min = 8000,
	.rate_max = 48000,
	.channels_min = 1,
	.channels_max = 1,
	.buffer_bytes_max = DMA_BUFFERSIZE_CAPTURE,
	.period_bytes_min = DMA_BUFFERSIZE_CAPTURE/16,
	.period_bytes_max = DMA_BUFFERSIZE_CAPTURE/8,
	.periods_min = 1,
	.periods_max = 1024,
};

static struct nds *capture_chip;  //Global reference for capture to deal with the FIFO signals.



/* Set the sample format */
void nds_playback_set_sample_format(struct nds *chip, snd_pcm_format_t format)
{
	switch (format) {
	case SNDRV_PCM_FORMAT_S8:
		nds_fifo_send(FIFO_SOUND | FIFO_SOUND_FORMAT | 0);
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
		nds_fifo_send(FIFO_SOUND | FIFO_SOUND_FORMAT | 1);
		break;
	case SNDRV_PCM_FORMAT_IMA_ADPCM:
		nds_fifo_send(FIFO_SOUND | FIFO_SOUND_FORMAT | 2);
		break;
	default:
		break;
	}
}

void nds_capture_set_sample_format(struct nds *chip, snd_pcm_format_t format)
{
	switch (format)
	{
		case SNDRV_PCM_FORMAT_U8:
			nds_fifo_send(FIFO_MIC | FIFO_MIC_FORMAT | MIC_FORMAT_U8);
			break;
		case SNDRV_PCM_FORMAT_S8:
			nds_fifo_send(FIFO_MIC | FIFO_MIC_FORMAT | MIC_FORMAT_S8);
			break;
		case SNDRV_PCM_FORMAT_U16_LE:
			nds_fifo_send(FIFO_MIC | FIFO_MIC_FORMAT | MIC_FORMAT_U16);
			break;
		case SNDRV_PCM_FORMAT_S16_LE:
			nds_fifo_send(FIFO_MIC | FIFO_MIC_FORMAT | MIC_FORMAT_S16);
			break;
		default:
			snd_printk("nds_capture_set_sample_format(): Unrecognized Format: %d\n", format);
	}
}

/* Set the sample rate */
void nds_playback_set_sample_rate(struct nds *chip, unsigned int rate)
{
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_RATE | rate);
}

void nds_capture_set_sample_rate(struct nds *chip, unsigned int rate)
{
	nds_fifo_send(FIFO_MIC | FIFO_MIC_RATE | rate);
}

/* Set the number of channels */
void nds_playback_set_channels(struct nds *chip, unsigned int channels)
{
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_CHANNELS | channels);
}

/* Setup the DMA */
void nds_playback_set_dma_setup(struct nds *chip, unsigned char *dma_area,
		       size_t buffer_size, size_t period_size)
{
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_DMA_SIZE | buffer_size);
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_DMA_ADDRESS |
		(((u32) dma_area) & 0xffffff));
}

void nds_capture_set_dma_setup(struct nds *chip, unsigned char *dma_area,
			       size_t buffer_size, size_t period_size)
{
	nds_fifo_send(FIFO_MIC | FIFO_MIC_DMA_SIZE | buffer_size);
	nds_fifo_send(FIFO_MIC | FIFO_MIC_DMA_ADDRESS |
		(((u32) dma_area) & 0xffffff));
	nds_fifo_send(FIFO_MIC | FIFO_MIC_PERIOD_SIZE | period_size);
}

/* open callback */
static int snd_nds_playback_open(snd_pcm_substream_t * substream)
{
	struct nds *chip = snd_pcm_substream_chip(substream);
	snd_pcm_runtime_t *runtime = substream->runtime;

	runtime->hw = snd_nds_playback_hw;

	spin_lock(&chip->lock);

	chip->substream = substream;

	// turn the power on
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_POWER | 1);

	spin_unlock(&chip->lock);

	return 0;
}

/* close callback */
static int snd_nds_playback_close(snd_pcm_substream_t * substream)
{
	// turn the power off
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_POWER | 0);
	TIMER1_CR = 0;
	TIMER2_CR = 0;

	return 0;
}

/* open callback */
static int snd_nds_capture_open(snd_pcm_substream_t * substream)
{
	struct nds *chip = snd_pcm_substream_chip(substream);

	snd_pcm_runtime_t *runtime = substream->runtime;

	runtime->hw = snd_nds_capture_hw;
	spin_lock(&chip -> lock);

	chip -> substream = substream;

	//turn on the mic power.
	nds_fifo_send(FIFO_MIC | FIFO_MIC_POWER | 1);

	spin_unlock(&chip -> lock);
	return 0;
}

/* close callback */
static int snd_nds_capture_close(snd_pcm_substream_t * substream) 
{
	// turn off the power to the mic.
	nds_fifo_send(FIFO_MIC | FIFO_MIC_POWER | 0);
	capture_chip = NULL;
	return 0;
}

/* hw_params callback */
static int snd_nds_pcm_hw_params(snd_pcm_substream_t * substream,
				 snd_pcm_hw_params_t * hw_params)
{
	return snd_pcm_lib_malloc_pages(substream,
					params_buffer_bytes(hw_params));
}

/* hw_free callback */
static int snd_nds_pcm_hw_free(snd_pcm_substream_t * substream)
{
	return snd_pcm_lib_free_pages(substream);
}

/* prepare callback */
static int snd_nds_playback_prepare(snd_pcm_substream_t * substream)
{
	struct nds *chip = snd_pcm_substream_chip(substream);
	snd_pcm_runtime_t *runtime = substream->runtime;
	/* set up the hardware with the current configuration
	 * for example...
	 */

	chip->buffer_size = snd_pcm_lib_buffer_bytes(substream);
	chip->period_size = snd_pcm_lib_period_bytes(substream);

	TIMER1_CR = 0;
	TIMER2_CR = 0;
	/* use exactly the same formula as for ARM7, to get the
	   same period regarding rounding errors */
	TIMER1_DATA = 0 - ((0x1000000 / runtime->rate)*2);
	switch (runtime->format) {
	case SNDRV_PCM_FORMAT_S8:
		TIMER2_DATA = 0 - (chip->period_size / runtime->channels);
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
		TIMER2_DATA = 0 - ((chip->period_size / 2) / runtime->channels);
		break;
	case SNDRV_PCM_FORMAT_IMA_ADPCM:
		TIMER2_DATA = 0 - ((chip->period_size * 2) / runtime->channels);
		break;
	default:
		break;
	}

	nds_playback_set_channels(chip, runtime->channels);
	nds_playback_set_sample_format(chip, runtime->format);
	nds_playback_set_sample_rate(chip, runtime->rate);
	nds_playback_set_dma_setup(chip, runtime->dma_area,
			  chip->buffer_size, chip->period_size);
	chip->period = 0;

	return 0;
}

/****************************************
 *snd_nds_capture_prepare:
 ****************************************/
static int snd_nds_capture_prepare(snd_pcm_substream_t * substream)
{
	struct nds *chip = snd_pcm_substream_chip(substream);
	snd_pcm_runtime_t *runtime = substream->runtime;

	chip->buffer_size = snd_pcm_lib_buffer_bytes(substream);
	chip->period_size = snd_pcm_lib_period_bytes(substream);

	nds_capture_set_sample_format(chip, runtime->format);
	nds_capture_set_sample_rate(chip, runtime->rate);
	nds_capture_set_dma_setup(chip, capturebuf,
				   chip->buffer_size, chip->period_size);
	chip->period = 0;

	//pointer to the chip so we can log the period that are finished by the arm7.
	capture_chip = chip;
	return 0;
}



/****************************************
 *snd_nds_playback_trigger()
 ****************************************/
static int snd_nds_playback_trigger(snd_pcm_substream_t * substream, int cmd)
{
	snd_pcm_runtime_t *runtime = substream->runtime;

	switch (cmd) 
	{
	case SNDRV_PCM_TRIGGER_START:
		// start the PCM engine
		nds_fifo_send(FIFO_SOUND | FIFO_SOUND_TRIGGER | 1);

		/* wait until we know for shure that the ARM7 has 
		   started the sound output */
		
		/* send a dummy command */
		nds_fifo_send(FIFO_SOUND | FIFO_SOUND_CHANNELS | runtime->channels);

		/* wait until FIFO is empty */
		while (!(NDS_REG_IPCFIFOCNT & FIFO_EMPTY))
			;

		/* now we can start the timer and be shure that
	       the timer interrupt is not comming to early. */
		TIMER1_CR = TIMER_ENABLE;
		TIMER2_CR = TIMER_CASCADE | TIMER_IRQ_REQ;
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		// stop the PCM engine
		nds_fifo_send(FIFO_SOUND | FIFO_SOUND_TRIGGER | 0);
		TIMER1_CR = 0;
		TIMER2_CR = 0;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/*************************************
 *snd_nds_capture_trigger()
 *************************************/
static int snd_nds_capture_trigger(snd_pcm_substream_t * substream, int cmd)
{

	switch (cmd) 
	{
	case SNDRV_PCM_TRIGGER_START:
		// start the PCM engine
		nds_fifo_send(FIFO_MIC | FIFO_MIC_TRIGGER | 1);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		// stop the PCM engine
		nds_fifo_send(FIFO_MIC | FIFO_MIC_TRIGGER | 0);
		capture_chip = NULL;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/* pointer callback */
static snd_pcm_uframes_t snd_nds_pcm_pointer(snd_pcm_substream_t * substream)
{
	struct nds *chip = snd_pcm_substream_chip(substream);
	snd_pcm_runtime_t *runtime = substream->runtime;

	/* fake the current hardware pointer */
	return chip->period * runtime->period_size;
}

/*******************************
 *snd_nds_capture_copy()
 *******************************/
static int snd_nds_capture_copy(snd_pcm_substream_t *substream, int channel,
			snd_pcm_uframes_t pos, void *dst, snd_pcm_uframes_t count)
{
	snd_pcm_runtime_t *runtime = substream->runtime;
	char *cdst = (char*)dst;
	int cpos;
	int ccount;
	int i;
	switch (runtime -> format) 
	{
	default:
	case SNDRV_PCM_FORMAT_U8:
	case SNDRV_PCM_FORMAT_S8:
		cpos = pos;
		ccount = count;
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
	case SNDRV_PCM_FORMAT_U16_LE:
		cpos = pos * 2;
		ccount = count * 2;
		break;
	}
	memcpy(dst, &capturebuf[cpos], ccount);

	return 0;
}

/* operators */
static snd_pcm_ops_t snd_nds_playback_ops = {
	.open = snd_nds_playback_open,
	.close = snd_nds_playback_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = snd_nds_pcm_hw_params,
	.hw_free = snd_nds_pcm_hw_free,
	.prepare = snd_nds_playback_prepare,
	.trigger = snd_nds_playback_trigger,
	.pointer = snd_nds_pcm_pointer,
};

/* operators */
static snd_pcm_ops_t snd_nds_capture_ops = {
	.open = snd_nds_capture_open,
	.close = snd_nds_capture_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = snd_nds_pcm_hw_params,
	.hw_free = snd_nds_pcm_hw_free,
	.prepare = snd_nds_capture_prepare,
	.trigger = snd_nds_capture_trigger,
	.pointer = snd_nds_pcm_pointer,
	.copy = snd_nds_capture_copy,
};


/* create a pcm device */
static int __devinit snd_nds_new_pcm(struct nds *chip)
{
	snd_pcm_t *pcm;
	snd_pcm_substream_t *substream;
	int err;

	if ((err = snd_pcm_new(chip->card, "NDS", 0, 1, 1, &pcm)) < 0)
		return err;

	pcm->private_data = chip;
	strcpy(pcm->name, "NDS");
	chip->pcm = pcm;
	/* set operators */
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &snd_nds_playback_ops);
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, &snd_nds_capture_ops);

	/* set the DMA buffer for the playback substream */
	substream = pcm->streams[0].substream;
	substream->dma_buffer.dev.type = SNDRV_DMA_TYPE_CONTINUOUS;
	substream->dma_buffer.dev.dev  = snd_dma_continuous_data(GFP_DMA);
	substream->dma_buffer.bytes    = DMA_BUFFERSIZE;
	substream->dma_buffer.area     = samplebuf;
	substream->dma_buffer.addr     = (unsigned long)samplebuf;
	substream->buffer_bytes_max    = DMA_BUFFERSIZE;
	substream->dma_max             = DMA_BUFFERSIZE;

	/* set the DMA buffer for the capture substream */
	substream = pcm -> streams[1].substream;
	substream -> dma_buffer.dev.type = SNDRV_DMA_TYPE_CONTINUOUS;
	substream -> dma_buffer.dev.dev  = snd_dma_continuous_data(GFP_DMA);
	substream -> dma_buffer.bytes    = DMA_BUFFERSIZE_CAPTURE;
	substream -> dma_buffer.area     = capturebuf;
	substream -> dma_buffer.addr     = (unsigned long)capturebuf;
	substream -> buffer_bytes_max    = DMA_BUFFERSIZE_CAPTURE;
	substream -> dma_max             = DMA_BUFFERSIZE_CAPTURE;

	return 0;
}

/*************************************
 *nds_mic_handler(): Handles the HAVE_DATA signals from the arm7 when recording microphone data.
 *************************************/
static void nds_mic_handler(void) 
{
	struct nds *chip = capture_chip;
	snd_pcm_substream_t *substream;
	snd_pcm_runtime_t *runtime;
	u8 period;
	if (chip)
	{
		substream = chip->substream;
		runtime = substream->runtime;

		spin_lock(&chip->lock);

		period = chip->period + 1;
		if (period == runtime->periods)
			period = 0;
		chip->period = period;

		spin_unlock(&chip->lock);

		/* call updater, unlock before it */
		snd_pcm_period_elapsed(chip->substream);
	}
}

static struct fifo_cb nds_mic_fifocb = {
	.type = FIFO_MIC,
	.handler.mic_handler = nds_mic_handler
};

/*****************************************
 *snd_nds_interrupt() : Handles the interrupt from the timer for sound
 *****************************************/
static irqreturn_t snd_nds_interrupt(int irq, void *dev_id,
				     struct pt_regs *regs)
{
	struct nds *chip = dev_id;
	snd_pcm_substream_t *substream = chip->substream;
	snd_pcm_runtime_t *runtime = substream->runtime;
    	u8 period ;

	spin_lock(&chip->lock);

	period = chip->period + 1;
	if (period == runtime->periods)
		period = 0;
    	chip->period = period;

	spin_unlock(&chip->lock);

	/* call updater, unlock before it */
	snd_pcm_period_elapsed(chip->substream);

	return IRQ_HANDLED;
}

/* chip-specific destructor
 * (see "PCI Resource Managements")
 */
static int snd_nds_free(struct nds *chip)
{
	// turn the power off
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_POWER | 0);
	nds_fifo_send(FIFO_MIC | FIFO_MIC_POWER | 0);
	kfree(chip);
	return 0;
}

/* component-destructor
 * (see "Management of Cards and Components")
 */
static int snd_nds_dev_free(snd_device_t * device)
{
	return snd_nds_free(device->device_data);
}

/* chip-specific constructor
 * (see "Management of Cards and Components")
 */
static int __devinit snd_nds_create(snd_card_t * card, struct nds **rchip)
{
	struct nds *chip;
	int err;
	static snd_device_ops_t ops = {
		.dev_free = snd_nds_dev_free,
	};

	*rchip = NULL;

	/* allocate a chip-specific data with zero filled */
#if 0
	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
#endif
	chip = kcalloc(1, sizeof(*chip), GFP_KERNEL);
	if (chip == NULL)
		return -ENOMEM;
	chip->card = card;
	spin_lock_init(&chip->lock);

	register_fifocb(&nds_mic_fifocb);

	if (request_irq(IRQ_TC2, snd_nds_interrupt,
			SA_INTERRUPT, "NDS sound", chip)) {
		printk(KERN_ERR "cannot grab irq %d\n", IRQ_TC2);
		snd_nds_free(chip);
		return -EBUSY;
	}

	if ((err = snd_device_new(card, SNDRV_DEV_LOWLEVEL, chip, &ops)) < 0) {
		snd_nds_free(chip);
		return err;
	}

	*rchip = chip;
	return 0;
}


/* constructor -- see "Constructor" sub-section */
static int __init snd_nds_init(void)
{
	static int dev;
	snd_card_t *card;
	struct nds *chip;
	int err;

	/* (0) */
	if (!machine_is_nds())
		return -ENODEV;

	/* (1) */
	if (dev >= SNDRV_CARDS)
		return -ENODEV;
	if (!enable[dev]) {
		dev++;
		return -ENOENT;
	}
	/* (2) */
	card = snd_card_new(index[dev], id[dev], THIS_MODULE, 0);
	if (card == NULL)
		return -ENOMEM;
	/* (3) */
	if ((err = snd_nds_create(card, &chip)) < 0) {
		snd_card_free(card);
		return err;
	}
	/* (4) */
	strcpy(card->driver, "nds");
	strcpy(card->shortname, "NDS sound");
	sprintf(card->longname, "Nintendo DS sound");
	/* (5) */
	if ((err = snd_nds_new_pcm(chip)) < 0) {
		snd_card_free(card);
		return err;
	}
	/* (6) */
	if ((err = snd_card_register(card)) < 0) {
		snd_card_free(card);
		return err;
	}
	/* (7) */
	dev++;
	return 0;
}

/* destructor -- see "Destructor" sub-section */
static void __exit snd_nds_exit(void)
{
	//snd_card_free(nds_sound->card);
	unregister_fifocb(&nds_mic_fifocb);
}

module_init(snd_nds_init);
module_exit(snd_nds_exit);
