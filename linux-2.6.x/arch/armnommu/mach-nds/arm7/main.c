
#include "asm/types.h"

extern void swiDelay( u32 duration );
extern void swiWaitForVBlank( void );

#define REG_IPCFIFOSEND	(*(volatile u32*) 0x04000188)
#define REG_IPCFIFORECV	(*(volatile u32*) 0x04100000)
#define REG_IPCFIFOCNT	(*(volatile u16*) 0x04000184)

#define NDS_IE		(*(volatile u32*)0x04000210)	/* Interrupt mask */
#define NDS_IF		(*(volatile u32*)0x04000214)	/* Interrup service */
#define NDS_IME		(*(volatile u32*)0x04000208)	/* Enable/disable */

#define DISP_SR		(*(volatile u16*)0x04000004)

#define DISP_VBLANK_IRQ	(1 << 3)

#define IRQ_VBLANK	(1 << 0)

#define XKEYS		(*(volatile u16*)0x04000136)

void InterruptHandler(void)
{
	u16 buttons;
	u32 data;

	if ( NDS_IF & IRQ_VBLANK )
	{
		/* Read any FIFO messages */
#if 0
		while ( ! ( REG_IPCFIFOCNT & (1<<3) ) )
		{
			data = REG_IPCFIFORECV;
			/* do something */
		}
#endif

		/* read the X, Y and touch buttons */
		buttons = XKEYS;

		/* send to ARM9 */
		REG_IPCFIFOSEND = buttons;

		/* clear FIFO errors (just in case) */
		if ( REG_IPCFIFOCNT & (1<<14) )
			REG_IPCFIFOCNT |= (1<<15) | (1<<14);
	}

	/* Acknowledge Interrupts */
	NDS_IF = NDS_IF;
}

int main( void )
{
	/* Disable Interrupts */
	NDS_IME = 0;

	/* Enable VBLANK Interrupt */
	DISP_SR = DISP_VBLANK_IRQ;
	NDS_IE = IRQ_VBLANK;

	/* Enable FIFO */
	REG_IPCFIFOCNT = (1<<15) | (1<<3) | (1<<10) | (1<<14);

	/* Set interrupt handler */
	*(volatile u32*)(0x04000000-4) = &InterruptHandler;

	/* Enable Interrupts */
	NDS_IF = ~0;
	NDS_IME = 1;

	while(1)
	{
		swiWaitForVBlank();
	}
}
