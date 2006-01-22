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
#include <sound/core.h>
#include <sound/initval.h>
#include <sound/pcm.h>

/* module parameters (see "Module Parameters") */
static int index[SNDRV_CARDS] = SNDRV_DEFAULT_IDX;
static char *id[SNDRV_CARDS] = SNDRV_DEFAULT_STR;
static int enable[SNDRV_CARDS] = SNDRV_DEFAULT_ENABLE_PNP;

MODULE_AUTHOR("Malcolm Parsons <malcolm.parsons@gmail.com>");
MODULE_LICENSE("GPL");

/* definition of the chip-specific record */
struct nds {
        struct snd_card *card;
        // rest of implementation will be in the section
        // "PCI Resource Managements"
};

/* hardware definition */
static struct snd_pcm_hardware snd_nds_playback_hw = {
        .info = (SNDRV_PCM_INFO_MMAP |
                 SNDRV_PCM_INFO_NONINTERLEAVED |
                 SNDRV_PCM_INFO_BLOCK_TRANSFER |
                 SNDRV_PCM_INFO_MMAP_VALID),
        .formats =          SNDRV_PCM_FMTBIT_S16_LE,
        .rates =            SNDRV_PCM_RATE_8000_48000,
        .rate_min =         8000,
        .rate_max =         48000,
        .channels_min =     2,
        .channels_max =     2,
        .buffer_bytes_max = 32768,
        .period_bytes_min = 4096,
        .period_bytes_max = 32768,
        .periods_min =      1,
        .periods_max =      1024,
};
/* hardware definition */
static struct snd_pcm_hardware snd_nds_capture_hw = {
        .info = (SNDRV_PCM_INFO_MMAP |
                 SNDRV_PCM_INFO_INTERLEAVED |
                 SNDRV_PCM_INFO_BLOCK_TRANSFER |
                 SNDRV_PCM_INFO_MMAP_VALID),
        .formats =          SNDRV_PCM_FMTBIT_S16_LE,
        .rates =            SNDRV_PCM_RATE_8000_48000,
        .rate_min =         8000,
        .rate_max =         48000,
        .channels_min =     1,
        .channels_max =     1,
        .buffer_bytes_max = 32768,
        .period_bytes_min = 4096,
        .period_bytes_max = 32768,
        .periods_min =      1,
        .periods_max =      1024,
};
/* open callback */
static int snd_nds_playback_open(struct snd_pcm_substream *substream)
{
        struct nds *chip = snd_pcm_substream_chip(substream);
        struct snd_pcm_runtime *runtime = substream->runtime;
        runtime->hw = snd_nds_playback_hw;
        // more hardware-initialization will be done here
        return 0;
}
/* close callback */
static int snd_nds_playback_close(struct snd_pcm_substream *substream)
{
        struct nds *chip = snd_pcm_substream_chip(substream);
        // the hardware-specific codes will be here
        return 0;
}
/* open callback */
static int snd_nds_capture_open(struct snd_pcm_substream *substream)
{
        struct nds *chip = snd_pcm_substream_chip(substream);
        struct snd_pcm_runtime *runtime = substream->runtime;
        runtime->hw = snd_nds_capture_hw;
        // more hardware-initialization will be done here
        return 0;
}
/* close callback */
static int snd_nds_capture_close(struct snd_pcm_substream *substream)
{
        struct nds *chip = snd_pcm_substream_chip(substream);
        // the hardware-specific codes will be here
        return 0;
}
/* hw_params callback */
static int snd_nds_pcm_hw_params(struct snd_pcm_substream *substream,
                             struct snd_pcm_hw_params *hw_params)
{
        return snd_pcm_lib_malloc_pages(substream,
                                   params_buffer_bytes(hw_params));
}
/* hw_free callback */
static int snd_nds_pcm_hw_free(struct snd_pcm_substream *substream)
{
        return snd_pcm_lib_free_pages(substream);
}
/* prepare callback */
static int snd_nds_pcm_prepare(struct snd_pcm_substream *substream)
{
        struct nds *chip = snd_pcm_substream_chip(substream);
        struct snd_pcm_runtime *runtime = substream->runtime;
        /* set up the hardware with the current configuration
         * for example...
         */
        nds_set_sample_format(chip, runtime->format);
        nds_set_sample_rate(chip, runtime->rate);
        nds_set_channels(chip, runtime->channels);
        nds_set_dma_setup(chip, runtime->dma_area,
                             chip->buffer_size,
                             chip->period_size);
        return 0;
}
/* trigger callback */
static int snd_nds_pcm_trigger(struct snd_pcm_substream *substream,
                                  int cmd)
{
        switch (cmd) {
        case SNDRV_PCM_TRIGGER_START:
                // do something to start the PCM engine
                break;
        case SNDRV_PCM_TRIGGER_STOP:
                // do something to stop the PCM engine
                break;
        default:
                return -EINVAL;
        }
}
/* pointer callback */
static snd_pcm_uframes_t
snd_nds_pcm_pointer(struct snd_pcm_substream *substream)
{
        struct nds *chip = snd_pcm_substream_chip(substream);
        unsigned int current_ptr;
        /* get the current hardware pointer */
        current_ptr = nds_get_hw_pointer(chip);
        return current_ptr;
}
/* operators */
static struct snd_pcm_ops snd_nds_playback_ops = {
        .open =        snd_nds_playback_open,
        .close =       snd_nds_playback_close,
        .ioctl =       snd_pcm_lib_ioctl,
        .hw_params =   snd_nds_pcm_hw_params,
        .hw_free =     snd_nds_pcm_hw_free,
        .prepare =     snd_nds_pcm_prepare,
        .trigger =     snd_nds_pcm_trigger,
        .pointer =     snd_nds_pcm_pointer,
};
/* operators */
static struct snd_pcm_ops snd_nds_capture_ops = {
        .open =        snd_nds_capture_open,
        .close =       snd_nds_capture_close,
        .ioctl =       snd_pcm_lib_ioctl,
        .hw_params =   snd_nds_pcm_hw_params,
        .hw_free =     snd_nds_pcm_hw_free,
        .prepare =     snd_nds_pcm_prepare,
        .trigger =     snd_nds_pcm_trigger,
        .pointer =     snd_nds_pcm_pointer,
};
/*
 * definitions of capture are omitted here...
 */
/* create a pcm device */
static int __devinit snd_nds_new_pcm(struct nds *chip)
{
	struct snd_pcm *pcm;
	int err;
	if ((err = snd_pcm_new(chip->card, "NDS", 0, 1, 1,
			       &pcm)) < 0)
		return err;
	pcm->private_data = chip;
	strcpy(pcm->name, "NDS");
	chip->pcm = pcm;
	/* set operators */
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK,
			&snd_nds_playback_ops);
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_CAPTURE,
			&snd_nds_capture_ops);
	/* pre-allocation of buffers */
	/* NOTE: this may fail */
	snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_DEV,
					      snd_dma_isa_data(),/*pretend to be an ISA device*/
					      64*1024, 64*1024);
	return 0;
}


/* chip-specific destructor
 * (see "PCI Resource Managements")
 */
static int snd_nds_free(struct nds *chip)
{
	// TODO: stop sound hardware here
	kfree(chip);
}

/* component-destructor
 * (see "Management of Cards and Components")
 */
static int snd_nds_dev_free(struct snd_device *device)
{
        return snd_nds_free(device->device_data);
}

/* chip-specific constructor
 * (see "Management of Cards and Components")
 */
static int __devinit snd_nds_create(struct snd_card *card,
                                       struct nds **rchip)
{
        struct nds *chip;
        int err;
        static struct snd_device_ops ops = {
               .dev_free = snd_nds_dev_free,
        };

        *rchip = NULL;

        /* allocate a chip-specific data with zero filled */
        chip = kzalloc(sizeof(*chip), GFP_KERNEL);
        if (chip == NULL)
                return -ENOMEM;
        chip->card = card;

	// TODO: register with FIFO or IPC here

        if ((err = snd_device_new(card, SNDRV_DEV_LOWLEVEL,
                                  chip, &ops)) < 0) {
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
        struct snd_card *card;
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
        strcpy(card->shortname, "nds audio");
        sprintf(card->longname, "Nintendo DS audio");
        /* (5) */
        .... // implemented later
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
	snd_card_free(nds_sound->card);
}



module_init(snd_nds_init);
module_exit(snd_nds_exit);

