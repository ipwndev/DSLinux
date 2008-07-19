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
 * See http://embedded.centurysoftware.com/gpl/ for GPL licensing       
 * information.                                                         
 *                                                                      
 * See http://embedded.centurysoftware.com/license.html or              
 * email cetsales@centurysoftware.com for information about the PIXIL   
 * Commercial License Agreement, or if any conditions of this licensing 
 * are not clear to you.                                                
 */


#ifndef _PIXLIB_H_
#define _PIXLIB_H_

#include <sys/utsname.h>
#include <pixlib/timelib.h>

#ifdef		__cplusplus
extern "C"
{
#endif

    /* Typedef/Macro definitions */

#define			PIXLIB_STUB_VAL			-2

    /* Defines for HW support */
#define			PIX_HW_BACKLIGHT		0x0001
#define			PIX_HW_CALIBRATION		0x0002
#define			PIX_HW_POWER_MGMT		0x0004


    /* Defines for power management */
#define			PWR_BAT_PERCENT			1	/* Return value in percent */
#define			PWR_BAT_SECONDS			2	/* Return value in # of seconds left */

    /* Defines for Comm */

    /* Array Sizes */
#define PIX_COMM_IP_LEN		16

    /* PPP Types */
#define PIX_COMM_PPP_NULLMODEM   1
#define PIX_COMM_PPP_MODEM       2

#define PIX_COMM_PPP_AUTH        0x01
#define PIX_COMM_PPP_HWFLOW      0x02

#define PIX_COMM_TYPE_PPP        0x01
#define PIX_COMM_TYPE_ETHERNET   0x02
#define PIX_COMM_TYPE_ANY        0x03

    /* ERROR CODES */
#define PIX_COMM_OK               0
#define PIX_COMM_ERROR           -1
#define PIX_COMM_NOT_IMPLEMENTED -2
#define PIX_COMM_IPADDR_ERROR    -3
#define PIX_COMM_NO_INTERFACES   -4
#define PIX_COMM_ACTIVE          -5
#define PIX_COMM_INACTIVE        -6
#define PIX_COMM_INVALID         -7

    /* Internal codes, not normally used by the outside world */
    /* These are used for a IOCTL type of interaction between the API and */
    /* processor specific functions */

#define PIX_COMM_GET 1
#define PIX_COMM_SET 2
#define PIX_COMM_ADD 3
#define PIX_COMM_DEL 4

    /* Defines for Sys */

    /* Backlight power control */
#define PIX_SYS_BLITE_POWER_ON  1
#define PIX_SYS_BLITE_POWER_OFF 0

    /* PPP Flags */
#define PIX_SYS_PPP_NULLMODEM   1
#define PIX_SYS_PPP_MODEM       2

    /* From x86.h/ipaq.h network constants */
#ifdef DEBUG
#define ETH0_FILE		"~/ifcfg-eth0"
#define RESOLV_FILE 	"~/ifcfg-eth0"
#else
#define ETH0_FILE	"/etc/sysconfig/network-scripts/ifcfg-eth0"
#define RESOLV_FILE	"/etc/resolv.conf"
#endif

#define GATEWAY_FILE 	"/tmp/gateway"

    /* Battery info flags */
#define PIX_SYS_BATTERY_AC_OFFLINE 0x0
#define PIX_SYS_BATTERY_AC_ONLINE  0x1
#define PIX_SYS_BATTERY_ACTIVE     0x2
#define PIX_SYS_BATTERY_CHARGING   0x4

#define PIX_SYS_BATTERY_PRESENT    0x6	/* Active and/or charging */

    /* ERROR CODES */
#define PIX_SYS_OK              0
#define PIX_SYS_ERROR           1
#define PIX_SYS_NOT_IMPLEMENTED 2
#define PIX_SYS_BAD_VALUE       3
#define PIX_SYS_NOT_SUPERUSER   4
#define PIX_SYS_IPADDR_ERROR    5
#define PIX_SYS_FILE_ERROR      6

#define PIXIO_MONO 0
#define PIXIO_STEREO 1

#define PIXIO_DEFAULT_RATE 44100

#define PIXIO_WAV_FILE 0x01

#define PIXIO_PLAY   0x01
#define PIXIO_RECORD 0x02

/* Mixer devices */

/* These are equal to the Linux settings */
/* for other architectures, you need a   */
/* conversion macro to your settings     */

#define PIXIO_MIXER_COUNT  6

#define PIXIO_MIXER_VOLUME 0
#define PIXIO_MIXER_BASS   1
#define PIXIO_MIXER_TREBLE 2
#define PIXIO_MIXER_MIC    3
#define PIXIO_MIXER_LINE1  4
#define PIXIO_MIXER_LINE2  5

#define PIXIO_MIXER_LABELS {"Vol ", "Bass ", "Trebl", \
                           "Mic ", "Line1", "Line2"}

/* Error Messages */

#define PIXIO_OK               0
#define PIXIO_ERROR            -1
#define PIXIO_NOT_IMPLEMENTED  -2
#define PIXIO_BAD_DEVICE       -3
#define PIXIO_AGAIN            -4
#define PIXIO_FILE_TOO_BIG     -5

    /* Common pixlib structures */

    // FS List structure
    typedef struct nxFSlist_s
    {
	char name[15];
	char path[20];
    }
    nxFSlist;

    /* Comm STRUCTURES */
    typedef struct
    {
	char *addr;
	char *netmask;
	char *broadcast;
	char *subnet;
	char *gateway;
	char *essid;
	char *wepid;
	int dhcp;
    }
    pix_comm_ipaddr_str_t;

    typedef struct
    {
	const char *domain;
	const char *search;
	unsigned long long_dns_1;
	unsigned long long_dns_2;
	unsigned long long_dns_3;
	char *str_dns_1;
	char *str_dns_2;
	char *str_dns_3;
    }
    pix_comm_dns_t;


    typedef struct
    {
	unsigned long addr;
	unsigned long netmask;
	unsigned long broadcast;
    }
    pix_comm_ipaddr_t;

    typedef struct
    {
	char device[128];
	int speed;
	char telephone[12];
	char account[20];
	char password[20];
	unsigned long local_ipaddr;
	unsigned long remote_ipaddr;
	unsigned long netmask;
	unsigned long flags;
    }
    pix_comm_ppp_options_t;

    typedef struct
    {
	char name[15];
	unsigned char type;
	unsigned char active;
    }
    pix_comm_interface_t;

    typedef struct
    {
	unsigned long in_bytes;
	unsigned long out_bytes;
    }
    pix_comm_ppp_stats_t;

    typedef struct
    {
	char device[128];
	char ifname[25];
	char remote_ipaddr[20];
	char local_ipaddr[20];
	int speed;
	unsigned long start_time;
    }
    pix_comm_ppp_info_t;

    typedef struct
    {
	char name[16];
	unsigned short status;
	unsigned char quality;
	unsigned char level;
	unsigned char noise;
	struct
	{
	    unsigned long nwid;
	    unsigned long code;
	    unsigned long misc;
	}
	discard;
    }
    pix_comm_wirestats_t;

    /* io STRUCTURES */

    typedef struct
    {
	unsigned char left;
	unsigned char right;
    }
    pix_io_level_t;

    typedef struct
    {
	unsigned char stereo;
	unsigned char sample;
	int speed;
	unsigned long size;
	int blocksize;
    }
    pix_io_audio_t;

    /* sys STRUCTURES */

    typedef struct
    {
	char *addr;
	char *netmask;
	char *broadcast;
	char *subnet;
	char *gateway;
	int dhcp;
    }
    pix_sys_ipaddr_str_t;

    typedef struct
    {
	const char *domain;
	const char *search;
	unsigned long long_dns_1;
	unsigned long long_dns_2;
	unsigned long long_dns_3;
	char *str_dns_1;
	char *str_dns_2;
	char *str_dns_3;
    }
    pix_sys_dns_t;

    typedef struct
    {
	unsigned long addr;
	unsigned long netmask;
	unsigned long broadcast;
    }
    pix_sys_ipaddr_t;

    typedef struct
    {
	char device[128];
	int speed;
	char telephone[12];
	char account[20];
	char password[20];
	unsigned long local_ipaddr;
	unsigned long remote_ipaddr;
	unsigned long netmask;
    }
    pix_sys_ppp_options_t;

    /* Note:  The following two structures are pretty        */
    /* *NIX specific.  The reason being that most apps       */
    /* will want to know all of the available information    */
    /* If your platform isn't *NIX based, you can either     */
    /* pass dummy values for those unused fields, or use     */
    /* a IFDEF to define an alternate struct for your system */

    typedef struct
    {
	unsigned long user;	/* User CPU time */
	unsigned long nice;	/* CPU time of "nice" prodcesses */
	unsigned long system;	/* Total kernel CPU time */
	unsigned long idle;	/* Total idle time */
    }
    pix_sys_cpu_t;

    typedef struct
    {
	unsigned long total;	/* Total RAM */
	unsigned long used;	/* Total used RAM */
	unsigned long free;	/* Free RAM */
	unsigned long shared;	/* shared RAM (user & kernel) */
	unsigned long buffers;	/* RAM allocated to kernel buffers */
	unsigned long cached;	/* cached RAM */
	unsigned long stotal;	/* Total swap space */
	unsigned long sused;	/* Total swap used */
    }
    pix_sys_memory_t;

    typedef struct
    {
	unsigned char flags;
	unsigned char percent;
	unsigned short battery_life;
    }
    pix_sys_battery_t;

    typedef struct
    {
	short x,		/* X coordinate */
	  y;			/* Y coordintae */
    }
    CalPt_t;

    typedef struct
    {
	float mtotal,
	    mfree, mused, mshare, mbuffer, mcache, stotal, sused, sfree;
    }
    pixMemInfo_t;

    typedef struct
    {
	char cpu[80];
    }
    pixCpuInfo_t;

    typedef struct
    {
	char socket0[80], socket1[80];
    }
    pixPCMCIAInfo_t;


    /* System independent API calls */

    // FS List Functions
    char *nxFSgettwd(void);
    int nxFSgetfslist(nxFSlist **);

    /* System dependent API calls */

  /*******************************************************************************\
   **
   **	Function:	int pix_PDSupport()
   **	Desc:		Determines if hardware support is provided for the given capability
   **				on the specific platform
   **	Accepts:	int capability = Bitmask for the capabilities
   **	Returns:	int; 0 if not supported; non-zero (bitwise AND) value
   **
\*******************************************************************************/
    extern int pix_PDSupport(int capability);

    /* Backlight controls */

  /*******************************************************************************\
   **
   **	Function:	int pix_bl_ctrl()
   **	Desc:		Controls the backlite settings
   **	Accepts:	int pwr = Power flag (0 = off; 1 = on)
   **				int level = level flag (0 - f_maxval) -- will clamp to f_maxval
   **	Returns:	int; 0 on success, -1 on error (errno will be set)
   **
\*******************************************************************************/
    extern int pix_bl_ctrl(int pwr, int level);

  /*******************************************************************************\
   **
   **	Function:	int pix_bl_getmxval()
   **	Desc:		Returns the maximum backlight value supported by the hardware for
   **				the given platform
   **	Accepts:	Nothing (void)
   **	Returns:	int >= 0 on success -1 for unknown
   **
\*******************************************************************************/
    extern int pix_bl_getmxval();

    /* Power management calls */

  /*******************************************************************************\
   **
   **	Function:	int pix_pwr_getbat()
   **	Desc:		Returns the current battery level from the power management system
   **	Accepts:	int flags = Flag indicating how to return the value:
   **					PWR_BAT_PERCENT = returns value as a percent
   **					PWR_BAT_SECONDS = returns value as number of seconds remaining
   **	Returns:	int; >= 0 (amount left), -1 for UNKNOWN, PIXLIB_STUB_VAL for
   **					a stub function
   **
\*******************************************************************************/
    extern int pix_pwr_getbat(int flags);

  /*******************************************************************************\
   **
   **	Function:	int pix_pwr_isCharging()
   **	Desc:		Determines if the battery is being charged
   **	Accepts:	Nothing (void)
   **	Returns:	int; 0 for not charging, 1 if charging, -1 on unknown, PIXLIB_STUB_VAL
   **				if stubbed
   **
\*******************************************************************************/
    extern int pix_pwr_isCharging(void);

  /*******************************************************************************\
   **
   **	Function:	int pix_pwr_onBattery()
   **	Desc:		Determines if device is using  the battery power vs external(line)
   **	Accepts:	Nothing (void)
   **	Returns:	int; 0 if on line power, 1 on battery power, -1 on unknown,
   **					PIXLIB_STUB_VAL if stubbed
   **
\*******************************************************************************/
    extern int pix_pwr_onBattery(void);

  /*******************************************************************************\
   **
   **	Function:	int pix_pwr_suspend()
   **	Desc:	 suspend the device	
   **	Accepts:	Nothing (void)
   **	Returns: returns 0 on succes -1 on failure	
   **					PIXLIB_STUB_VAL if stubbed
   **
\*******************************************************************************/
    extern int pix_pwr_suspend(void);

/* Calibration calls */

/*******************************************************************************\
**
**	Function:	int pix_cal_Calibrate()
**	Desc:		Handles the calibration details for a given platform based on
**				the control data and user-speicified data.
**	Accepts:	int npts = Number of pts to use in calculations
**				CalPt_t *ctrldata = Array of npts control elements
**				CalPt_t *userdata = Array of npts user-specified elements
**	Returns:	int; 0 on success (i.e. kernel was notified), or -1 on error
**
\*******************************************************************************/
    extern int pix_cal_Calibrate(int npts, CalPt_t * ctrldata,
				 CalPt_t * userdata);

/*******************************************************************************\
**
**	Function:	int pix_cal_GetCtrlPts()
**	Desc:		Gets the device-specific control points for calibration
**	Accepts:	int *npts = Ptr to return the number of points (and size of array)
**				CalPt_t **ctrldata = Ptr to a ptr for dynamic memory allocation...
**	Returns:	int; 0 on success, -1 on error
**
\*******************************************************************************/
    extern int pix_cal_GetCtrlPts(int *npts, CalPt_t ** ctrldata, int w,
				  int h, int bpp);

/*******************************************************************************\
**
**	Function:	int pix_cal_GetDrawPt()
**	Desc:		Determines if the drawing point needs to be altered in any way
**	Accepts:	CalPt_t *ctrldata = Ptr to the control point
**				CalPt_t *drawdata = Ptr to the storage for the draw point
**	Returns:	int; 0 on success, -1 on error
**
\*******************************************************************************/
    extern int pix_cal_GetDrawPt(CalPt_t * ctrldata, CalPt_t * drawdata);

/*******************************************************************************\
**
**	Function:	int pix_cal_GetDataPt()
**	Desc:		Retrieves the data point from the touch-screen device, in 
**				response to a calibration point tap.
**	Accepts:	CalPt_t *ptdata = Storage to return the points
**	Returns:	int; 0 on success, -1 on error
**
\*******************************************************************************/
    extern int pix_cal_GetDataPt(CalPt_t * ptdata);


/******************/
/* Comm functions */
/******************/

    int pix_comm_set_netscript_values(char *file, const char *keys[],
				      const char *vals[], int size);
    int pix_comm_get_netscript_values(char *file, char *keys[], char *vals[],
				      int size);
    int pix_comm_set_ip_address(char *ifr, pix_comm_ipaddr_t *);
    int pix_comm_get_ip_address(char *ifr, pix_comm_ipaddr_t *);
    int pix_comm_add_default_gateway(unsigned long addr, char *interface);
    int pix_comm_remove_default_gateway();
    int pix_comm_get_default_gateway(unsigned long *addr);

    int pix_comm_if_up(char *ifname, int dhcp);
    int pix_comm_if_down(char *ifname);

    int pix_comm_set_nameserver(char *, char *, unsigned long *, int);

    int pix_comm_get_if_list(int, pix_comm_interface_t *, int *);

    // Wireless
    ///////////

    /* Wireless Active? */
    int pix_comm_wireless_get_name(char *interface, char *name);

    /* WEP */
    int pix_comm_wireless_set_encode(char *interface, char *name);
    int pix_comm_wireless_get_encode(char *interface, char *name);

    int pix_comm_wireless_set_essid(char *interface, char *name);
    int pix_comm_wireless_get_essid(char *interface, char *name);

    int pix_comm_wireless_get_stats(char *interface,
				    pix_comm_wirestats_t * stats);
    int pix_comm_wireless_get_if_list(pix_comm_interface_t *, int *);

    int pix_comm_ppp_connect(int, pix_comm_ppp_options_t *, char **, int, int,
			     char *);

    int pix_comm_ppp_disconnect(char *);
    int pix_comm_if_active(char *);
    int pix_comm_ppp_get_stats(char *, pix_comm_ppp_stats_t *);
    int pix_comm_ppp_get_ip_info(char *, pix_comm_ppp_options_t *);

    int pix_comm_ppp_write_info(char *filename,
				pix_comm_ppp_info_t * pppinfo);
    int pix_comm_ppp_read_info(char *filename, pix_comm_ppp_info_t * pppinfo);

/* These are macros that abstract the ppp_connect call */
#define pix_comm_dialup_ppp_connect(opts, cmds, count) \
                                 (pix_comm_ppp_connect(PIX_COMM_PPP_MODEM, opts, cmds, count))

#define pix_comm_direct_ppp_connect(opts) \
                                 (pix_comm_ppp_connect(PIX_COMM_PPP_NULLMODEM, opts, 0, 0))

  /*****************/
    /* sys functions */
  /*****************/

/* FUNCTION PROTOTYPES */

/* SYSTEM SETTINGS */
    int pix_sys_set_date(pix_sys_date_t *);
    int pix_sys_get_date(pix_sys_date_t *);

/* NETWORKING SETTINGS */
    int pix_sys_set_ip_address(char *ifr, pix_sys_ipaddr_t *);
    int pix_sys_get_ip_address(char *ifr, pix_sys_ipaddr_t *);
    int pix_sys_get_net_value(char *, char *);
    int pix_sys_write_net_values(pix_sys_ipaddr_str_t, pix_sys_dns_t);
    int pix_sys_add_default_gateway(unsigned long addr);
    int pix_sys_remove_default_gateway();
    int pix_sys_ppp_connect(int, pix_sys_ppp_options_t *, char **, int);
    int pix_sys_ppp_disconnect();

/* POWER MANAGEMENT */
    int pix_sys_set_backlight(unsigned char, unsigned char);

/* STATISTICS */
    int pix_sys_get_cpu_load(pix_sys_cpu_t *);
    int pix_sys_get_memory_usage(pix_sys_memory_t *);
    int pix_sys_get_battery(pix_sys_battery_t *);

/* System info (osinfo, cpuinfo, meminfo, pcmciainfo) */
    int pix_sys_osinfo(struct utsname *punm);	/* Common api */
    int pix_sys_cpuinfo(pixCpuInfo_t * pcpu);
    int pix_sys_meminfo(pixMemInfo_t * pmi);
    int pix_sys_pcmciainfo(pixPCMCIAInfo_t * ppi);

    /* From x86.h/ipaq.h network functions */
    int pix_set_nameserver(char *, char *, unsigned long *, int);
    int pix_write_net_values(pix_sys_ipaddr_str_t, pix_sys_dns_t);
    int pix_get_net_value(char *, char *);

/* Pixil io functions */
    int pix_io_play_soundfile(int type, char *filename);

    int pix_io_save_sound_file(int type, char *filename,
			       pix_io_audio_t * settings,
			       unsigned char *buffer);

    int pix_io_load_sound_file(int type, char *filename,
			       pix_io_audio_t * settings,
			       unsigned char *buffer, int size);

    int pix_io_get_wav_stats(char *filename, pix_io_audio_t * settings);

    int pix_io_open_stream(int direction, pix_io_audio_t * settings);
    int pix_io_stream_record(int fd, unsigned char *buffer, int size);
    int pix_io_stream_play(int fd, unsigned char *buffer, int size);

    int pix_io_get_mixer_devices(unsigned long *bitmask);

    int pix_io_set_mixer_level(int device, pix_io_level_t * level);
    int pix_io_get_mixer_level(int device, pix_io_level_t * level);

#define pix_io_set_volume(level) \
        pix_io_set_mixer_level(PIXIO_MIXER_VOLUME, level)

#define pix_io_get_volume(level) \
        pix_io_get_mixer_level(PIXIO_MIXER_VOLUME, level)
#ifdef		__cplusplus
}
#endif

#endif
