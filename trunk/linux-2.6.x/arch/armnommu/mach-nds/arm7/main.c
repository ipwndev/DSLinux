#include "asm/types.h"
#include "asm/arch/fifo.h"
#include "asm/arch/ipcsync.h"
#include "asm/arch/shmemipc.h"

#include "sound.h"
#include "arm7.h"
#include "shmemipc-arm7.h"
#include "spi.h"

static s16 cntrl_width;
static s16 cntrl_height;
static s16 touch_cntrl_x1;
static s16 touch_cntrl_y1;
static s16 touch_width;
static s16 touch_height;
static s16 touch_cal_x1;
static s16 touch_cal_y1;

/* recieve outstanding FIFO commands from ARM9 */
static void recieveFIFOCommand(void)
{
	u32 data;
    u32 seconds = 0;
    u32 address;

	while ( ! ( REG_IPCFIFOCNT & (1<<8) ) )
	{
		data = REG_IPCFIFORECV;

        switch ( data & 0xf0000000 )
        {
            case FIFO_POWER :
                poweroff() ;
                break ;
            case FIFO_TIME :
                seconds = nds_get_time7();
                REG_IPCFIFOSEND = ( FIFO_TIME | FIFO_HIGH_BITS | (seconds>>16)      );
                REG_IPCFIFOSEND = ( FIFO_TIME | FIFO_LOW_BITS  | (seconds & 0xFFFF) );
                break;
            case FIFO_SOUND :
                address = data & 0xffffff + 0x02000000 ;
                playSound(address); 
                break ;
        }
	}
}

/* sends touch state to ARM9 */
static void sendTouchState(u16 buttons)
{
	u16 x,y;
	static u8 lastx = 0, lasty = 0;

	if ( buttons & TOUCH_RELEASED )
	{
		lastx = -1 ;
		lasty = -1 ;
		REG_IPCFIFOSEND = FIFO_TOUCH ;
	} else { /* Some dude is smacking his fingerprint on the touchscreen. */
		x = touchRead(TSC_MEASURE_X);
		y = touchRead(TSC_MEASURE_Y);
		x = ( (x - touch_cal_x1) * cntrl_width) /
			(touch_width) + touch_cntrl_x1;
		y = ( (y - touch_cal_y1) * cntrl_height) /
			(touch_height) + touch_cntrl_y1;
		x = MIN(255,MAX(x,0));
		y = MIN(191,MAX(y,0));

		if ( lastx + 6 > x && lastx < x + 6 &&
		     lasty + 6 > y && lasty < y + 6 )
		{
			REG_IPCFIFOSEND = FIFO_TOUCH | 1 << 16 | x << 8 | y ;
		}
		lastx = x ;
		lasty = y ;
	}
}

void InterruptHandler(void)
{
	u16 buttons;
	static u16 oldbuttons;

	if ( NDS_IF & IRQ_RECV )
		recieveFIFOCommand();

	if ( NDS_IF & IRQ_VBLANK )
	{
		/* read X and Y, lid and touchscreen buttons */
		buttons = XKEYS;

		/* send button state to ARM9 */
		if (buttons != oldbuttons) {
			REG_IPCFIFOSEND = FIFO_BUTTONS | buttons;
			oldbuttons = buttons;
		}

		sendTouchState(buttons);

		/* clear FIFO errors (just in case) */
		if ( REG_IPCFIFOCNT & (1<<14) )
			REG_IPCFIFOCNT |= (1<<15) | (1<<14);
	}

	if (NDS_IF & IRQ_ARM9) {
		switch (ipcsync_get_remote_status()) {
			case SHMEMIPC_REQUEST_FLUSH:
				shmemipc_serve_flush_request();
				break;
			case SHMEMIPC_FLUSH_COMPLETE:
				shmemipc_flush_complete();
				break;
		}
	}

	/* Acknowledge Interrupts */
	NDS_IF = NDS_IF;
}

int main( void )
{
	/* Disable Interrupts */
	NDS_IME = 0;

	/* Read calibration values, the arm9 will probably overwrite the originals later */
	touch_width  = TOUCH_CAL_X2 - TOUCH_CAL_X1;
	touch_height = TOUCH_CAL_Y2 - TOUCH_CAL_Y1;
	touch_cal_x1 = TOUCH_CAL_X1;
	touch_cal_y1 = TOUCH_CAL_Y1;
	cntrl_width  = TOUCH_CNTRL_X2 - TOUCH_CNTRL_X1;
	cntrl_height = TOUCH_CNTRL_Y2 - TOUCH_CNTRL_Y1;
	touch_cntrl_x1 = TOUCH_CNTRL_X1;
	touch_cntrl_y1 = TOUCH_CNTRL_Y1;

	/* Enable VBLANK Interrupt */
	DISP_SR = DISP_VBLANK_IRQ;
	NDS_IE = IRQ_VBLANK | IRQ_RECV | IRQ_ARM9;

	/* Enable FIFO */
	REG_IPCFIFOCNT = (1<<15) | (1<<3) | (1<<10) | (1<<14);

	ipcsync_allow_local_interrupt();

	/* Set interrupt handler */
	*(volatile u32*)(0x04000000-4) = (u32)&InterruptHandler;

	/* Enable Interrupts */
	NDS_IF = ~0;
	NDS_IME = 1;

	while(1)
	{
		swiWaitForVBlank();
	}
}
