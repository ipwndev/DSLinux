/****************************************************************************/

/*
 *	gba_audio.c -- audio driver for Gameboy Advance internal audio.
 *
 *	(C) Copyright 2004, Wolfram Miksch (wolfram.miksch@fh-hagenberg.at)
 */

/****************************************************************************/

//#include "gba.h"
//#include "i.c"

#include <linux/config.h>
#include <linux/major.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/soundcard.h>

/****************************************************************************/
/*
 *	Driver settings and state
 */
int	gba_audio_isopen;

/****************************************************************************/
/*
 *	Prototypes
 */

//	Init
void gba_audio_init(void);

//	File operations
int gba_audio_open(struct inode *, struct file *);
void gba_audio_close(struct inode *, struct file *);
int gba_audio_write(struct inode *, struct file *, const char *, int);
int gba_audio_ioctl(struct inode *, struct file *,
			unsigned int, unsigned long);

/****************************************************************************/
/*
 *	File operations, which must be given in gba_audio_init() routine
 */

static struct file_operations gba_audio_fops = {
	open: gba_audio_open,
	release: gba_audio_close,
	write: gba_audio_write,
	ioctl: gba_audio_ioctl,
};

/****************************************************************************/
/*
 *	Function implementations
 */
 
/****************************************************************************/
/*
 *	gba_audio_open() -- defines playback defaults
 */

int gba_audio_open(struct inode *inode, struct file *file)
{
	if(gba_audio_isopen)
		return(-EBUSY);
	gba_audio_isopen = 1;
	return(0);
}

/****************************************************************************/
/*
 *	gba_audio_close() -- turn off sound and so on
 */
 
void gba_audio_close(struct inode *inode, struct file *file)
{
	gba_audio_isopen = 0;
}

/****************************************************************************/
/*
 *	gba_audio_write() -- shift audio data to the dma buffer
 */

int gba_audio_write(struct inode *inode, struct file *file,
			const char *buf, int count)
{
	int i;
	/*REG_SGCNT0_H = 256 | 512 | 2048;
	REG_SGCNT1 = 128;
	REG_DM1SAD = (u32)buf;
	REG_DM1DAD = 0x40000A0;
	REG_DM1CNT_H = 64 | 512 | 1024 | 4096 | 8192 | 32768;
	REG_TM0D = 65535 - (16777216/11025);
	REG_TM0CNT = 128;
*/
	*(unsigned short*)0x4000082 = 256 | 512 | 2048;
	*(unsigned short*)0x4000084 = 128;
	*(unsigned long*)0x40000BC = (unsigned long)buf;
	*(unsigned long*)0x40000C0 = 0x40000A0;
	*(unsigned short*)0x40000C6 = 64 | 512 | 1024 | 4096 | 8192 | 32768;
	*(unsigned short*)0x4000108 = 65535 - (16777216/11025);
	*(unsigned short*)0x400010A = 128;
	for(i=0;i<count/0x170;++i) {
		while(*(unsigned long*)0x4000006!=160);
		while(*(unsigned long*)0x4000006==160);
	}
	*(unsigned short*)0x40000C6 = 0;	
	return count;
}

/****************************************************************************/
/*
 *	gba_audio_ioctl() -- react on i/o messages
 */

 int gba_audio_ioctl(struct inode *inode, struct file *file,
 			unsigned int cmd, unsigned long arg)
{
	switch (cmd) {

	case SNDCTL_DSP_SPEED:
		break;
	
	case SNDCTL_DSP_SAMPLESIZE:
		break;
	
	case SNDCTL_DSP_CHANNELS:
		break;
	
	case SNDCTL_DSP_STEREO:
		break;
	
	case SNDCTL_DSP_GETBLKSIZE:
		break;
	
	case SNDCTL_DSP_SYNC:
		break;
	default:
		return(-EINVAL);
	
	}
	
	return(0);
}

/****************************************************************************/
/*
 *	gba_audio_init() -- registers the audio driver
 *	will be called in init/main.c from the init function
 */
 
void gba_audio_init()
{
	long i=0;
	printk("GBA_AUDIO: (C) Copyright 2004, "
  		"Wolfram Miksch (wolfram.miksch@fh-hagenberg.at)\n");

	gba_audio_isopen = 0;
	if (register_chrdev(SOUND_MAJOR, "sound", &gba_audio_fops) < 0) {
		printk(KERN_WARNING "SOUND: failed to register major %d\n",
			SOUND_MAJOR);
		return;
	}

/*	
  REG_SGCNT0_H = 256 | 512 | 2048;
  REG_SGCNT1 = 128;
  REG_DM1SAD = (u32)I_DATA;
  REG_DM1DAD = 0x40000A0;
  REG_DM1CNT_H = 64 | 512 | 1024 | 4096 | 8192 | 32768;
  REG_TM0D = 65535 - (16777216/I_SAMPRATE);
  REG_TM0CNT = 128;
  */
}

