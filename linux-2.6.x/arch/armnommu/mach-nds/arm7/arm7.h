extern void swiDelay(u32 duration);
extern void swiWaitForVBlank(void);

#define REG_IPCFIFOSEND	(*(volatile u32*) 0x04000188)
#define REG_IPCFIFORECV	(*(volatile u32*) 0x04100000)
#define REG_IPCFIFOCNT	(*(volatile u16*) 0x04000184)

#define NDS_IE		(*(volatile u32*)0x04000210)	/* Interrupt mask */
#define NDS_IF		(*(volatile u32*)0x04000214)	/* Interrup service */
#define NDS_IME		(*(volatile u32*)0x04000208)	/* Enable/disable */

#define DISP_SR		(*(volatile u16*)0x04000004)

#define POWERCNT7   (*(volatile u16*)0x04000304)

// Wifi regs
#define WIFI_REG(ofs)   (*(volatile u16*)(0x04800000+(ofs)))
#define WIFI_WEPKEY0    ( (volatile u16*)0x04805F80)
#define WIFI_WEPKEY1    ( (volatile u16*)0x04805FA0)
#define WIFI_WEPKEY2    ( (volatile u16*)0x04805FC0)
#define WIFI_WEPKEY3    ( (volatile u16*)0x04805FE0)

#define WIFI_MODE_RST   (*(volatile u16*)0x04800004)
#define WIFI_MODE_WEP   (*(volatile u16*)0x04800006)
#define WIFI_IF         (*(volatile u16*)0x04800010)
#define WIFI_IE         (*(volatile u16*)0x04800012)
#define WIFI_MACADDR    ( (volatile u16*)0x04800018)
#define WIFI_BSSID      ( (volatile u16*)0x04800020)
#define WIFI_AIDS       (*(volatile u16*)0x04800028)
#define WIFI_RETRLIMIT  (*(volatile u16*)0x0480002C)
#define WIFI_POWERSTATE (*(volatile u16*)0x0480003C)
#define WIFI_RANDOM     (*(volatile u16*)0x04800044)

#define WIFI_BBSIOCNT   (*(volatile u16*)0x04800158)
#define WIFI_BBSIOWRITE (*(volatile u16*)0x0480015A)
#define WIFI_BBSIOREAD  (*(volatile u16*)0x0480015C)
#define WIFI_BBSIOBUSY  (*(volatile u16*)0x0480015E)
#define WIFI_RFSIODATA2 (*(volatile u16*)0x0480017C)
#define WIFI_RFSIODATA1 (*(volatile u16*)0x0480017E)
#define WIFI_RFSIOBUSY  (*(volatile u16*)0x04800180)


#define DISP_VBLANK_IRQ	(1 << 3)

#define IRQ_VBLANK	(1 << 0)
#define IRQ_ARM9	(1 << 16)
#define IRQ_RECV	(1 << 18)
#define IRQ_WIFI	(1 << 24)

#define XKEYS		(*(volatile u16*)0x04000136)
#define TOUCH_RELEASED  0x40

#define TOUCH_CAL_X1 (*(volatile s16*)0x027FFCD8)
#define TOUCH_CAL_Y1 (*(volatile s16*)0x027FFCDA)
#define TOUCH_CAL_X2 (*(volatile s16*)0x027FFCDE)
#define TOUCH_CAL_Y2 (*(volatile s16*)0x027FFCE0)

#define TOUCH_CNTRL_X1   (*(volatile u8*)0x027FFCDC)
#define TOUCH_CNTRL_Y1   (*(volatile u8*)0x027FFCDD)
#define TOUCH_CNTRL_X2   (*(volatile u8*)0x027FFCE2)
#define TOUCH_CNTRL_Y2   (*(volatile u8*)0x027FFCE3)

#define SERIAL_CR	(*(volatile u16*)0x040001C0)
#define SERIAL_DATA	(*(volatile u16*)0x040001C2)

#define SERIAL_ENABLE   0x8000
#define SERIAL_BUSY	0x80

#define TSC_MEASURE_Y		0x90
#define TSC_MEASURE_BATTERY	0xA4
#define TSC_MEASURE_Z1		0xB0
#define TSC_MEASURE_Z2		0xC0
#define TSC_MEASURE_X		0xD0

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))
