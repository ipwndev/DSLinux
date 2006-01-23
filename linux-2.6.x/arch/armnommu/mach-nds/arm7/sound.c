#include "asm/types.h"

#include "sound.h"

/* some sound defines */
#define SOUND_VOL(n)	(n)
#define SOUND_FREQ(n)	((-0x1000000 / (n)))
#define SOUND_ENABLE	(1<<15)
#define SOUND_REPEAT    (1<<27)
#define SOUND_ONE_SHOT  (1<<28)
#define SOUND_FORMAT_16BIT (1<<29)
#define SOUND_FORMAT_8BIT	(0<<29)
#define SOUND_FORMAT_PSG    (3<<29)
#define SOUND_FORMAT_ADPCM  (2<<29)
#define SOUND_16BIT      (1<<29)
#define SOUND_8BIT       (0)
#define SOUND_PAN(n)	 (n)
#define SCHANNEL_TIMER(n)			(*(volatile u16*)(0x04000408 + ((n)<<4)))
#define SCHANNEL_CR(n)				(*(volatile u32*)(0x04000400 + ((n)<<4)))
#define SCHANNEL_VOL(n)				(*(volatile u8*)(0x04000400 + ((n)<<4)))
#define SCHANNEL_PAN(n)				(*(volatile u8*)(0x04000402 + ((n)<<4)))
#define SCHANNEL_SOURCE(n)			(*(volatile u32*)(0x04000404 + ((n)<<4)))
#define SOUND_MASTER_VOL  (*(volatile u8*)0x04000500)
#define POWER_CR       (*(volatile u16*)0x04000304)

/* Plays sounds from the specified buffer */
void playSound(u32 buffer)
{
	SCHANNEL_SOURCE(0) = buffer;
	SCHANNEL_TIMER(0) = SOUND_FREQ(44100);
	SCHANNEL_PAN(0) = SOUND_PAN(64);
	SCHANNEL_VOL(0) = SOUND_VOL(128);
	SCHANNEL_CR(0) = SOUND_ENABLE | SOUND_ONE_SHOT | SOUND_FORMAT_16BIT | SOUND_16BIT;
};

/* Set the master volume */
void SetMasterVol(int vol)
{
	SOUND_MASTER_VOL = SOUND_VOL(vol);
};

/* Turn on/off sound 1 = on 0 = off */
void PowerSnd(int state)
{
	if( state ) {
		POWER_CR |= 1;
	} else {
		POWER_CR &= ~1;
	};
};
