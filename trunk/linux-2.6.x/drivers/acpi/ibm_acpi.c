/*
 *  ibm_acpi.c - IBM ThinkPad ACPI Extras
 *
 *
 *  Copyright (C) 2004-2005 Borislav Deianov <borislav@users.sf.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define IBM_VERSION "0.12a"

/*
 *  Changelog:
 *  
 *  2005-08-17  0.12	fix compilation on 2.6.13-rc kernels
 *  2005-03-17	0.11	support for 600e, 770x
 *			    thanks to Jamie Lentin <lentinj@dial.pipex.com>
 *			support for 770e, G41
 *			G40 and G41 don't have a thinklight
 *			temperatures no longer experimental
 *			experimental brightness control
 *			experimental volume control
 *			experimental fan enable/disable
 *  2005-01-16	0.10	fix module loading on R30, R31 
 *  2005-01-16	0.9	support for 570, R30, R31
 *			ultrabay support on A22p, A3x
 *			limit arg for cmos, led, beep, drop experimental status
 *			more capable led control on A21e, A22p, T20-22, X20
 *			experimental temperatures and fan speed
 *			experimental embedded controller register dump
 *			mark more functions as __init, drop incorrect __exit
 *			use MODULE_VERSION
 *			    thanks to Henrik Brix Andersen <brix@gentoo.org>
 *			fix parameter passing on module loading
 *			    thanks to Rusty Russell <rusty@rustcorp.com.au>
 *			    thanks to Jim Radford <radford@blackbean.org>
 *  2004-11-08	0.8	fix init error case, don't return from a macro
 *			    thanks to Chris Wright <chrisw@osdl.org>
 *  2004-10-23	0.7	fix module loading on A21e, A22p, T20, T21, X20
 *			fix led control on A21e
 *  2004-10-19	0.6	use acpi_bus_register_driver() to claim HKEY device
 *  2004-10-18	0.5	thinklight support on A21e, G40, R32, T20, T21, X20
 *			proc file format changed
 *			video_switch command
 *			experimental cmos control
 *			experimental led control
 *			experimental acpi sounds
 *  2004-09-16	0.4	support for module parameters
 *			hotkey mask can be prefixed by 0x
 *			video output switching
 *			video expansion control
 *			ultrabay eject support
 *			removed lcd brightness/on/off control, didn't work
 *  2004-08-17	0.3	support for R40
 *			lcd off, brightness control
 *			thinklight on/off
 *  2004-08-14	0.2	support for T series, X20
 *			bluetooth enable/disable
 *			hotkey events disabled by default
 *			removed fan control, currently useless
 *  2004-08-09	0.1	initial release, support for X series
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include <acpi/acpi_drivers.h>
#include <acpi/acnamesp.h>

#define IBM_NAME "ibm"
#define IBM_DESC "IBM ThinkPad ACPI Extras"
#define IBM_FILE "ibm_acpi"
#define IBM_URL "http://ibm-acpi.sf.net/"

MODULE_AUTHOR("Borislav Deianov");
MODULE_DESCRIPTION(IBM_DESC);
MODULE_VERSION(IBM_VERSION);
MODULE_LICENSE("GPL");

#define IBM_DIR IBM_NAME

#define IBM_LOG IBM_FILE ": "
#define IBM_ERR	   KERN_ERR    IBM_LOG
#define IBM_NOTICE KERN_NOTICE IBM_LOG
#define IBM_INFO   KERN_INFO   IBM_LOG
#define IBM_DEBUG  KERN_DEBUG  IBM_LOG

#define IBM_MAX_ACPI_ARGS 3

#define __unused __attribute__ ((unused))

static int experimental;
module_param(experimental, int, 0);

static acpi_handle root_handle = NULL;

#define IBM_HANDLE(object, parent, paths...)			\
	static acpi_handle  object##_handle;			\
	static acpi_handle *object##_parent = &parent##_handle;	\
	static char        *object##_path;			\
	static char        *object##_paths[] = { paths }

/*
 * The following models are supported to various degrees:
 *
 * 570, 600e, 600x, 770e, 770x
 * A20m, A21e, A21m, A21p, A22p, A30, A30p, A31, A31p
 * G40, G41
 * R30, R31, R32, R40, R40e, R50, R50e, R50p, R51
 * T20, T21, T22, T23, T30, T40, T40p, T41, T41p, T42, T42p, T43
 * X20, X21, X22, X23, X24, X30, X31, X40
 *
 * The following models have no supported features:
 *
 * 240, 240x, i1400
 *
 * Still missing DSDTs for the following models:
 *
 * A20p, A22e, A22m
 * R52
 * S31
 * T43p
 */

IBM_HANDLE(ec, root, "\\_SB.PCI0.ISA.EC0",	/* 240, 240x */
	   "\\_SB.PCI.ISA.EC",	/* 570 */
	   "\\_SB.PCI0.ISA0.EC0",	/* 600e/x, 770e, 770x */
	   "\\_SB.PCI0.ISA.EC",	/* A21e, A2xm/p, T20-22, X20-21 */
	   "\\_SB.PCI0.AD4S.EC0",	/* i1400, R30 */
	   "\\_SB.PCI0.ICH3.EC0",	/* R31 */
	   "\\_SB.PCI0.LPC.EC",	/* all others */
    );

IBM_HANDLE(vid, root, "\\_SB.PCI.AGP.VGA",	/* 570 */
	   "\\_SB.PCI0.AGP0.VID0",	/* 600e/x, 770x */
	   "\\_SB.PCI0.VID0",	/* 770e */
	   "\\_SB.PCI0.VID",	/* A21e, G4x, R50e, X30, X40 */
	   "\\_SB.PCI0.AGP.VID",	/* all others */
    );				/* R30, R31 */

IBM_HANDLE(vid2, root, "\\_SB.PCI0.AGPB.VID");	/* G41 */

IBM_HANDLE(cmos, root, "\\UCMS",	/* R50, R50e, R50p, R51, T4x, X31, X40 */
	   "\\CMOS",		/* A3x, G4x, R32, T23, T30, X22-24, X30 */
	   "\\CMS",		/* R40, R40e */
    );				/* all others */

IBM_HANDLE(dock, root, "\\_SB.GDCK",	/* X30, X31, X40 */
	   "\\_SB.PCI0.DOCK",	/* 600e/x,770e,770x,A2xm/p,T20-22,X20-21 */
	   "\\_SB.PCI0.PCI1.DOCK",	/* all others */
	   "\\_SB.PCI.ISA.SLCE",	/* 570 */
    );				/* A21e,G4x,R30,R31,R32,R40,R40e,R50e */

IBM_HANDLE(bay, root, "\\_SB.PCI.IDE.SECN.MAST",	/* 570 */
	   "\\_SB.PCI0.IDE0.IDES.IDSM",	/* 600e/x, 770e, 770x */
	   "\\_SB.PCI0.IDE0.SCND.MSTR",	/* all others */
    );				/* A21e, R30, R31 */

IBM_HANDLE(bay_ej, bay, "_EJ3",	/* 600e/x, A2xm/p, A3x */
	   "_EJ0",		/* all others */
    );				/* 570,A21e,G4x,R30,R31,R32,R40e,R50e */

IBM_HANDLE(bay2, root, "\\_SB.PCI0.IDE0.PRIM.SLAV",	/* A3x, R32 */
	   "\\_SB.PCI0.IDE0.IDEP.IDPS",	/* 600e/x, 770e, 770x */
    );				/* all others */

IBM_HANDLE(bay2_ej, bay2, "_EJ3",	/* 600e/x, 770e, A3x */
	   "_EJ0",		/* 770x */
    );				/* all others */

/* don't list other alternatives as we install a notify handler on the 570 */
IBM_HANDLE(pci, root, "\\_SB.PCI");	/* 570 */

IBM_HANDLE(hkey, ec, "\\_SB.HKEY",	/* 600e/x, 770e, 770x */
	   "^HKEY",		/* R30, R31 */
	   "HKEY",		/* all others */
    );				/* 570 */

IBM_HANDLE(lght, root, "\\LGHT");	/* A21e, A2xm/p, T20-22, X20-21 */
IBM_HANDLE(ledb, ec, "LEDB");	/* G4x */

IBM_HANDLE(led, ec, "SLED",	/* 570 */
	   "SYSL",		/* 600e/x, 770e, 770x, A21e, A2xm/p, T20-22, X20-21 */
	   "LED",		/* all others */
    );				/* R30, R31 */

IBM_HANDLE(beep, ec, "BEEP");	/* all except R30, R31 */
IBM_HANDLE(ecrd, ec, "ECRD");	/* 570 */
IBM_HANDLE(ecwr, ec, "ECWR");	/* 570 */
IBM_HANDLE(fans, ec, "FANS");	/* X31, X40 */

IBM_HANDLE(gfan, ec, "GFAN",	/* 570 */
	   "\\FSPD",		/* 600e/x, 770e, 770x */
    );				/* all others */

IBM_HANDLE(sfan, ec, "SFAN",	/* 570 */
	   "JFNS",		/* 770x-JL */
    );				/* all others */

#define IBM_HKEY_HID	"IBM0068"
#define IBM_PCI_HID	"PNP0A03"

struct ibm_struct {
	char *name;
	char param[32];

	char *hid;
	struct acpi_driver *driver;

	int (*init) (void);
	int (*read) (char *);
	int (*write) (char *);
	void (*exit) (void);

	void (*notify) (struct ibm_struct *, u32);
	acpi_handle *handle;
	int type;
	struct acpi_device *device;

	int driver_registered;
	int proc_created;
	int init_called;
	int notify_installed;

	int experimental;
};

static struct proc_dir_entry *proc_dir = NULL;

#define onoff(status,bit) ((status) & (1 << (bit)) ? "on" : "off")
#define enabled(status,bit) ((status) & (1 << (bit)) ? "enabled" : "disabled")
#define strlencmp(a,b) (strncmp((a), (b), strlen(b)))

static int acpi_evalf(acpi_handle handle,
		      void *res, char *method, char *fmt, ...)
{
	char *fmt0 = fmt;
	struct acpi_object_list params;
	union acpi_object in_objs[IBM_MAX_ACPI_ARGS];
	struct acpi_buffer result, *resultp;
	union acpi_object out_obj;
	acpi_status status;
	va_list ap;
	char res_type;
	int success;
	int quiet;

	if (!*fmt) {
		printk(IBM_ERR "acpi_evalf() called with empty format\n");
		return 0;
	}

	if (*fmt == 'q') {
		quiet = 1;
		fmt++;
	} else
		quiet = 0;

	res_type = *(fmt++);

	params.count = 0;
	params.pointer = &in_objs[0];

	va_start(ap, fmt);
	while (*fmt) {
		char c = *(fmt++);
		switch (c) {
		case 'd':	/* int */
			in_objs[params.count].integer.value = va_arg(ap, int);
			in_objs[params.count++].type = ACPI_TYPE_INTEGER;
			break;
			/* add more types as needed */
		default:
			printk(IBM_ERR "acpi_evalf() called "
			       "with invalid format character '%c'\n", c);
			return 0;
		}
	}
	va_end(ap);

	if (res_type != 'v') {
		result.length = sizeof(out_obj);
		result.pointer = &out_obj;
		resultp = &result;
	} else
		resultp = NULL;

	status = acpi_evaluate_object(handle, method, &params, resultp);

	switch (res_type) {
	case 'd':		/* int */
		if (res)
			*(int *)res = out_obj.integer.value;
		success = status == AE_OK && out_obj.type == ACPI_TYPE_INTEGER;
		break;
	case 'v':		/* void */
		success = status == AE_OK;
		break;
		/* add more types as needed */
	default:
		printk(IBM_ERR "acpi_evalf() called "
		       "with invalid format character '%c'\n", res_type);
		return 0;
	}

	if (!success && !quiet)
		printk(IBM_ERR "acpi_evalf(%s, %s, ...) failed: %d\n",
		       method, fmt0, status);

	return success;
}

static void __unused acpi_print_int(acpi_handle handle, char *method)
{
	int i;

	if (acpi_evalf(handle, &i, method, "d"))
		printk(IBM_INFO "%s = 0x%x\n", method, i);
	else
		printk(IBM_ERR "error calling %s\n", method);
}

static char *next_cmd(char **cmds)
{
	char *start = *cmds;
	char *end;

	while ((end = strchr(start, ',')) && end == start)
		start = end + 1;

	if (!end)
		return NULL;

	*end = 0;
	*cmds = end + 1;
	return start;
}

static int driver_init(void)
{
	printk(IBM_INFO "%s v%s\n", IBM_DESC, IBM_VERSION);
	printk(IBM_INFO "%s\n", IBM_URL);

	return 0;
}

static int driver_read(char *p)
{
	int len = 0;

	len += sprintf(p + len, "driver:\t\t%s\n", IBM_DESC);
	len += sprintf(p + len, "version:\t%s\n", IBM_VERSION);

	return len;
}

static int hotkey_supported;
static int hotkey_mask_supported;
static int hotkey_orig_status;
static int hotkey_orig_mask;

static int hotkey_get(int *status, int *mask)
{
	if (!acpi_evalf(hkey_handle, status, "DHKC", "d"))
		return 0;

	if (hotkey_mask_supported)
		if (!acpi_evalf(hkey_handle, mask, "DHKN", "d"))
			return 0;

	return 1;
}

static int hotkey_set(int status, int mask)
{
	int i;

	if (!acpi_evalf(hkey_handle, NULL, "MHKC", "vd", status))
		return 0;

	if (hotkey_mask_supported)
		for (i = 0; i < 32; i++) {
			int bit = ((1 << i) & mask) != 0;
			if (!acpi_evalf(hkey_handle,
					NULL, "MHKM", "vdd", i + 1, bit))
				return 0;
		}

	return 1;
}

static int hotkey_init(void)
{
	/* hotkey not supported on 570 */
	hotkey_supported = hkey_handle != NULL;

	if (hotkey_supported) {
		/* mask not supported on 570, 600e/x, 770e, 770x, A21e, A2xm/p,
		   A30, R30, R31, T20-22, X20-21, X22-24 */
		hotkey_mask_supported =
		    acpi_evalf(hkey_handle, NULL, "DHKN", "qv");

		if (!hotkey_get(&hotkey_orig_status, &hotkey_orig_mask))
			return -ENODEV;
	}

	return 0;
}

static int hotkey_read(char *p)
{
	int status, mask;
	int len = 0;

	if (!hotkey_supported) {
		len += sprintf(p + len, "status:\t\tnot supported\n");
		return len;
	}

	if (!hotkey_get(&status, &mask))
		return -EIO;

	len += sprintf(p + len, "status:\t\t%s\n", enabled(status, 0));
	if (hotkey_mask_supported) {
		len += sprintf(p + len, "mask:\t\t0x%04x\n", mask);
		len += sprintf(p + len,
			       "commands:\tenable, disable, reset, <mask>\n");
	} else {
		len += sprintf(p + len, "mask:\t\tnot supported\n");
		len += sprintf(p + len, "commands:\tenable, disable, reset\n");
	}

	return len;
}

static int hotkey_write(char *buf)
{
	int status, mask;
	char *cmd;
	int do_cmd = 0;

	if (!hotkey_supported)
		return -ENODEV;

	if (!hotkey_get(&status, &mask))
		return -EIO;

	while ((cmd = next_cmd(&buf))) {
		if (strlencmp(cmd, "enable") == 0) {
			status = 1;
		} else if (strlencmp(cmd, "disable") == 0) {
			status = 0;
		} else if (strlencmp(cmd, "reset") == 0) {
			status = hotkey_orig_status;
			mask = hotkey_orig_mask;
		} else if (sscanf(cmd, "0x%x", &mask) == 1) {
			/* mask set */
		} else if (sscanf(cmd, "%x", &mask) == 1) {
			/* mask set */
		} else
			return -EINVAL;
		do_cmd = 1;
	}

	if (do_cmd && !hotkey_set(status, mask))
		return -EIO;

	return 0;
}

static void hotkey_exit(void)
{
	if (hotkey_supported)
		hotkey_set(hotkey_orig_status, hotkey_orig_mask);
}

static void hotkey_notify(struct ibm_struct *ibm, u32 event)
{
	int hkey;

	if (acpi_evalf(hkey_handle, &hkey, "MHKP", "d"))
		acpi_bus_generate_event(ibm->device, event, hkey);
	else {
		printk(IBM_ERR "unknown hotkey event %d\n", event);
		acpi_bus_generate_event(ibm->device, event, 0);
	}
}

static int bluetooth_supported;

static int bluetooth_init(void)
{
	/* bluetooth not supported on 570, 600e/x, 770e, 770x, A21e, A2xm/p,
	   G4x, R30, R31, R40e, R50e, T20-22, X20-21 */
	bluetooth_supported = hkey_handle &&
	    acpi_evalf(hkey_handle, NULL, "GBDC", "qv");

	return 0;
}

static int bluetooth_status(void)
{
	int status;

	if (!bluetooth_supported ||
	    !acpi_evalf(hkey_handle, &status, "GBDC", "d"))
		status = 0;

	return status;
}

static int bluetooth_read(char *p)
{
	int len = 0;
	int status = bluetooth_status();

	if (!bluetooth_supported)
		len += sprintf(p + len, "status:\t\tnot supported\n");
	else if (!(status & 1))
		len += sprintf(p + len, "status:\t\tnot installed\n");
	else {
		len += sprintf(p + len, "status:\t\t%s\n", enabled(status, 1));
		len += sprintf(p + len, "commands:\tenable, disable\n");
	}

	return len;
}

static int bluetooth_write(char *buf)
{
	int status = bluetooth_status();
	char *cmd;
	int do_cmd = 0;

	if (!bluetooth_supported)
		return -ENODEV;

	while ((cmd = next_cmd(&buf))) {
		if (strlencmp(cmd, "enable") == 0) {
			status |= 2;
		} else if (strlencmp(cmd, "disable") == 0) {
			status &= ~2;
		} else
			return -EINVAL;
		do_cmd = 1;
	}

	if (do_cmd && !acpi_evalf(hkey_handle, NULL, "SBDC", "vd", status))
		return -EIO;

	return 0;
}

static int video_supported;
static int video_orig_autosw;

#define VIDEO_570 1
#define VIDEO_770 2
#define VIDEO_NEW 3

static int video_init(void)
{
	int ivga;

	if (vid2_handle && acpi_evalf(NULL, &ivga, "\\IVGA", "d") && ivga)
		/* G41, assume IVGA doesn't change */
		vid_handle = vid2_handle;

	if (!vid_handle)
		/* video switching not supported on R30, R31 */
		video_supported = 0;
	else if (acpi_evalf(vid_handle, &video_orig_autosw, "SWIT", "qd"))
		/* 570 */
		video_supported = VIDEO_570;
	else if (acpi_evalf(vid_handle, &video_orig_autosw, "^VADL", "qd"))
		/* 600e/x, 770e, 770x */
		video_supported = VIDEO_770;
	else
		/* all others */
		video_supported = VIDEO_NEW;

	return 0;
}

static int video_status(void)
{
	int status = 0;
	int i;

	if (video_supported == VIDEO_570) {
		if (acpi_evalf(NULL, &i, "\\_SB.PHS", "dd", 0x87))
			status = i & 3;
	} else if (video_supported == VIDEO_770) {
		if (acpi_evalf(NULL, &i, "\\VCDL", "d"))
			status |= 0x01 * i;
		if (acpi_evalf(NULL, &i, "\\VCDC", "d"))
			status |= 0x02 * i;
	} else if (video_supported == VIDEO_NEW) {
		acpi_evalf(NULL, NULL, "\\VUPS", "vd", 1);
		if (acpi_evalf(NULL, &i, "\\VCDC", "d"))
			status |= 0x02 * i;

		acpi_evalf(NULL, NULL, "\\VUPS", "vd", 0);
		if (acpi_evalf(NULL, &i, "\\VCDL", "d"))
			status |= 0x01 * i;
		if (acpi_evalf(NULL, &i, "\\VCDD", "d"))
			status |= 0x08 * i;
	}

	return status;
}

static int video_autosw(void)
{
	int autosw = 0;

	if (video_supported == VIDEO_570)
		acpi_evalf(vid_handle, &autosw, "SWIT", "d");
	else if (video_supported == VIDEO_770 || video_supported == VIDEO_NEW)
		acpi_evalf(vid_handle, &autosw, "^VDEE", "d");

	return autosw & 1;
}

static int video_read(char *p)
{
	int status = video_status();
	int autosw = video_autosw();
	int len = 0;

	if (!video_supported) {
		len += sprintf(p + len, "status:\t\tnot supported\n");
		return len;
	}

	len += sprintf(p + len, "status:\t\tsupported\n");
	len += sprintf(p + len, "lcd:\t\t%s\n", enabled(status, 0));
	len += sprintf(p + len, "crt:\t\t%s\n", enabled(status, 1));
	if (video_supported == VIDEO_NEW)
		len += sprintf(p + len, "dvi:\t\t%s\n", enabled(status, 3));
	len += sprintf(p + len, "auto:\t\t%s\n", enabled(autosw, 0));
	len += sprintf(p + len, "commands:\tlcd_enable, lcd_disable\n");
	len += sprintf(p + len, "commands:\tcrt_enable, crt_disable\n");
	if (video_supported == VIDEO_NEW)
		len += sprintf(p + len, "commands:\tdvi_enable, dvi_disable\n");
	len += sprintf(p + len, "commands:\tauto_enable, auto_disable\n");
	len += sprintf(p + len, "commands:\tvideo_switch, expand_toggle\n");

	return len;
}

static int video_switch(void)
{
	int autosw = video_autosw();
	int ret;

	if (!acpi_evalf(vid_handle, NULL, "_DOS", "vd", 1))
		return -EIO;
	ret = video_supported == VIDEO_570 ?
	    acpi_evalf(ec_handle, NULL, "_Q16", "v") :
	    acpi_evalf(vid_handle, NULL, "VSWT", "v");
	acpi_evalf(vid_handle, NULL, "_DOS", "vd", autosw);

	return ret;
}

static int video_expand(void)
{
	if (video_supported == VIDEO_570)
		return acpi_evalf(ec_handle, NULL, "_Q17", "v");
	else if (video_supported == VIDEO_770)
		return acpi_evalf(vid_handle, NULL, "VEXP", "v");
	else
		return acpi_evalf(NULL, NULL, "\\VEXP", "v");
}

static int video_switch2(int status)
{
	int ret;

	if (video_supported == VIDEO_570) {
		ret = acpi_evalf(NULL, NULL,
				 "\\_SB.PHS2", "vdd", 0x8b, status | 0x80);
	} else if (video_supported == VIDEO_770) {
		int autosw = video_autosw();
		if (!acpi_evalf(vid_handle, NULL, "_DOS", "vd", 1))
			return -EIO;

		ret = acpi_evalf(vid_handle, NULL,
				 "ASWT", "vdd", status * 0x100, 0);

		acpi_evalf(vid_handle, NULL, "_DOS", "vd", autosw);
	} else {
		ret = acpi_evalf(NULL, NULL, "\\VUPS", "vd", 0x80) &&
		    acpi_evalf(NULL, NULL, "\\VSDS", "vdd", status, 1);
	}

	return ret;
}

static int video_write(char *buf)
{
	char *cmd;
	int enable, disable, status;

	if (!video_supported)
		return -ENODEV;

	enable = disable = 0;

	while ((cmd = next_cmd(&buf))) {
		if (strlencmp(cmd, "lcd_enable") == 0) {
			enable |= 0x01;
		} else if (strlencmp(cmd, "lcd_disable") == 0) {
			disable |= 0x01;
		} else if (strlencmp(cmd, "crt_enable") == 0) {
			enable |= 0x02;
		} else if (strlencmp(cmd, "crt_disable") == 0) {
			disable |= 0x02;
		} else if (video_supported == VIDEO_NEW &&
			   strlencmp(cmd, "dvi_enable") == 0) {
			enable |= 0x08;
		} else if (video_supported == VIDEO_NEW &&
			   strlencmp(cmd, "dvi_disable") == 0) {
			disable |= 0x08;
		} else if (strlencmp(cmd, "auto_enable") == 0) {
			if (!acpi_evalf(vid_handle, NULL, "_DOS", "vd", 1))
				return -EIO;
		} else if (strlencmp(cmd, "auto_disable") == 0) {
			if (!acpi_evalf(vid_handle, NULL, "_DOS", "vd", 0))
				return -EIO;
		} else if (strlencmp(cmd, "video_switch") == 0) {
			if (!video_switch())
				return -EIO;
		} else if (strlencmp(cmd, "expand_toggle") == 0) {
			if (!video_expand())
				return -EIO;
		} else
			return -EINVAL;
	}

	if (enable || disable) {
		status = (video_status() & 0x0f & ~disable) | enable;
		if (!video_switch2(status))
			return -EIO;
	}

	return 0;
}

static void video_exit(void)
{
	acpi_evalf(vid_handle, NULL, "_DOS", "vd", video_orig_autosw);
}

static int light_supported;
static int light_status_supported;

static int light_init(void)
{
	/* light not supported on 570, 600e/x, 770e, 770x, G4x, R30, R31 */
	light_supported = (cmos_handle || lght_handle) && !ledb_handle;

	if (light_supported)
		/* light status not supported on
		   570, 600e/x, 770e, 770x, G4x, R30, R31, R32, X20 */
		light_status_supported = acpi_evalf(ec_handle, NULL,
						    "KBLT", "qv");

	return 0;
}

static int light_read(char *p)
{
	int len = 0;
	int status = 0;

	if (!light_supported) {
		len += sprintf(p + len, "status:\t\tnot supported\n");
	} else if (!light_status_supported) {
		len += sprintf(p + len, "status:\t\tunknown\n");
		len += sprintf(p + len, "commands:\ton, off\n");
	} else {
		if (!acpi_evalf(ec_handle, &status, "KBLT", "d"))
			return -EIO;
		len += sprintf(p + len, "status:\t\t%s\n", onoff(status, 0));
		len += sprintf(p + len, "commands:\ton, off\n");
	}

	return len;
}

static int light_write(char *buf)
{
	int cmos_cmd, lght_cmd;
	char *cmd;
	int success;

	if (!light_supported)
		return -ENODEV;

	while ((cmd = next_cmd(&buf))) {
		if (strlencmp(cmd, "on") == 0) {
			cmos_cmd = 0x0c;
			lght_cmd = 1;
		} else if (strlencmp(cmd, "off") == 0) {
			cmos_cmd = 0x0d;
			lght_cmd = 0;
		} else
			return -EINVAL;

		success = cmos_handle ?
		    acpi_evalf(cmos_handle, NULL, NULL, "vd", cmos_cmd) :
		    acpi_evalf(lght_handle, NULL, NULL, "vd", lght_cmd);
		if (!success)
			return -EIO;
	}

	return 0;
}

static int _sta(acpi_handle handle)
{
	int status;

	if (!handle || !acpi_evalf(handle, &status, "_STA", "d"))
		status = 0;

	return status;
}

#define dock_docked() (_sta(dock_handle) & 1)

static int dock_read(char *p)
{
	int len = 0;
	int docked = dock_docked();

	if (!dock_handle)
		len += sprintf(p + len, "status:\t\tnot supported\n");
	else if (!docked)
		len += sprintf(p + len, "status:\t\tundocked\n");
	else {
		len += sprintf(p + len, "status:\t\tdocked\n");
		len += sprintf(p + len, "commands:\tdock, undock\n");
	}

	return len;
}

static int dock_write(char *buf)
{
	char *cmd;

	if (!dock_docked())
		return -ENODEV;

	while ((cmd = next_cmd(&buf))) {
		if (strlencmp(cmd, "undock") == 0) {
			if (!acpi_evalf(dock_handle, NULL, "_DCK", "vd", 0) ||
			    !acpi_evalf(dock_handle, NULL, "_EJ0", "vd", 1))
				return -EIO;
		} else if (strlencmp(cmd, "dock") == 0) {
			if (!acpi_evalf(dock_handle, NULL, "_DCK", "vd", 1))
				return -EIO;
		} else
			return -EINVAL;
	}

	return 0;
}

static void dock_notify(struct ibm_struct *ibm, u32 event)
{
	int docked = dock_docked();
	int pci = ibm->hid && strstr(ibm->hid, IBM_PCI_HID);

	if (event == 1 && !pci)	/* 570 */
		acpi_bus_generate_event(ibm->device, event, 1);	/* button */
	else if (event == 1 && pci)	/* 570 */
		acpi_bus_generate_event(ibm->device, event, 3);	/* dock */
	else if (event == 3 && docked)
		acpi_bus_generate_event(ibm->device, event, 1);	/* button */
	else if (event == 3 && !docked)
		acpi_bus_generate_event(ibm->device, event, 2);	/* undock */
	else if (event == 0 && docked)
		acpi_bus_generate_event(ibm->device, event, 3);	/* dock */
	else {
		printk(IBM_ERR "unknown dock event %d, status %d\n",
		       event, _sta(dock_handle));
		acpi_bus_generate_event(ibm->device, event, 0);	/* unknown */
	}
}

static int bay_status_supported;
static int bay_status2_supported;
static int bay_eject_supported;
static int bay_eject2_supported;

static int bay_init(void)
{
	bay_status_supported = bay_handle &&
	    acpi_evalf(bay_handle, NULL, "_STA", "qv");
	bay_status2_supported = bay2_handle &&
	    acpi_evalf(bay2_handle, NULL, "_STA", "qv");

	bay_eject_supported = bay_handle && bay_ej_handle &&
	    (strlencmp(bay_ej_path, "_EJ0") == 0 || experimental);
	bay_eject2_supported = bay2_handle && bay2_ej_handle &&
	    (strlencmp(bay2_ej_path, "_EJ0") == 0 || experimental);

	return 0;
}

#define bay_occupied(b) (_sta(b##_handle) & 1)

static int bay_read(char *p)
{
	int len = 0;
	int occupied = bay_occupied(bay);
	int occupied2 = bay_occupied(bay2);
	int eject, eject2;

	len += sprintf(p + len, "status:\t\t%s\n", bay_status_supported ?
		       (occupied ? "occupied" : "unoccupied") :
		       "not supported");
	if (bay_status2_supported)
		len += sprintf(p + len, "status2:\t%s\n", occupied2 ?
			       "occupied" : "unoccupied");

	eject = bay_eject_supported && occupied;
	eject2 = bay_eject2_supported && occupied2;

	if (eject && eject2)
		len += sprintf(p + len, "commands:\teject, eject2\n");
	else if (eject)
		len += sprintf(p + len, "commands:\teject\n");
	else if (eject2)
		len += sprintf(p + len, "commands:\teject2\n");

	return len;
}

static int bay_write(char *buf)
{
	char *cmd;

	if (!bay_eject_supported && !bay_eject2_supported)
		return -ENODEV;

	while ((cmd = next_cmd(&buf))) {
		if (bay_eject_supported && strlencmp(cmd, "eject") == 0) {
			if (!acpi_evalf(bay_ej_handle, NULL, NULL, "vd", 1))
				return -EIO;
		} else if (bay_eject2_supported &&
			   strlencmp(cmd, "eject2") == 0) {
			if (!acpi_evalf(bay2_ej_handle, NULL, NULL, "vd", 1))
				return -EIO;
		} else
			return -EINVAL;
	}

	return 0;
}

static void bay_notify(struct ibm_struct *ibm, u32 event)
{
	acpi_bus_generate_event(ibm->device, event, 0);
}

static int cmos_read(char *p)
{
	int len = 0;

	/* cmos not supported on 570, 600e/x, 770e, 770x, A21e, A2xm/p,
	   R30, R31, T20-22, X20-21 */
	if (!cmos_handle)
		len += sprintf(p + len, "status:\t\tnot supported\n");
	else {
		len += sprintf(p + len, "status:\t\tsupported\n");
		len += sprintf(p + len, "commands:\t<cmd> (<cmd> is 0-21)\n");
	}

	return len;
}

static int cmos_eval(int cmos_cmd)
{
	if (cmos_handle)
		return acpi_evalf(cmos_handle, NULL, NULL, "vd", cmos_cmd);
	else
		return 1;
}

static int cmos_write(char *buf)
{
	char *cmd;
	int cmos_cmd;

	if (!cmos_handle)
		return -EINVAL;

	while ((cmd = next_cmd(&buf))) {
		if (sscanf(cmd, "%u", &cmos_cmd) == 1 &&
		    cmos_cmd >= 0 && cmos_cmd <= 21) {
			/* cmos_cmd set */
		} else
			return -EINVAL;

		if (!cmos_eval(cmos_cmd))
			return -EIO;
	}

	return 0;
}

static int led_supported;

#define LED_570 1
#define LED_OLD 2
#define LED_NEW 3

static int led_init(void)
{
	if (!led_handle)
		/* led not supported on R30, R31 */
		led_supported = 0;
	else if (strlencmp(led_path, "SLED") == 0)
		/* 570 */
		led_supported = LED_570;
	else if (strlencmp(led_path, "SYSL") == 0)
		/* 600e/x, 770e, 770x, A21e, A2xm/p, T20-22, X20-21 */
		led_supported = LED_OLD;
	else
		/* all others */
		led_supported = LED_NEW;

	return 0;
}

#define led_status(s) ((s) == 0 ? "off" : ((s) == 1 ? "on" : "blinking"))

static int led_read(char *p)
{
	int len = 0;

	if (!led_supported) {
		len += sprintf(p + len, "status:\t\tnot supported\n");
		return len;
	}
	len += sprintf(p + len, "status:\t\tsupported\n");

	if (led_supported == LED_570) {
		/* 570 */
		int i, status;
		for (i = 0; i < 8; i++) {
			if (!acpi_evalf(ec_handle,
					&status, "GLED", "dd", 1 << i))
				return -EIO;
			len += sprintf(p + len, "%d:\t\t%s\n",
				       i, led_status(status));
		}
	}

	len += sprintf(p + len, "commands:\t"
		       "<led> on, <led> off, <led> blink (<led> is 0-7)\n");

	return len;
}

/* off, on, blink */
static const int led_sled_arg1[] = { 0, 1, 3 };
static const int led_exp_hlbl[] = { 0, 0, 1 };	/* led# * */
static const int led_exp_hlcl[] = { 0, 1, 1 };	/* led# * */
static const int led_led_arg1[] = { 0, 0x80, 0xc0 };

#define EC_HLCL 0x0c
#define EC_HLBL 0x0d
#define EC_HLMS 0x0e

static int led_write(char *buf)
{
	char *cmd;
	int led, ind, ret;

	if (!led_supported)
		return -ENODEV;

	while ((cmd = next_cmd(&buf))) {
		if (sscanf(cmd, "%d", &led) != 1 || led < 0 || led > 7)
			return -EINVAL;

		if (strstr(cmd, "off")) {
			ind = 0;
		} else if (strstr(cmd, "on")) {
			ind = 1;
		} else if (strstr(cmd, "blink")) {
			ind = 2;
		} else
			return -EINVAL;

		if (led_supported == LED_570) {
			/* 570 */
			led = 1 << led;
			if (!acpi_evalf(led_handle, NULL, NULL, "vdd",
					led, led_sled_arg1[ind]))
				return -EIO;
		} else if (led_supported == LED_OLD) {
			/* 600e/x, 770e, 770x, A21e, A2xm/p, T20-22, X20 */
			led = 1 << led;
			ret = ec_write(EC_HLMS, led);
			if (ret >= 0)
				ret =
				    ec_write(EC_HLBL, led * led_exp_hlbl[ind]);
			if (ret >= 0)
				ret =
				    ec_write(EC_HLCL, led * led_exp_hlcl[ind]);
			if (ret < 0)
				return ret;
		} else {
			/* all others */
			if (!acpi_evalf(led_handle, NULL, NULL, "vdd",
					led, led_led_arg1[ind]))
				return -EIO;
		}
	}

	return 0;
}

static int beep_read(char *p)
{
	int len = 0;

	if (!beep_handle)
		len += sprintf(p + len, "status:\t\tnot supported\n");
	else {
		len += sprintf(p + len, "status:\t\tsupported\n");
		len += sprintf(p + len, "commands:\t<cmd> (<cmd> is 0-17)\n");
	}

	return len;
}

static int beep_write(char *buf)
{
	char *cmd;
	int beep_cmd;

	if (!beep_handle)
		return -ENODEV;

	while ((cmd = next_cmd(&buf))) {
		if (sscanf(cmd, "%u", &beep_cmd) == 1 &&
		    beep_cmd >= 0 && beep_cmd <= 17) {
			/* beep_cmd set */
		} else
			return -EINVAL;
		if (!acpi_evalf(beep_handle, NULL, NULL, "vdd", beep_cmd, 0))
			return -EIO;
	}

	return 0;
}

static int acpi_ec_read(int i, u8 * p)
{
	int v;

	if (ecrd_handle) {
		if (!acpi_evalf(ecrd_handle, &v, NULL, "dd", i))
			return 0;
		*p = v;
	} else {
		if (ec_read(i, p) < 0)
			return 0;
	}

	return 1;
}

static int acpi_ec_write(int i, u8 v)
{
	if (ecwr_handle) {
		if (!acpi_evalf(ecwr_handle, NULL, NULL, "vdd", i, v))
			return 0;
	} else {
		if (ec_write(i, v) < 0)
			return 0;
	}

	return 1;
}

static int thermal_tmp_supported;
static int thermal_updt_supported;

static int thermal_init(void)
{
	/* temperatures not supported on 570, G4x, R30, R31, R32 */
	thermal_tmp_supported = acpi_evalf(ec_handle, NULL, "TMP7", "qv");

	/* 600e/x, 770e, 770x */
	thermal_updt_supported = acpi_evalf(ec_handle, NULL, "UPDT", "qv");

	return 0;
}

static int thermal_read(char *p)
{
	int len = 0;

	if (!thermal_tmp_supported)
		len += sprintf(p + len, "temperatures:\tnot supported\n");
	else {
		int i, t;
		char tmpi[] = "TMPi";
		s8 tmp[8];

		if (thermal_updt_supported)
			if (!acpi_evalf(ec_handle, NULL, "UPDT", "v"))
				return -EIO;

		for (i = 0; i < 8; i++) {
			tmpi[3] = '0' + i;
			if (!acpi_evalf(ec_handle, &t, tmpi, "d"))
				return -EIO;
			if (thermal_updt_supported)
				tmp[i] = (t - 2732 + 5) / 10;
			else
				tmp[i] = t;
		}

		len += sprintf(p + len,
			       "temperatures:\t%d %d %d %d %d %d %d %d\n",
			       tmp[0], tmp[1], tmp[2], tmp[3],
			       tmp[4], tmp[5], tmp[6], tmp[7]);
	}

	return len;
}

static u8 ecdump_regs[256];

static int ecdump_read(char *p)
{
	int len = 0;
	int i, j;
	u8 v;

	len += sprintf(p + len, "EC      "
		       " +00 +01 +02 +03 +04 +05 +06 +07"
		       " +08 +09 +0a +0b +0c +0d +0e +0f\n");
	for (i = 0; i < 256; i += 16) {
		len += sprintf(p + len, "EC 0x%02x:", i);
		for (j = 0; j < 16; j++) {
			if (!acpi_ec_read(i + j, &v))
				break;
			if (v != ecdump_regs[i + j])
				len += sprintf(p + len, " *%02x", v);
			else
				len += sprintf(p + len, "  %02x", v);
			ecdump_regs[i + j] = v;
		}
		len += sprintf(p + len, "\n");
		if (j != 16)
			break;
	}

	/* These are way too dangerous to advertise openly... */
#if 0
	len += sprintf(p + len, "commands:\t0x<offset> 0x<value>"
		       " (<offset> is 00-ff, <value> is 00-ff)\n");
	len += sprintf(p + len, "commands:\t0x<offset> <value>  "
		       " (<offset> is 00-ff, <value> is 0-255)\n");
#endif
	return len;
}

static int ecdump_write(char *buf)
{
	char *cmd;
	int i, v;

	while ((cmd = next_cmd(&buf))) {
		if (sscanf(cmd, "0x%x 0x%x", &i, &v) == 2) {
			/* i and v set */
		} else if (sscanf(cmd, "0x%x %u", &i, &v) == 2) {
			/* i and v set */
		} else
			return -EINVAL;
		if (i >= 0 && i < 256 && v >= 0 && v < 256) {
			if (!acpi_ec_write(i, v))
				return -EIO;
		} else
			return -EINVAL;
	}

	return 0;
}

static int brightness_offset = 0x31;

static int brightness_read(char *p)
{
	int len = 0;
	u8 level;

	if (!acpi_ec_read(brightness_offset, &level)) {
		len += sprintf(p + len, "level:\t\tunreadable\n");
	} else {
		len += sprintf(p + len, "level:\t\t%d\n", level & 0x7);
		len += sprintf(p + len, "commands:\tup, down\n");
		len += sprintf(p + len, "commands:\tlevel <level>"
			       " (<level> is 0-7)\n");
	}

	return len;
}

#define BRIGHTNESS_UP	4
#define BRIGHTNESS_DOWN	5

static int brightness_write(char *buf)
{
	int cmos_cmd, inc, i;
	u8 level;
	int new_level;
	char *cmd;

	while ((cmd = next_cmd(&buf))) {
		if (!acpi_ec_read(brightness_offset, &level))
			return -EIO;
		level &= 7;

		if (strlencmp(cmd, "up") == 0) {
			new_level = level == 7 ? 7 : level + 1;
		} else if (strlencmp(cmd, "down") == 0) {
			new_level = level == 0 ? 0 : level - 1;
		} else if (sscanf(cmd, "level %d", &new_level) == 1 &&
			   new_level >= 0 && new_level <= 7) {
			/* new_level set */
		} else
			return -EINVAL;

		cmos_cmd = new_level > level ? BRIGHTNESS_UP : BRIGHTNESS_DOWN;
		inc = new_level > level ? 1 : -1;
		for (i = level; i != new_level; i += inc) {
			if (!cmos_eval(cmos_cmd))
				return -EIO;
			if (!acpi_ec_write(brightness_offset, i + inc))
				return -EIO;
		}
	}

	return 0;
}

static int volume_offset = 0x30;

static int volume_read(char *p)
{
	int len = 0;
	u8 level;

	if (!acpi_ec_read(volume_offset, &level)) {
		len += sprintf(p + len, "level:\t\tunreadable\n");
	} else {
		len += sprintf(p + len, "level:\t\t%d\n", level & 0xf);
		len += sprintf(p + len, "mute:\t\t%s\n", onoff(level, 6));
		len += sprintf(p + len, "commands:\tup, down, mute\n");
		len += sprintf(p + len, "commands:\tlevel <level>"
			       " (<level> is 0-15)\n");
	}

	return len;
}

#define VOLUME_DOWN	0
#define VOLUME_UP	1
#define VOLUME_MUTE	2

static int volume_write(char *buf)
{
	int cmos_cmd, inc, i;
	u8 level, mute;
	int new_level, new_mute;
	char *cmd;

	while ((cmd = next_cmd(&buf))) {
		if (!acpi_ec_read(volume_offset, &level))
			return -EIO;
		new_mute = mute = level & 0x40;
		new_level = level = level & 0xf;

		if (strlencmp(cmd, "up") == 0) {
			if (mute)
				new_mute = 0;
			else
				new_level = level == 15 ? 15 : level + 1;
		} else if (strlencmp(cmd, "down") == 0) {
			if (mute)
				new_mute = 0;
			else
				new_level = level == 0 ? 0 : level - 1;
		} else if (sscanf(cmd, "level %d", &new_level) == 1 &&
			   new_level >= 0 && new_level <= 15) {
			/* new_level set */
		} else if (strlencmp(cmd, "mute") == 0) {
			new_mute = 0x40;
		} else
			return -EINVAL;

		if (new_level != level) {	/* mute doesn't change */
			cmos_cmd = new_level > level ? VOLUME_UP : VOLUME_DOWN;
			inc = new_level > level ? 1 : -1;

			if (mute && (!cmos_eval(cmos_cmd) ||
				     !acpi_ec_write(volume_offset, level)))
				return -EIO;

			for (i = level; i != new_level; i += inc)
				if (!cmos_eval(cmos_cmd) ||
				    !acpi_ec_write(volume_offset, i + inc))
					return -EIO;

			if (mute && (!cmos_eval(VOLUME_MUTE) ||
				     !acpi_ec_write(volume_offset,
						    new_level + mute)))
				return -EIO;
		}

		if (new_mute != mute) {	/* level doesn't change */
			cmos_cmd = new_mute ? VOLUME_MUTE : VOLUME_UP;

			if (!cmos_eval(cmos_cmd) ||
			    !acpi_ec_write(volume_offset, level + new_mute))
				return -EIO;
		}
	}

	return 0;
}

static int fan_status_offset = 0x2f;
static int fan_rpm_offset = 0x84;

static int fan_read(char *p)
{
	int len = 0;
	int s;
	u8 lo, hi, status;

	if (gfan_handle) {
		/* 570, 600e/x, 770e, 770x */
		if (!acpi_evalf(gfan_handle, &s, NULL, "d"))
			return -EIO;

		len += sprintf(p + len, "level:\t\t%d\n", s);
	} else {
		/* all except 570, 600e/x, 770e, 770x */
		if (!acpi_ec_read(fan_status_offset, &status))
			len += sprintf(p + len, "status:\t\tunreadable\n");
		else
			len += sprintf(p + len, "status:\t\t%s\n",
				       enabled(status, 7));

		if (!acpi_ec_read(fan_rpm_offset, &lo) ||
		    !acpi_ec_read(fan_rpm_offset + 1, &hi))
			len += sprintf(p + len, "speed:\t\tunreadable\n");
		else
			len += sprintf(p + len, "speed:\t\t%d\n",
				       (hi << 8) + lo);
	}

	if (sfan_handle)
		/* 570, 770x-JL */
		len += sprintf(p + len, "commands:\tlevel <level>"
			       " (<level> is 0-7)\n");
	if (!gfan_handle)
		/* all except 570, 600e/x, 770e, 770x */
		len += sprintf(p + len, "commands:\tenable, disable\n");
	if (fans_handle)
		/* X31, X40 */
		len += sprintf(p + len, "commands:\tspeed <speed>"
			       " (<speed> is 0-65535)\n");

	return len;
}

static int fan_write(char *buf)
{
	char *cmd;
	int level, speed;

	while ((cmd = next_cmd(&buf))) {
		if (sfan_handle &&
		    sscanf(cmd, "level %d", &level) == 1 &&
		    level >= 0 && level <= 7) {
			/* 570, 770x-JL */
			if (!acpi_evalf(sfan_handle, NULL, NULL, "vd", level))
				return -EIO;
		} else if (!gfan_handle && strlencmp(cmd, "enable") == 0) {
			/* all except 570, 600e/x, 770e, 770x */
			if (!acpi_ec_write(fan_status_offset, 0x80))
				return -EIO;
		} else if (!gfan_handle && strlencmp(cmd, "disable") == 0) {
			/* all except 570, 600e/x, 770e, 770x */
			if (!acpi_ec_write(fan_status_offset, 0x00))
				return -EIO;
		} else if (fans_handle &&
			   sscanf(cmd, "speed %d", &speed) == 1 &&
			   speed >= 0 && speed <= 65535) {
			/* X31, X40 */
			if (!acpi_evalf(fans_handle, NULL, NULL, "vddd",
					speed, speed, speed))
				return -EIO;
		} else
			return -EINVAL;
	}

	return 0;
}

static struct ibm_struct ibms[] = {
	{
	 .name = "driver",
	 .init = driver_init,
	 .read = driver_read,
	 },
	{
	 .name = "hotkey",
	 .hid = IBM_HKEY_HID,
	 .init = hotkey_init,
	 .read = hotkey_read,
	 .write = hotkey_write,
	 .exit = hotkey_exit,
	 .notify = hotkey_notify,
	 .handle = &hkey_handle,
	 .type = ACPI_DEVICE_NOTIFY,
	 },
	{
	 .name = "bluetooth",
	 .init = bluetooth_init,
	 .read = bluetooth_read,
	 .write = bluetooth_write,
	 },
	{
	 .name = "video",
	 .init = video_init,
	 .read = video_read,
	 .write = video_write,
	 .exit = video_exit,
	 },
	{
	 .name = "light",
	 .init = light_init,
	 .read = light_read,
	 .write = light_write,
	 },
	{
	 .name = "dock",
	 .read = dock_read,
	 .write = dock_write,
	 .notify = dock_notify,
	 .handle = &dock_handle,
	 .type = ACPI_SYSTEM_NOTIFY,
	 },
	{
	 .name = "dock",
	 .hid = IBM_PCI_HID,
	 .notify = dock_notify,
	 .handle = &pci_handle,
	 .type = ACPI_SYSTEM_NOTIFY,
	 },
	{
	 .name = "bay",
	 .init = bay_init,
	 .read = bay_read,
	 .write = bay_write,
	 .notify = bay_notify,
	 .handle = &bay_handle,
	 .type = ACPI_SYSTEM_NOTIFY,
	 },
	{
	 .name = "cmos",
	 .read = cmos_read,
	 .write = cmos_write,
	 },
	{
	 .name = "led",
	 .init = led_init,
	 .read = led_read,
	 .write = led_write,
	 },
	{
	 .name = "beep",
	 .read = beep_read,
	 .write = beep_write,
	 },
	{
	 .name = "thermal",
	 .init = thermal_init,
	 .read = thermal_read,
	 },
	{
	 .name = "ecdump",
	 .read = ecdump_read,
	 .write = ecdump_write,
	 .experimental = 1,
	 },
	{
	 .name = "brightness",
	 .read = brightness_read,
	 .write = brightness_write,
	 .experimental = 1,
	 },
	{
	 .name = "volume",
	 .read = volume_read,
	 .write = volume_write,
	 .experimental = 1,
	 },
	{
	 .name = "fan",
	 .read = fan_read,
	 .write = fan_write,
	 .experimental = 1,
	 },
};

static int dispatch_read(char *page, char **start, off_t off, int count,
			 int *eof, void *data)
{
	struct ibm_struct *ibm = (struct ibm_struct *)data;
	int len;

	if (!ibm || !ibm->read)
		return -EINVAL;

	len = ibm->read(page);
	if (len < 0)
		return len;

	if (len <= off + count)
		*eof = 1;
	*start = page + off;
	len -= off;
	if (len > count)
		len = count;
	if (len < 0)
		len = 0;

	return len;
}

static int dispatch_write(struct file *file, const char __user * userbuf,
			  unsigned long count, void *data)
{
	struct ibm_struct *ibm = (struct ibm_struct *)data;
	char *kernbuf;
	int ret;

	if (!ibm || !ibm->write)
		return -EINVAL;

	kernbuf = kmalloc(count + 2, GFP_KERNEL);
	if (!kernbuf)
		return -ENOMEM;

	if (copy_from_user(kernbuf, userbuf, count)) {
		kfree(kernbuf);
		return -EFAULT;
	}

	kernbuf[count] = 0;
	strcat(kernbuf, ",");
	ret = ibm->write(kernbuf);
	if (ret == 0)
		ret = count;

	kfree(kernbuf);

	return ret;
}

static void dispatch_notify(acpi_handle handle, u32 event, void *data)
{
	struct ibm_struct *ibm = (struct ibm_struct *)data;

	if (!ibm || !ibm->notify)
		return;

	ibm->notify(ibm, event);
}

static int __init setup_notify(struct ibm_struct *ibm)
{
	acpi_status status;
	int ret;

	if (!*ibm->handle)
		return 0;

	ret = acpi_bus_get_device(*ibm->handle, &ibm->device);
	if (ret < 0) {
		printk(IBM_ERR "%s device not present\n", ibm->name);
		return 0;
	}

	acpi_driver_data(ibm->device) = ibm;
	sprintf(acpi_device_class(ibm->device), "%s/%s", IBM_NAME, ibm->name);

	status = acpi_install_notify_handler(*ibm->handle, ibm->type,
					     dispatch_notify, ibm);
	if (ACPI_FAILURE(status)) {
		printk(IBM_ERR "acpi_install_notify_handler(%s) failed: %d\n",
		       ibm->name, status);
		return -ENODEV;
	}

	return 0;
}

static int __init ibm_device_add(struct acpi_device *device)
{
	return 0;
}

static int __init register_driver(struct ibm_struct *ibm)
{
	int ret;

	ibm->driver = kmalloc(sizeof(struct acpi_driver), GFP_KERNEL);
	if (!ibm->driver) {
		printk(IBM_ERR "kmalloc(ibm->driver) failed\n");
		return -1;
	}

	memset(ibm->driver, 0, sizeof(struct acpi_driver));
	sprintf(ibm->driver->name, "%s/%s", IBM_NAME, ibm->name);
	ibm->driver->ids = ibm->hid;
	ibm->driver->ops.add = &ibm_device_add;

	ret = acpi_bus_register_driver(ibm->driver);
	if (ret < 0) {
		printk(IBM_ERR "acpi_bus_register_driver(%s) failed: %d\n",
		       ibm->hid, ret);
		kfree(ibm->driver);
	}

	return ret;
}

static int __init ibm_init(struct ibm_struct *ibm)
{
	int ret;
	struct proc_dir_entry *entry;

	if (ibm->experimental && !experimental)
		return 0;

	if (ibm->hid) {
		ret = register_driver(ibm);
		if (ret < 0)
			return ret;
		ibm->driver_registered = 1;
	}

	if (ibm->init) {
		ret = ibm->init();
		if (ret != 0)
			return ret;
		ibm->init_called = 1;
	}

	if (ibm->read) {
		entry = create_proc_entry(ibm->name,
					  S_IFREG | S_IRUGO | S_IWUSR,
					  proc_dir);
		if (!entry) {
			printk(IBM_ERR "unable to create proc entry %s\n",
			       ibm->name);
			return -ENODEV;
		}
		entry->owner = THIS_MODULE;
		entry->data = ibm;
		entry->read_proc = &dispatch_read;
		if (ibm->write)
			entry->write_proc = &dispatch_write;
		ibm->proc_created = 1;
	}

	if (ibm->notify) {
		ret = setup_notify(ibm);
		if (ret < 0)
			return ret;
		ibm->notify_installed = 1;
	}

	return 0;
}

static void ibm_exit(struct ibm_struct *ibm)
{
	if (ibm->notify_installed)
		acpi_remove_notify_handler(*ibm->handle, ibm->type,
					   dispatch_notify);

	if (ibm->proc_created)
		remove_proc_entry(ibm->name, proc_dir);

	if (ibm->init_called && ibm->exit)
		ibm->exit();

	if (ibm->driver_registered) {
		acpi_bus_unregister_driver(ibm->driver);
		kfree(ibm->driver);
	}
}

static void __init ibm_handle_init(char *name,
				   acpi_handle * handle, acpi_handle parent,
				   char **paths, int num_paths, char **path)
{
	int i;
	acpi_status status;

	for (i = 0; i < num_paths; i++) {
		status = acpi_get_handle(parent, paths[i], handle);
		if (ACPI_SUCCESS(status)) {
			*path = paths[i];
			return;
		}
	}

	*handle = NULL;
}

#define IBM_HANDLE_INIT(object)						\
	ibm_handle_init(#object, &object##_handle, *object##_parent,	\
		object##_paths, ARRAY_SIZE(object##_paths), &object##_path)

static int set_ibm_param(const char *val, struct kernel_param *kp)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(ibms); i++)
		if (strcmp(ibms[i].name, kp->name) == 0 && ibms[i].write) {
			if (strlen(val) > sizeof(ibms[i].param) - 2)
				return -ENOSPC;
			strcpy(ibms[i].param, val);
			strcat(ibms[i].param, ",");
			return 0;
		}

	return -EINVAL;
}

#define IBM_PARAM(feature) \
	module_param_call(feature, set_ibm_param, NULL, NULL, 0)

IBM_PARAM(hotkey);
IBM_PARAM(bluetooth);
IBM_PARAM(video);
IBM_PARAM(light);
IBM_PARAM(dock);
IBM_PARAM(bay);
IBM_PARAM(cmos);
IBM_PARAM(led);
IBM_PARAM(beep);
IBM_PARAM(ecdump);
IBM_PARAM(brightness);
IBM_PARAM(volume);
IBM_PARAM(fan);

static void acpi_ibm_exit(void)
{
	int i;

	for (i = ARRAY_SIZE(ibms) - 1; i >= 0; i--)
		ibm_exit(&ibms[i]);

	remove_proc_entry(IBM_DIR, acpi_root_dir);
}

static int __init acpi_ibm_init(void)
{
	int ret, i;

	if (acpi_disabled)
		return -ENODEV;

	if (!acpi_specific_hotkey_enabled) {
		printk(IBM_ERR "using generic hotkey driver\n");
		return -ENODEV;
	}

	/* ec is required because many other handles are relative to it */
	IBM_HANDLE_INIT(ec);
	if (!ec_handle) {
		printk(IBM_ERR "ec object not found\n");
		return -ENODEV;
	}

	/* these handles are not required */
	IBM_HANDLE_INIT(vid);
	IBM_HANDLE_INIT(vid2);
	IBM_HANDLE_INIT(ledb);
	IBM_HANDLE_INIT(led);
	IBM_HANDLE_INIT(hkey);
	IBM_HANDLE_INIT(lght);
	IBM_HANDLE_INIT(cmos);
	IBM_HANDLE_INIT(dock);
	IBM_HANDLE_INIT(pci);
	IBM_HANDLE_INIT(bay);
	if (bay_handle)
		IBM_HANDLE_INIT(bay_ej);
	IBM_HANDLE_INIT(bay2);
	if (bay2_handle)
		IBM_HANDLE_INIT(bay2_ej);
	IBM_HANDLE_INIT(beep);
	IBM_HANDLE_INIT(ecrd);
	IBM_HANDLE_INIT(ecwr);
	IBM_HANDLE_INIT(fans);
	IBM_HANDLE_INIT(gfan);
	IBM_HANDLE_INIT(sfan);

	proc_dir = proc_mkdir(IBM_DIR, acpi_root_dir);
	if (!proc_dir) {
		printk(IBM_ERR "unable to create proc dir %s", IBM_DIR);
		return -ENODEV;
	}
	proc_dir->owner = THIS_MODULE;

	for (i = 0; i < ARRAY_SIZE(ibms); i++) {
		ret = ibm_init(&ibms[i]);
		if (ret >= 0 && *ibms[i].param)
			ret = ibms[i].write(ibms[i].param);
		if (ret < 0) {
			acpi_ibm_exit();
			return ret;
		}
	}

	return 0;
}

module_init(acpi_ibm_init);
module_exit(acpi_ibm_exit);
