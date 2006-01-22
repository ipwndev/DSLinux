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

/* chip-specific destructor
 * (see "PCI Resource Managements")
 */
static int snd_nds_free(struct nds *chip)
{
        .... // will be implemented later...
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
                                       struct pci_dev *pci,
                                       struct nds **rchip)
{
        struct nds *chip;
        int err;
        static struct snd_device_ops ops = {
               .dev_free = snd_nds_dev_free,
        };

        *rchip = NULL;
        // check PCI availability here
        // (see "PCI Resource Managements")
        ....
        /* allocate a chip-specific data with zero filled */
        chip = kzalloc(sizeof(*chip), GFP_KERNEL);
        if (chip == NULL)
                return -ENOMEM;
        chip->card = card;
        // rest of initialization here; will be implemented
        // later, see "PCI Resource Managements"
        ....
        if ((err = snd_device_new(card, SNDRV_DEV_LOWLEVEL,
                                  chip, &ops)) < 0) {
                snd_nds_free(chip);
                return err;
        }
        snd_card_set_dev(card, &pci->dev);
        *rchip = chip;
        return 0;
}
/* constructor -- see "Constructor" sub-section */
static int __init nds_sound_init(void)
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
        if ((err = snd_nds_create(card, pci, &chip)) < 0) {
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

