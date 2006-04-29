#ifndef _mach_nds_arm7_arm7_h_
#define _mach_nds_arm7_arm7_h_

extern void swiDelay(u32 duration);
extern void swiWaitForVBlank(void);

/*---------------------------------------------------------------------------------
	IntrWait (swi 0x04)

	waitForSet -	0: Return if the interrupt has already occured
			1: Wait until the interrupt has been set since the call
	flags - 	interrupt sensitivity bitmask to wait for
---------------------------------------------------------------------------------*/
extern void swiIntrWait(int waitForSet, u32 flags);


#define NDS_IE		(*(volatile u32*)0x04000210)	/* Interrupt mask */
#define NDS_IF		(*(volatile u32*)0x04000214)	/* Interrup service */
#define NDS_IME		(*(volatile u32*)0x04000208)	/* Enable/disable */

#define DISP_SR		(*(volatile u16*)0x04000004)

#define VBLANK_INTR_WAIT_FLAGS  (*(volatile u32*)(0x03FFFFF8))
#define IRQ_HANDLER             (*(volatile u32*)(0x03FFFFFC))

#define POWERCNT7   (*(volatile u16*)0x04000304)

// Timer registers
#define REG_TM0CNT_L    (*(volatile u16*)0x04000100)
#define REG_TM1CNT_L    (*(volatile u16*)0x04000104)
#define REG_TM2CNT_L    (*(volatile u16*)0x04000108)
#define REG_TM3CNT_L    (*(volatile u16*)0x0400010C)

#define REG_TM0CNT_H    (*(volatile u16*)0x04000102)
#define REG_TM1CNT_H    (*(volatile u16*)0x04000106)
#define REG_TM2CNT_H    (*(volatile u16*)0x0400010A)
#define REG_TM3CNT_H    (*(volatile u16*)0x0400010E)

/**
 **  Timer control register flags.
 **/
#define NDS_TCR_CLK 0x0000          /* Use clock freq */
#define NDS_TCR_CLK64   0x0001          /* Use clock/64 freq */
#define NDS_TCR_CLK256  0x0002          /* Use clock/256 freq */
#define NDS_TCR_CLK1024 0x0003          /* Use clock/1024 freq */
#define NDS_TCR_CASCADE 0x0004          /* Cascade timer */
#define NDS_TCR_IRQ 0x0040          /* Generate IRQ */
#define NDS_TCR_ENB 0x0080          /* Enable timer */


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
#define IRQ_TIMER0	(1 << 3)
#define IRQ_TIMER1	(1 << 4)
#define IRQ_TIMER2	(1 << 5)
#define IRQ_TIMER3	(1 << 6)
#define IRQ_ARM9	(1 << 16)
#define IRQ_RECV	(1 << 18)
#define IRQ_WIFI	(1 << 24)

#define XKEYS		(*(volatile u16*)0x04000136)
#define TOUCH_RELEASED  0x40

// Code from devkitpro/libnds/include/nds/system.h
typedef struct tPERSONAL_DATA {
  u8  RESERVED0[2];           //0x023FFC80  05 00 ?

  u8  theme;                  //0x027FFC82  favorite color (0-15)
  u8  birthMonth;             //0x027FFC83  birthday month (1-12)
  u8  birthDay;               //0x027FFC84  birthday day (1-31)

  u8  RESERVED1[1];           //0x027FFC85  ???

  s16 name[10];               //0x027FFC86  name, UTF-16?
  u16 nameLen;                //0x027FFC9A  length of name in characters

  s16 message[26];            //0x027FFC9C  message, UTF-16?
  u16 messageLen;             //0x027FFCD0  length of message in characters

  u8  alarmHour;              //0x027FFCD2  alarm hour
  u8  alarmMinute;            //0x027FFCD3  alarm minute

  u8  RESERVED2[4];           //0x027FFCD4  ??

  //calibration information
  u16 calX1;                  //0x027FFCD8
  u16 calY1;                  //0x027FFCDA
  u8  calX1px;                //0x027FFCDC
  u8  calY1px;                //0x027FFCDD

  u16 calX2;                  //0x027FFCDE
  u16 calY2;                  //0x027FFCE0
  u8  calX2px;                //0x027FFCE2
  u8  calY2px;                //0x027FFCE3

                              //0x027FFCE4
    unsigned language    : 3; //            language
    unsigned gbaScreen   : 1; //            GBA mode screen selection. 0=upper, 1=lower
    unsigned RESERVED3   : 2; //            ??
    unsigned autoMode    : 1; //            auto/manual mode. 0=manual, 1=auto
    unsigned RESERVED4   : 1; //            ??
} __attribute__ ((__packed__)) PERSONAL_DATA ;

#define PersonalData ((PERSONAL_DATA*)0x27FFC80)

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

#endif
