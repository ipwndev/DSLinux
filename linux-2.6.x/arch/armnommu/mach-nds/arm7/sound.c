#include "asm/types.h"

#include "sound.h"

/* some sound defines */
#define SOUND_VOL(n)		(n)
#define SOUND_FREQ(n)		((-0x1000000 / (n)))
#define SOUND_ENABLE		(1<<15)
#define SOUND_REPEAT		(1<<27)
#define SOUND_ONE_SHOT		(1<<28)
#define SOUND_FORMAT_8BIT	(0<<29)
#define SOUND_FORMAT_16BIT	(1<<29)
#define SOUND_FORMAT_ADPCM	(2<<29)
#define SOUND_FORMAT_PSG	(3<<29)

#define SCHANNEL_TIMER(n)	(*(volatile u16*)(0x04000408 + ((n)<<4)))
#define SCHANNEL_CR(n)		(*(volatile u32*)(0x04000400 + ((n)<<4)))
#define SCHANNEL_VOL(n)		(*(volatile u8*)(0x04000400 + ((n)<<4)))
#define SCHANNEL_PAN(n)		(*(volatile u8*)(0x04000402 + ((n)<<4)))
#define SCHANNEL_SOURCE(n)	(*(volatile u32*)(0x04000404 + ((n)<<4)))
#define SCHANNEL_LENGTH(n)	(*(volatile u32*)(0x0400040C + ((n)<<4)))
#define SOUND_MASTER_VOL	(*(volatile u8*)0x04000500)
#define POWER_CR		(*(volatile u16*)0x04000304)

static u8 s_channels ;
static u32 s_format ;

/* Plays sounds from the specified buffer */
void sound_play(void) {
	u8 i;
	for ( i = 0 ; i < s_channels ; i++ ) {
		SCHANNEL_CR(i) = SOUND_ENABLE | SOUND_ONE_SHOT | s_format ;
		SCHANNEL_VOL(i) = SOUND_VOL(128);
	}
	if ( s_channels == 1 ) {
		SCHANNEL_PAN(0) = 64;
	}
	else {
		for ( i = 0 ; i < s_channels ; i++ ) {
			SCHANNEL_PAN(i) = i%2 ? 128 : 0 ;
		}
	}
}

void sound_set_rate(u32 rate) {
	SCHANNEL_SOURCE(0) = SOUND_FREQ(rate);
};

void sound_set_address(u32 buffer) {
	SCHANNEL_SOURCE(0) = buffer;
	//SCHANNEL_SOURCE(1) = buffer;
};

void sound_set_size(u32 size) {
	SCHANNEL_LENGTH(0) = size;
};

void sound_set_channels(u8 channels) {
	s_channels = channels ;
}

void sound_set_format(u8 format) {
	s_format = format << 29 ;
}

/* Set the master volume */
void sound_set_master_volume(u8 vol) {
	SOUND_MASTER_VOL = SOUND_VOL(vol);
};

/* Turn on/off sound 1 = on 0 = off */
void sound_set_power(u8 state) {
	if( state ) {
		POWER_CR |= 1;
	} else {
		POWER_CR &= ~1;
	};
};

