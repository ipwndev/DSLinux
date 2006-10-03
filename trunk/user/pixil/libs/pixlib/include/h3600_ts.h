/*                                                                       
 * Copyright (c) 2003 Century Software, Inc.   All Rights Reserved.     
 *                                                                       
 * This file is part of the PIXIL Operating Environment                 
 *                                                                       
 * The use, copying and distribution of this file is governed by one    
 * of two licenses, the PIXIL Commercial License, or the GNU General    
 * Public License, version 2.                                           
 *                                                                       
 * Licensees holding a valid PIXIL Commercial License may use this file 
 * in accordance with the PIXIL Commercial License Agreement provided   
 * with the Software. Others are governed under the terms of the GNU   
 * General Public License version 2.                                    
 *                                                                       
 * This file may be distributed and/or modified under the terms of the  
 * GNU General Public License version 2 as published by the Free        
 * Software Foundation and appearing in the file LICENSE.GPL included   
 * in the packaging of this file.                                      
 *                                                                       
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING  
 * THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A            
 * PARTICULAR PURPOSE.                                                  
 *                                                                       
 * RESTRICTED RIGHTS LEGEND                                             
 *                                                                     
 * Use, duplication, or disclosure by the government is subject to      
 * restriction as set forth in paragraph (b)(3)(b) of the Rights in     
 * Technical Data and Computer Software clause in DAR 7-104.9(a).       
 *                                                                      
 * See http://www.pixil.org/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://www.pixil.org/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */


/*
*
* Driver for the H3600 Touch Screen and other Atmel controlled devices.
*
* Copyright 2000 Compaq Computer Corporation.
*
* Use consistent with the GNU GPL is permitted,
* provided that this copyright notice is
* preserved in its entirety in all copies and derived works.
*
* COMPAQ COMPUTER CORPORATION MAKES NO WARRANTIES, EXPRESSED OR IMPLIED,
* AS TO THE USEFULNESS OR CORRECTNESS OF THIS CODE OR ITS
* FITNESS FOR ANY PARTICULAR PURPOSE.
*
* Author: Charles Flynn.
*
*/


#ifndef __H3600_TS_H__
#define __H3600_TS_H__

#ifndef _LINUX_IOCTL_H
#include <linux/ioctl.h>
#endif

typedef struct
{
    int xscale;
    int xtrans;
    int yscale;
    int ytrans;
    int xyswap;
}
TS_CAL;

typedef struct
{
    unsigned short pressure;
    unsigned short x;
    unsigned short y;
    unsigned short pad;		/* TODO TODO word boundary pad */
}
TS_EVENT;


#ifdef __KERNEL__

/* ++++++++++++++++++ ISR related defines ++++++++++++++++ */

#define TS_IRQ         IRQ_Ser1UART	/* see irqs.h */

/* The start and end of frame characters SOF and EOF */
#define CHAR_SOF		0x02
#define	CHAR_EOF		0x03
#define FRAME_OVERHEAD		3	/* CHAR_SOF,CHAR_EOF,LENGTH = 3 */

/* decode States  */
#define	STATE_SOF	0	/* start of FRAME */
#define	STATE_ID	1	/* state where we decode the ID & len */
#define	STATE_DATA	2	/* state where we decode data */
#define	STATE_EOF	3	/* state where we decode checksum or EOF */

/*
	Atmel events and response IDs contained in frame.
	Programmer has no control over these numbers.
	TODO there are holes - specifically  1,7,0x0a
*/
#define VERSION_ID		0	/* Get Version (request/respose) */
#define	KEYBD_ID		2	/* Keboard (event) */
#define TOUCHS_ID		3	/* Touch Screen (event) */
#define EEPROM_READ_ID		4	/* (request/response) */
#define EEPROM_WRITE_ID		5	/* (request/response) */
#define	THERMAL_ID		6	/* (request/response) */
#define	NOTIFY_LED_ID		8	/* (request/response) */
#define	BATTERY_ID		9	/* (request/response) */
#define IIC_READ_ID		0x0b	/* ( request/response) */
#define IIC_WRITE_ID		0x0c	/* ( request/response) */
#define FLITE_ID		0x0d	/* backlight ( request/response) */

#define	MAX_ID			14

#define TS_MINOR		0	/* ts interface */
#define TSRAW_MINOR		1	/* ts raw interface */
#define KEY_MINOR		2	/* button events */
#define MAX_MINOR		3

/* WARNING TXBUF_MAX must be a power of 2 (16 32 64 128 256 ....) */
#define TXBUF_MAX	32
/* TODO change TXBUF_MAX to SERIAL_XMIT_SIZE */
typedef struct
{
    unsigned char state;	/* context of tx state machine */
    unsigned char event;	/* event ID from packet */
    unsigned char chksum;
    unsigned char len;		/* tx buffer length */
    unsigned char buf[TXBUF_MAX];	/* transmitter buffer */
    unsigned head;
    unsigned tail;
}
TXDEV;



/* event independent structure */
#define RXBUF_MAX	16	/* Do NOT make this bigger!! */
#define MAX_RXDEV_COUNTERS      3	/* array of counters */
typedef struct
{
    unsigned char state;	/* context of rx state machine */
    unsigned char event;	/* event ID from packet */
    unsigned char chksum;
    unsigned char len;		/* rx buffer length */
    unsigned int idx;		/* rx buffer index */
    unsigned int counter[MAX_RXDEV_COUNTERS];
    /* WARNING making buf[]  signed will cause problems */
    unsigned char buf[RXBUF_MAX];	/* receive event buffer */
}
RXDEV;

/* status returns from ReadByte() *//* TODO */
#define RX_NODATA	-1
#define	RX_ERR		-2

/* ++++++++++++++++++ end of ISR related defines ++++++++++++++++ */

/*
	Define all the module name
	TODO non-contiguous is this a problem?
	TODO do we have a separate tsraw device?
	The following device nodes will also be created
	Minor	Name
	0	ver
	2	keybd
	3	ts
	4	eeprom	( 4=eeprom_read 5=eeprom_write )
	8	led
	9	battery
	11	iic	( 11=iicread 12=iicwrite)
	13	light	( back light )
	
*/
#define MOD_NAME	"h3600"

/* TODO temp struct what will the client want returned - change accordingly */
typedef struct
{
    unsigned short pressure;
    unsigned short x;
    unsigned short y;
    unsigned short pad;		/* TODO TODO word boundary pad */
}
TS_RET;

/* TODO */
typedef struct
{
    int dummy1;
    int dummy2;
}
TS_RAW;


/* ++++++++++++++ declare the generic driver structures ++++++++++++++*/
typedef struct id
{
    void *pdev;			/* (1)Used by Atmel to queue events */
    void *pReturn;		/* (2)This struct is returned to user */
    int (*processEvent) (struct id *);	/* (3)pointer to event handler */
    int (*initDev) (struct id *);	/* (4)initialisation function */
    unsigned lenDev;		/* (5)sizeof(*pdev) */
    unsigned lenRet;		/* (6)sizeof(*Return) */
    unsigned sn;		/* (7)used by all except events */
    wait_queue_head_t wq;	/* (8)for 2.2.3.xx kernels */
}
ID;

typedef struct
{
    int (*processRead) (struct id *);
    int (*processIoctl) (void *, unsigned int, unsigned long);
    unsigned id;		/* (3) Atmel ID - gets info from ID table */
    unsigned head;		/* (4) head and tail for queued events */
    unsigned tail;		/* (5) */
    struct fasync_struct *aq;	/* (6) async notifications */
}
EVENT;

#endif /* ifndef __KERNEL__ */

/* ++++++++++++++ +++++++++++++++++++++++++++++++++++++ */


/* Repeat above for other events example the Dummy event (DM) */
int NullRes(void);		/* place holder for holes in the event table */

/* bit 7 = state bits0-3 = key number */
#define KEY_RELEASED   0x80
#define KEY_NUM	0x0f
#define MAX_KEY_EVENTS 4
typedef struct
{
    unsigned char buf[MAX_KEY_EVENTS];
}
KEY_DEV;


#define MAX_TS_EVENTS	8	/* how many do we want to buffer */
#define TS_DATA_LEN	4	/* sizeof payload ( in bytes) */

#define	PEN_UP		0	/* default at module load time */
#define	PEN_DOWN	1	/* pen is well and truely down */
#define	PEN_FLEETING	2	/* pen is neither up or down */

typedef struct
{
    unsigned rate;		/* rate in samples per sec from the ATMEL */
    unsigned penStatus;		/* PEN_UP, PEN_DOWN, PEN_SAMPLE */
    TS_CAL cal;			/* ts calibration parameters */
    TS_EVENT events[MAX_TS_EVENTS];	/* protect against overrun */
}
TS_DEV;

typedef struct therm_dev
{
    short data;
}
THERM_DEV;

typedef struct bat_dev
{
    char ac_status;
    char batt1_chemistry;
    short batt1_voltage;
    char batt1_status;
    char batt2_chemistry;
    short batt2_voltage;
    char batt2_status;
}
BAT_DEV;

#define H3600_BATT_CHEM_ALKALINE    0x1
#define H3600_BATT_CHEM_NICD        0x2
#define H3600_BATT_CHEM_NIMH        0x3
#define H3600_BATT_CHEM_LION        0x4
#define H3600_BATT_CHEM_LIPOLY      0x5
#define H3600_BATT_CHEM_UNKNOWN    0xff

#define H3600_AC_STATUS_AC_OFFLINE	0x0
#define H3600_AC_STATUS_AC_ONLINE	0x1
#define H3600_AC_STATUS_AC_BACKUP	0x2
#define H3600_AC_STATUS_AC_UNKNOWN	0xff

#define H3600_BATT_STATUS_HIGH		0x1
#define H3600_BATT_STATUS_LOW		0x2
#define H3600_BATT_STATUS_CRITICAL	0x4
#define H3600_BATT_STATUS_CHARGING	0x8
#define H3600_BATT_STATUS_NOBATT	0x80
#define H3600_BATT_STATUS_UNKNOWN	0xff


typedef struct
{
    unsigned short addr;
    unsigned char len;
    unsigned char data[64];
}
IIC_REQ;


typedef struct
{
    int dummy1;
    int dummy2;
}
FLT_DEV;


/* User space structure to control the LED */

typedef struct
{
    unsigned short maj;
    unsigned short min;
}
VER_RET;

typedef struct
{
    unsigned char OffOnBlink;	/* 0=off 1=on 2=Blink */
    unsigned char TotalTime;	/* Units of 5 seconds */
    unsigned char OnTime;	/* units of 100m/s */
    unsigned char OffTime;	/* units of 100m/s */
}
LED_IN;

enum flite_mode
{
    FLITE_MODE1 = 1
};
enum flite_pwr
{
    FLITE_PWR_OFF = 0,
    FLITE_PWR_ON = 1
};
typedef struct
{
    unsigned char mode;
    unsigned char pwr;
    unsigned char brightness;
}
FLITE_IN;



typedef struct
{
    unsigned char addr;		/* 0-255 ? */
    unsigned char len;		/* 0-255 ? */
    unsigned char buf[16];	/* TODO cant have len > 16 */
}
EEPROM_REQ;


/* IOCTL cmds  user or kernel space */

/* Use 'f' as magic number */
#define IOC_MAGIC  'f'

#define GET_VERSION		_IOR(IOC_MAGIC, 1,VER_RET)
#define READ_EEPROM		_IOWR(IOC_MAGIC, 2,EEPROM_REQ)
#define WRITE_EEPROM		_IOWR(IOC_MAGIC, 3,EEPROM_REQ)
#define GET_THERMAL		_IOR(IOC_MAGIC, 4, THERM_DEV)
#define LED_ON			_IOW(IOC_MAGIC, 5, LED_IN)
#define GET_BATTERY_STATUS	_IOR(IOC_MAGIC, 6, BAT_DEV)
#define FLITE_ON		_IOW(IOC_MAGIC, 7, FLITE_IN)
#define READ_IIC		_IOWR(IOC_MAGIC, 8,IIC_REQ)
#define WRITE_IIC		_IOWR(IOC_MAGIC, 9,IIC_REQ)


#define TS_GET_RATE		_IO(IOC_MAGIC, 8)
#define TS_SET_RATE		_IO(IOC_MAGIC, 9)
#define TS_GET_CAL		_IOR(IOC_MAGIC, 10, TS_CAL)
#define TS_SET_CAL		_IOW(IOC_MAGIC, 11, TS_CAL)

#define WT_IOC_MAXNR 11		/* TODO do we check this? */

#define H3600_SCANCODE_RECORD   120
#define H3600_SCANCODE_CALENDAR 122
#define H3600_SCANCODE_CONTACTS 123
#define H3600_SCANCODE_Q        124
#define H3600_SCANCODE_START    125
#define H3600_SCANCODE_UP       103	/* keycode up */
#define H3600_SCANCODE_RIGHT    106	/* keycode right */
#define H3600_SCANCODE_LEFT     105	/* keycode left */
#define H3600_SCANCODE_DOWN     108	/* keycode down */
#define H3600_SCANCODE_ACTION   96  /* keycode keypad enter */	/* 28 is regular enter, 126 is rocker enter */
#define H3600_SCANCODE_SUSPEND  121	/* keycode powerdown */

extern unsigned int h3600_flite_control(enum flite_pwr pwr,
					unsigned char brightness);
/* uses last brightness setting */
extern unsigned int h3600_flite_power(enum flite_pwr pwr);

unsigned int h3600_apm_get_power_status(u_char * ac_line_status,
					u_char * battery_status,
					u_char * battery_flag,
					u_char * battery_percentage,
					u_short * battery_life);

#endif
