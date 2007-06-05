/* dvb-usb.h is part of the DVB USB library.
 *
 * Copyright (C) 2004-5 Patrick Boettcher (patrick.boettcher@desy.de)
 * see dvb-usb-init.c for copyright information.
 *
 * the headerfile, all dvb-usb-drivers have to include.
 */
#ifndef __DVB_USB_H__
#define __DVB_USB_H__

#include <linux/config.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/usb.h>

#include "dvb_frontend.h"
#include "dvb_demux.h"
#include "dvb_net.h"
#include "dmxdev.h"

#include "dvb-pll.h"

#include "dvb-usb-ids.h"

/* debug */
#ifdef CONFIG_DVB_USB_DEBUG
#define dprintk(var,level,args...) \
	    do { if ((var & level)) { printk(args); } } while (0)

#define debug_dump(b,l,func) {\
	int loop_; \
	for (loop_ = 0; loop_ < l; loop_++) func("%02x ", b[loop_]); \
	func("\n");\
}
#define DVB_USB_DEBUG_STATUS
#else
#define dprintk(args...)
#define debug_dump(b,l,func)

#define DVB_USB_DEBUG_STATUS " (debugging is not enabled)"

#endif

/* generic log methods - taken from usb.h */
#ifndef DVB_USB_LOG_PREFIX
 #define DVB_USB_LOG_PREFIX "dvb-usb (please define a log prefix)"
#endif

#undef err
#define err(format, arg...)  printk(KERN_ERR     DVB_USB_LOG_PREFIX ": " format "\n" , ## arg)
#undef info
#define info(format, arg...) printk(KERN_INFO    DVB_USB_LOG_PREFIX ": " format "\n" , ## arg)
#undef warn
#define warn(format, arg...) printk(KERN_WARNING DVB_USB_LOG_PREFIX ": " format "\n" , ## arg)

/**
 * struct dvb_usb_device_description - name and its according USB IDs
 * @name: real name of the box, regardless which DVB USB device class is in use
 * @cold_ids: array of struct usb_device_id which describe the device in
 *  pre-firmware state
 * @warm_ids: array of struct usb_device_id which describe the device in
 *  post-firmware state
 *
 * Each DVB USB device class can have one or more actual devices, this struct
 * assigns a name to it.
 */
struct dvb_usb_device_description {
	const char *name;

#define DVB_USB_ID_MAX_NUM 15
	struct usb_device_id *cold_ids[DVB_USB_ID_MAX_NUM];
	struct usb_device_id *warm_ids[DVB_USB_ID_MAX_NUM];
};

/**
 * struct dvb_usb_rc_key - a remote control key and its input-event
 * @custom: the vendor/custom part of the key
 * @data: the actual key part
 * @event: the input event assigned to key identified by custom and data
 */
struct dvb_usb_rc_key {
	u8 custom,data;
	u32 event;
};

struct dvb_usb_device;

/**
 * struct dvb_usb_properties - properties of a dvb-usb-device
 * @caps: capabilites of the DVB USB device.
 * @pid_filter_count: number of PID filter position in the optional hardware
 *  PID-filter.
 *
 * @usb_ctrl: which USB device-side controller is in use. Needed for firmware
 *  download.
 * @firmware: name of the firmware file.
 *
 * @size_of_priv: how many bytes shall be allocated for the private field
 *  of struct dvb_usb_device.
 *
 * @power_ctrl: called to enable/disable power of the device.
 * @streaming_crtl: called to start and stop the MPEG2-TS streaming of the
 *  device (not URB submitting/killing).
 * @pid_filter_ctrl: called to en/disable the PID filter, if any.
 * @pid_filter: called to set/unset a PID for filtering.
 *
 * @read_mac_address: called to read the MAC address of the device.
 *
 * @frontend_attach: called to attach the possible frontends (fill fe-field
 *  of struct dvb_usb_device).
 * @tuner_attach: called to attach the correct tuner and to fill pll_addr,
 *  pll_desc and pll_init_buf of struct dvb_usb_device).
 * @identify_state: called to determine the state (cold or warm), when it
 *  is not distinguishable by the USB IDs.
 *
 * @rc_key_map: a hard-wired array of struct dvb_usb_rc_key (NULL to disable
 *  remote control handling).
 * @rc_key_map_size: number of items in @rc_key_map.
 * @rc_query: called to query an event event.
 * @rc_interval: time in ms between two queries.
 *
 * @i2c_algo: i2c_algorithm if the device has I2CoverUSB.
 *
 * @generic_bulk_ctrl_endpoint: most of the DVB USB devices have a generic
 *  endpoint which received control messages with bulk transfers. When this
 *  is non-zero, one can use dvb_usb_generic_rw and dvb_usb_generic_write-
 *  helper functions.
 *
 * @urb: describes the kind of USB transfer used for MPEG2-TS-streaming.
 *  (BULK or ISOC)
 *
 * @num_device_descs: number of struct dvb_usb_device_description in @devices
 * @devices: array of struct dvb_usb_device_description compatibles with these
 *  properties.
 */
struct dvb_usb_properties {

#define DVB_USB_HAS_PID_FILTER               0x01
#define DVB_USB_PID_FILTER_CAN_BE_TURNED_OFF 0x02
#define DVB_USB_NEED_PID_FILTERING           0x04
#define DVB_USB_IS_AN_I2C_ADAPTER            0x08
	int caps;
	int pid_filter_count;

#define CYPRESS_AN2135  0
#define CYPRESS_AN2235  1
#define CYPRESS_FX2     2
	int usb_ctrl;
	const char *firmware;

	int size_of_priv;

	int (*power_ctrl) (struct dvb_usb_device *, int);
	int (*streaming_ctrl) (struct dvb_usb_device *, int);
	int (*pid_filter_ctrl) (struct dvb_usb_device *, int);
	int (*pid_filter) (struct dvb_usb_device *, int, u16, int);

	int (*read_mac_address) (struct dvb_usb_device *, u8 []);
	int (*frontend_attach) (struct dvb_usb_device *);
	int (*tuner_attach) (struct dvb_usb_device *);

	int (*identify_state) (struct usb_device *, struct dvb_usb_properties *,
			struct dvb_usb_device_description **, int *);

/* remote control properties */
#define REMOTE_NO_KEY_PRESSED      0x00
#define REMOTE_KEY_PRESSED         0x01
#define REMOTE_KEY_REPEAT          0x02
	struct dvb_usb_rc_key *rc_key_map;
	int rc_key_map_size;
	int (*rc_query) (struct dvb_usb_device *, u32 *, int *);
	int rc_interval;

	struct i2c_algorithm *i2c_algo;

	int generic_bulk_ctrl_endpoint;

	struct {
#define DVB_USB_BULK  1
#define DVB_USB_ISOC  2
		int type;
		int count;
		int endpoint;

		union {
			struct {
				int buffersize; /* per URB */
			} bulk;
			struct {
				int framesperurb;
				int framesize;
				int interval;
			} isoc;
		} u;
	} urb;

	int num_device_descs;
	struct dvb_usb_device_description devices[9];
};


/**
 * struct dvb_usb_device - object of a DVB USB device
 * @props: copy of the struct dvb_usb_properties this device belongs to.
 * @desc: pointer to the device's struct dvb_usb_device_description.
 * @state: initialization and runtime state of the device.
 *
 * @udev: pointer to the device's struct usb_device.
 * @urb_list: array of dynamically allocated struct urb for the MPEG2-TS-
 *  streaming.
 *
 * @buf_num: number of buffer allocated.
 * @buf_size: size of each buffer in buf_list.
 * @buf_list: array containing all allocate buffers for streaming.
 * @dma_addr: list of dma_addr_t for each buffer in buf_list.
 *
 * @urbs_initialized: number of URBs initialized.
 * @urbs_submitted: number of URBs submitted.
 *
 * @feedcount: number of reqested feeds (used for streaming-activation)
 * @pid_filtering: is hardware pid_filtering used or not.
 *
 * @usb_sem: semaphore of USB control messages (reading needs two messages)
 * @i2c_sem: semaphore for i2c-transfers
 *
 * @i2c_adap: device's i2c_adapter if it uses I2CoverUSB
 * @pll_addr: I2C address of the tuner for programming
 * @pll_init: array containing the initialization buffer
 * @pll_desc: pointer to the appropriate struct dvb_pll_desc
 *
 * @tuner_pass_ctrl: called to (de)activate tuner passthru of the demod or the board
 *
 * @dvb_adap: device's dvb_adapter.
 * @dmxdev: device's dmxdev.
 * @demux: device's software demuxer.
 * @dvb_net: device's dvb_net interfaces.
 * @dvb_frontend: device's frontend.
 * @max_feed_count: how many feeds can be handled simultaneously by this
 *  device
 * @fe_sleep: rerouted frontend-sleep function.
 * @fe_init: rerouted frontend-init (wakeup) function.
 * @rc_input_dev: input device for the remote control.
 * @rc_query_work: struct work_struct frequent rc queries
 * @last_event: last triggered event
 * @last_state: last state (no, pressed, repeat)
 * @owner: owner of the dvb_adapter
 * @priv: private data of the actual driver (allocate by dvb-usb, size defined
 *  in size_of_priv of dvb_usb_properties).
 */
struct dvb_usb_device {
	struct dvb_usb_properties props;
	struct dvb_usb_device_description *desc;

#define DVB_USB_STATE_INIT        0x000
#define DVB_USB_STATE_URB_LIST    0x001
#define DVB_USB_STATE_URB_BUF     0x002
#define DVB_USB_STATE_DVB         0x004
#define DVB_USB_STATE_I2C         0x008
#define DVB_USB_STATE_REMOTE      0x010
#define DVB_USB_STATE_URB_SUBMIT  0x020
	int state;

	/* usb */
	struct usb_device *udev;
	struct urb **urb_list;

	int buf_num;
	unsigned long buf_size;
	u8 **buf_list;
	dma_addr_t *dma_addr;

	int urbs_initialized;
	int urbs_submitted;

	int feedcount;
	int pid_filtering;

	/* locking */
	struct semaphore usb_sem;

	/* i2c */
	struct semaphore i2c_sem;
	struct i2c_adapter i2c_adap;

	/* tuner programming information */
	u8 pll_addr;
	u8 pll_init[4];
	struct dvb_pll_desc *pll_desc;
	int (*tuner_pass_ctrl)(struct dvb_frontend *, int, u8);

	/* dvb */
	struct dvb_adapter dvb_adap;
	struct dmxdev dmxdev;
	struct dvb_demux demux;
	struct dvb_net dvb_net;
	struct dvb_frontend* fe;
	int max_feed_count;

	int (*fe_sleep) (struct dvb_frontend *);
	int (*fe_init)  (struct dvb_frontend *);

	/* remote control */
	struct input_dev rc_input_dev;
	struct work_struct rc_query_work;
	u32 last_event;
	int last_state;

	struct module *owner;

	void *priv;
};

extern int dvb_usb_device_init(struct usb_interface *, struct dvb_usb_properties *, struct module *, struct dvb_usb_device **);
extern void dvb_usb_device_exit(struct usb_interface *);

/* the generic read/write method for device control */
extern int dvb_usb_generic_rw(struct dvb_usb_device *, u8 *, u16, u8 *, u16,int);
extern int dvb_usb_generic_write(struct dvb_usb_device *, u8 *, u16);

/* commonly used remote control parsing */
extern int dvb_usb_nec_rc_key_to_event(struct dvb_usb_device *, u8[], u32 *, int *);

/* commonly used pll init and set functions */
extern int dvb_usb_pll_init_i2c(struct dvb_frontend *);
extern int dvb_usb_pll_set(struct dvb_frontend *, struct dvb_frontend_parameters *, u8[]);
extern int dvb_usb_pll_set_i2c(struct dvb_frontend *, struct dvb_frontend_parameters *);


#endif
