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

/* module parameters (see "Module Parameters") */
static int index[SNDRV_CARDS] = SNDRV_DEFAULT_IDX;
static char *id[SNDRV_CARDS] = SNDRV_DEFAULT_STR;
static int enable[SNDRV_CARDS] = SNDRV_DEFAULT_ENABLE_PNP;

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
	.buffer_bytes_max = 32768,
	.period_bytes_min = 512,
	.period_bytes_max = 4096,
	.periods_min = 1,
	.periods_max = 1024,
};

/* hardware definition */
static snd_pcm_hardware_t snd_nds_capture_hw = {
	.info = (SNDRV_PCM_INFO_MMAP |
		 SNDRV_PCM_INFO_NONINTERLEAVED |
		 SNDRV_PCM_INFO_BLOCK_TRANSFER | SNDRV_PCM_INFO_MMAP_VALID),
	.formats = SNDRV_PCM_FMTBIT_U8 | SNDRV_PCM_FMTBIT_U16_LE,
	.rates = SNDRV_PCM_RATE_8000_48000,
	.rate_min = 8000,
	.rate_max = 48000,
	.channels_min = 1,
	.channels_max = 1,
	.buffer_bytes_max = 32768,
	.period_bytes_min = 512,
	.period_bytes_max = 4096,
	.periods_min = 1,
	.periods_max = 1024,
};

/* Set the sample format */
void nds_set_sample_format(struct nds *chip, snd_pcm_format_t format)
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

/* Set the sample rate */
void nds_set_sample_rate(struct nds *chip, unsigned int rate)
{
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_RATE | rate);
}

/* Set the number of channels */
void nds_set_channels(struct nds *chip, unsigned int channels)
{
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_CHANNELS | channels);
}

/* Setup the DMA */
void nds_set_dma_setup(struct nds *chip, unsigned char *dma_area,
		       size_t buffer_size, size_t period_size)
{
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_DMA_SIZE | buffer_size);
	nds_fifo_send(FIFO_SOUND | FIFO_SOUND_DMA_ADDRESS |
	    (((u32) dma_area) & 0xffffff));

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
	struct nds *chip = snd_pcm_substream_chip(substream);

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
	// more hardware-initialization will be done here
	return 0;
}

/* close callback */
static int snd_nds_capture_close(snd_pcm_substream_t * substream)
{
	struct nds *chip = snd_pcm_substream_chip(substream);
	// the hardware-specific codes will be here
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
static int snd_nds_pcm_prepare(snd_pcm_substream_t * substream)
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
	TIMER1_DATA = (-0x2000000/ (s32)(runtime->rate));
	switch (runtime->format) {
	case SNDRV_PCM_FORMAT_S8:
		TIMER2_DATA = -(chip->period_size);
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
		TIMER2_DATA = -(chip->period_size / 2);
		break;
	case SNDRV_PCM_FORMAT_IMA_ADPCM:
		TIMER2_DATA = -(chip->period_size * 2);
		break;
	default:
		break;
	}

	nds_set_channels(chip, runtime->channels);
	nds_set_sample_format(chip, runtime->format);
	nds_set_sample_rate(chip, runtime->rate);
	nds_set_dma_setup(chip, runtime->dma_area,
			  chip->buffer_size, chip->period_size);
	chip->period = 0;

	return 0;
}

/* trigger callback */
static int snd_nds_pcm_trigger(snd_pcm_substream_t * substream, int cmd)
{
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
		// start the PCM engine
		nds_fifo_send(FIFO_SOUND | FIFO_SOUND_TRIGGER | 1);

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

/* pointer callback */
static snd_pcm_uframes_t snd_nds_pcm_pointer(snd_pcm_substream_t * substream)
{
	struct nds *chip = snd_pcm_substream_chip(substream);
	unsigned int current_ptr;
	snd_pcm_runtime_t *runtime = substream->runtime;

	/* fake the current hardware pointer */
	return chip->period * runtime->period_size;
}

/* operators */
static snd_pcm_ops_t snd_nds_playback_ops = {
	.open = snd_nds_playback_open,
	.close = snd_nds_playback_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = snd_nds_pcm_hw_params,
	.hw_free = snd_nds_pcm_hw_free,
	.prepare = snd_nds_pcm_prepare,
	.trigger = snd_nds_pcm_trigger,
	.pointer = snd_nds_pcm_pointer,
};

/* operators */
static snd_pcm_ops_t snd_nds_capture_ops = {
	.open = snd_nds_capture_open,
	.close = snd_nds_capture_close,
	.ioctl = snd_pcm_lib_ioctl,
	.hw_params = snd_nds_pcm_hw_params,
	.hw_free = snd_nds_pcm_hw_free,
	.prepare = snd_nds_pcm_prepare,
	.trigger = snd_nds_pcm_trigger,
	.pointer = snd_nds_pcm_pointer,
};

/*
 * definitions of capture are omitted here...
 */
/* create a pcm device */
static int __devinit snd_nds_new_pcm(struct nds *chip)
{
	snd_pcm_t *pcm;
	int err;
	if ((err = snd_pcm_new(chip->card, "NDS", 0, 1, 1, &pcm)) < 0)
		return err;
	pcm->private_data = chip;
	strcpy(pcm->name, "NDS");
	chip->pcm = pcm;
	/* set operators */
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &snd_nds_playback_ops);
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE, &snd_nds_capture_ops);
	/* pre-allocation of buffers */
	/* NOTE: this may fail */
	//return snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_DEV, snd_dma_isa_data(),	/*pretend to be an ISA device */
	return snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_CONTINUOUS, snd_dma_continuous_data(GFP_KERNEL),
					      32 * 1024, 64 * 1024);
}

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

	// TODO: register with FIFO or IPC here

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
}

module_init(snd_nds_init);
module_exit(snd_nds_exit);
