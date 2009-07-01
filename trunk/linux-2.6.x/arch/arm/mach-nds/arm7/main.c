
#define __DSLINUX_ARM7__

#include "asm/types.h"
#include "asm/arch/fifo.h"
#include "asm/arch/wifi.h"
#include "asm/arch/firmware.h"

#include "microphone.h"
#include "sound.h"
#include "arm7.h"
#include "spi.h"
#include "wifi.h"
#include "time.h"

static s32 xscale, yscale;
static s32 xoffset, yoffset;

static struct nds_firmware_block *firmware_block;

int isDSPhat(void)
{
	return !((power_read(POWER_BACKLIGHT)) & (1<<6));
}
	
/* recieve outstanding FIFO commands from ARM9 */
static void recieveFIFOCommand(void)
{
	u32 fifo_recv;
	u32 data;
	u32 seconds = 0;
	int cmd;
	int cmddata;
	struct nds_tx_packet *tx_packet = NULL;

	while (!(NDS_REG_IPCFIFOCNT & FIFO_EMPTY)) {
		fifo_recv = NDS_REG_IPCFIFORECV;
		data = FIFO_GET_TYPE_DATA(fifo_recv);
		switch (FIFO_GET_TYPE(fifo_recv)) {
		case FIFO_FIRMWARE:
			cmd = FIFO_FIRMWARE_GET_CMD(data);
			switch (cmd) {
			case FIFO_FIRMWARE_CMD_BUFFER_ADDRESS:
				firmware_block = (struct nds_firmware_block*)
				    FIFO_FIRMWARE_DECODE_ADDRESS(
					FIFO_FIRMWARE_GET_DATA(data));
				break;
			case FIFO_FIRMWARE_CMD_READ:
				read_firmware(firmware_block->from,
				    (u8*)firmware_block->data,
				    (int)firmware_block->len);
				    nds_fifo_send(FIFO_FIRMWARE_CMD(
					FIFO_FIRMWARE_CMD_READ, 0));
				break;
			}
			break;
		case FIFO_POWER:
			cmd = FIFO_POWER_GET_CMD(data);
			cmddata = FIFO_POWER_GET_DATA(data);
			switch (cmd) {
			case FIFO_POWER_CMD_BACKLIGHT_BRIGHTNESS:
					if (isDSPhat()) 
						break;
					switch (cmddata) {
					case 0:
						data = power_read(POWER_BACKLIGHT);
						data &=  ~3;
						power_write(POWER_BACKLIGHT, data);
						break;
					case 1:
						data = power_read(POWER_BACKLIGHT);
						data &=  ~3;
						data |= (POWER4_BRIGHTNESS_MED);
						power_write(POWER_BACKLIGHT, data);
						break;
					case 2:
						data = power_read(POWER_BACKLIGHT);
						data &=  ~3;
						data |= (POWER4_BRIGHTNESS_HIGH);
						power_write(POWER_BACKLIGHT, data);
						break;
					case 3:
						
						data = power_read(POWER_BACKLIGHT);
						data |= (POWER4_BRIGHTNESS_MAX);
						power_write(POWER_BACKLIGHT, data);
						break;
					}
			break;
			case FIFO_POWER_CMD_BACKLIGHT_POWER:
					switch (cmddata) {
					case 0:
	 					data = power_read(POWER_CONTROL);
		        			data &= ~(POWER0_LOWER_BACKLIGHT | POWER0_UPPER_BACKLIGHT);
						power_write(POWER_CONTROL, data);
						break;
					case 1:
						data = power_read(POWER_CONTROL);
						data |= (POWER0_LOWER_BACKLIGHT | POWER0_UPPER_BACKLIGHT);
						power_write(POWER_CONTROL, data);
						break;
					}
			break;
			case FIFO_POWER_CMD_SYSTEM_POWER:
					power_write(POWER_CONTROL, POWER0_SYSTEM_POWER);
				break;
				}
	 		break;
		case FIFO_TIME:
			seconds = nds_get_time7();
			nds_fifo_send(FIFO_TIME | FIFO_HIGH_BITS |
			    (seconds >> 16));
			 nds_fifo_send(FIFO_TIME | FIFO_LOW_BITS |
			    (seconds & 0xffff));
			break;
		case FIFO_SOUND:
			switch (data & 0x0f000000) {
			case FIFO_SOUND_FORMAT:
				sound_set_format(data & 0x3);
				break;
			case FIFO_SOUND_RATE:
				sound_set_rate(data & 0x00ffffff);
				break;
			case FIFO_SOUND_CHANNELS:
				sound_set_channels(data & 0xff);
				break;
			case FIFO_SOUND_DMA_ADDRESS:
				sound_set_address((data & 0x00ffffff) +
							  0x02000000);
				break;
			case FIFO_SOUND_DMA_SIZE:
				sound_set_size(data & 0x00ffffff);
				break;
			case FIFO_SOUND_TRIGGER:
				if (data & 1)
					sound_play();
				else
					sound_stop();
				break;
			case FIFO_SOUND_POWER:
				sound_set_power(data & 0x1);
				break;
			}
			break;
		case FIFO_MIC:
			switch (FIFO_MIC_CMD(data)) {
			case FIFO_MIC_FORMAT:
				mic_set_format(FIFO_MIC_DATA(data));
				break;
			case FIFO_MIC_POWER:
				if (data & 0x1) 
					mic_on(); 
				else mic_off();
				break;
			case FIFO_MIC_DMA_ADDRESS:
				mic_set_address(FIFO_MIC_DMA(data));
				break;
			case FIFO_MIC_DMA_SIZE:
				mic_set_size(FIFO_MIC_DATA(data));
				break;
			case FIFO_MIC_PERIOD_SIZE:
				mic_set_period_size(FIFO_MIC_DATA(data));
				break;
			case FIFO_MIC_RATE:
				mic_set_rate(FIFO_MIC_DATA(data));
				break;
			case FIFO_MIC_TRIGGER:
				if (data & 1)
					mic_start();
				else
					mic_stop();
				break;
			}
			break;
		case FIFO_WIFI:
			cmd = FIFO_WIFI_GET_CMD(data);
			switch (cmd) {
			case FIFO_WIFI_CMD_UP:
				wifi_open();
				break;
			case FIFO_WIFI_CMD_DOWN:
				wifi_close();
				break;
			case FIFO_WIFI_CMD_MAC_QUERY:
				wifi_mac_query();
				break;
			case FIFO_WIFI_CMD_TX:
				tx_packet = (struct nds_tx_packet*)FIFO_WIFI_DECODE_ADDRESS(FIFO_WIFI_GET_DATA(data));
				wifi_send_ether_packet(tx_packet->len, tx_packet->data);
				tx_packet = NULL;
				break;
			case FIFO_WIFI_CMD_RX:
				/* ARM9 wants to tell us the address of a
				 * recieve buffer we can use. */
				rx_packet = (struct nds_rx_packet*)FIFO_WIFI_DECODE_ADDRESS(FIFO_WIFI_GET_DATA(data));
				break;	
			case FIFO_WIFI_CMD_RX_COMPLETE:
				wifi_rx_q_complete();
				break;
			case FIFO_WIFI_CMD_STATS_QUERY:
				if (FIFO_WIFI_GET_DATA(data) != 0) {
					/* ARM9 gives us stats buffer address */
					wifi_data.stats = (u32*) \
					    FIFO_WIFI_DECODE_ADDRESS( \
						FIFO_WIFI_GET_DATA(data));
				} else if (wifi_data.stats) {
					/* stats requested */
					wifi_stats_query();
				}
				break;
			case FIFO_WIFI_CMD_SET_ESSID:
				Wifi_SetSSID(/* #essid (0 - 3) */
					     4 * ((data >> 20) & 0x3)
					     /* offset */
					     + ((data >> 16) & 0x3),
					     /* 2 chars */
					     (data >> 8) & 0xff, data & 0xff);
				break;
			case FIFO_WIFI_CMD_SET_CHANNEL:
				Wifi_RequestChannel(data & 0xff);
				break;
			case FIFO_WIFI_CMD_SET_WEPKEY:
				Wifi_SetWepKey( /* #key */
						((data >> 20) & 0x3),
						/* offset */
						4 * ((data >> 18) & 0x3)
						+ ((data >> 16) & 0x3),
						/* 2 chars */
					       (data >> 8) & 0xff, data & 0xff);
				break;
			case FIFO_WIFI_CMD_SET_WEPKEYID:
				Wifi_SetWepKeyID(data & 0xff);
				break;
			case FIFO_WIFI_CMD_SET_WEPMODE:
				Wifi_SetWepMode(data & 0xff);
				break;
			case FIFO_WIFI_CMD_AP_QUERY:
				if (FIFO_WIFI_GET_DATA(data) > 1) {
					/* Getting the address of the ap query
					 * buffer*/
					wifi_data.aplist = \
					    (Wifi_AccessPoint*) \
					    FIFO_WIFI_DECODE_ADDRESS( \
						FIFO_WIFI_GET_DATA(data));
				} else if (wifi_data.aplist) {
					wifi_ap_query(data & 0xffff);
				}
				break;
			case FIFO_WIFI_CMD_SCAN:
				wifi_start_scan();
				break;
			case FIFO_WIFI_CMD_SET_AP_MODE:
				Wifi_SetAPMode(FIFO_WIFI_GET_DATA(data));
				break;
			case FIFO_WIFI_CMD_GET_AP_MODE:
				Wifi_GetAPMode();
				break;
			}
		}
	}
}

#define _MaxRetry 5
#define _MaxRange 30

/* sends touch state to ARM9 */
static void sendTouchState(u16 buttons)
{
	s16 x, y, px, py;
	static u8 lastx = 0, lasty = 0;

	if (buttons & TOUCH_RELEASED) {
		if (lasty != 255)
			nds_fifo_send(FIFO_TOUCH);
		lastx = 255;
		lasty = 255;
	} else {
		/* Some dude is smacking his fingerprint on the touchscreen. */
                
		/* Code from devkitpro/libnds/source/arm7/touch.c */
                x = touch_read_value(TSC_MEASURE_X, _MaxRetry, _MaxRange);
                y = touch_read_value(TSC_MEASURE_Y, _MaxRetry, _MaxRange);

                px = ( x * xscale - xoffset + xscale/2 ) >>19;
                py = ( y * yscale - yoffset + yscale/2 ) >>19;

                if ( px < 0) px = 0;
                if ( py < 0) py = 0;
                if ( px > (256 -1)) px = 256 -1;
                if ( py > (192 -1)) py = 192 -1;

                x = px;
                y = py;

		if (lastx + 6 > x && lastx < x + 6 &&
		    lasty + 6 > y && lasty < y + 6) {
			nds_fifo_send(FIFO_TOUCH | 1 << 16 | x << 8 | y);
		}
		lastx = x;
		lasty = y;
	}
}

void InterruptHandler(void)
{
	u16 buttons;
	static u16 oldbuttons = 0x43; /* default stats */
	u32 wif;

	wif = NDS_IF;

	if (wif & IRQ_RECV) {
		/* Acknowledge Interrupt */
		NDS_IF = IRQ_RECV;
		wif &= ~IRQ_RECV;

		recieveFIFOCommand();
	}

	if (wif & IRQ_VBLANK) {
		/* read X and Y, lid and touchscreen buttons */
		buttons = XKEYS;

		/* send button state to ARM9 */
		if (buttons != oldbuttons) {
			nds_fifo_send(FIFO_BUTTONS | buttons);
			oldbuttons = buttons;
		}

		sendTouchState(buttons);

		/* clear FIFO errors (just in case) */
		if (NDS_REG_IPCFIFOCNT & (1 << 14))
			NDS_REG_IPCFIFOCNT |= (1 << 15) | (1 << 14);

		/* Acknowledge Interrupt */
		NDS_IF = IRQ_VBLANK;
		wif &= ~IRQ_VBLANK;
	}

	if (wif & IRQ_TIMER0) {
		/* Acknowledge Interrupt */
		NDS_IF = IRQ_TIMER0;
		wif &= ~IRQ_TIMER0;
		wifi_timer_handler();
	}
	
	if (wif & IRQ_TIMER3) {
		/* Acknowlege Interrupt, send interrupt to mic */
		NDS_IF = IRQ_TIMER3;
		wif &= ~IRQ_TIMER3;
		mic_timer_handler();
	}

	if (wif & IRQ_ARM9) {
		/* Acknowledge Interrupt */
		NDS_IF = IRQ_ARM9;
		wif &= ~IRQ_ARM9;
	}

	if ((wif & IRQ_WIFI) || WIFI_IF)  {
		NDS_IF = IRQ_WIFI;
		wif &= ~IRQ_WIFI;

		wifi_interrupt();
	}

	/* Acknowledge unhandled Interrupts */
	NDS_IF = wif;
}

int main(void)
{
	/* Disable Interrupts */
	NDS_IME = 0;

	/* Read calibration values, the arm9 will probably overwrite the originals later */
	// Code from devkitpro/libnds/source/arm7/touch.c
	xscale = ((PersonalData->calX2px - PersonalData->calX1px) << 19) / ((PersonalData->calX2) - (PersonalData->calX1));
	yscale = ((PersonalData->calY2px - PersonalData->calY1px) << 19) / ((PersonalData->calY2) - (PersonalData->calY1));
                
	xoffset = ((PersonalData->calX1 + PersonalData->calX2) * xscale  - ((PersonalData->calX1px + PersonalData->calX2px) << 19) ) / 2;
	yoffset = ((PersonalData->calY1 + PersonalData->calY2) * yscale  - ((PersonalData->calY1px + PersonalData->calY2px) << 19) ) / 2;

	/* Dummy read to enable the touchpad PENIRQ */
	touch_read_value(TSC_MEASURE_X, _MaxRetry, _MaxRange);

	/* Enable VBLANK Interrupt */
	DISP_SR = DISP_VBLANK_IRQ;

	/* Set the INT enable flags */
	NDS_IE = IRQ_VBLANK | IRQ_RECV | IRQ_ARM9 | IRQ_WIFI;

	/* Set the INT bitmask for the swi wait call */
	VBLANK_INTR_WAIT_FLAGS = IRQ_VBLANK;

	/* Enable FIFO */
	NDS_REG_IPCFIFOCNT = FIFO_ENABLE | FIFO_IRQ_ENABLE | FIFO_CLEAR | FIFO_ERROR ;

	/* Set interrupt handler */
	IRQ_HANDLER = (u32) &InterruptHandler;

	/* init the wifi stuff */
	wifi_init();

	/* Enable Interrupts */
	NDS_IF = ~0;
	NDS_IME = 1;

	/* power down ARM7, until the next activated interrupt */
	while (1) {
		swiWaitForVBlank();
	}
}
