/*
	CA-driver for TwinHan DST Frontend/Card

	Copyright (C) 2004, 2005 Manu Abraham (manu@kromtek.com)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/dvb/ca.h>
#include "dvbdev.h"
#include "dvb_frontend.h"
#include "dst_ca.h"
#include "dst_common.h"

#define DST_CA_ERROR		0
#define DST_CA_NOTICE		1
#define DST_CA_INFO		2
#define DST_CA_DEBUG		3

#define dprintk(x, y, z, format, arg...) do {						\
	if (z) {									\
		if	((x > DST_CA_ERROR) && (x > y))					\
			printk(KERN_ERR "%s: " format "\n", __FUNCTION__ , ##arg);	\
		else if	((x > DST_CA_NOTICE) && (x > y))				\
			printk(KERN_NOTICE "%s: " format "\n", __FUNCTION__ , ##arg);	\
		else if ((x > DST_CA_INFO) && (x > y))					\
			printk(KERN_INFO "%s: " format "\n", __FUNCTION__ , ##arg);	\
		else if ((x > DST_CA_DEBUG) && (x > y))					\
			printk(KERN_DEBUG "%s: " format "\n", __FUNCTION__ , ##arg);	\
	} else {									\
		if (x > y)								\
			printk(format, ## arg);						\
	}										\
} while(0)


static unsigned int verbose = 5;
module_param(verbose, int, 0644);
MODULE_PARM_DESC(verbose, "verbose startup messages, default is 1 (yes)");

/*	Need some more work	*/
static int ca_set_slot_descr(void)
{
	/*	We could make this more graceful ?	*/
	return -EOPNOTSUPP;
}

/*	Need some more work	*/
static int ca_set_pid(void)
{
	/*	We could make this more graceful ?	*/
	return -EOPNOTSUPP;
}


static int put_checksum(u8 *check_string, int length)
{
	u8 i = 0, checksum = 0;

	dprintk(verbose, DST_CA_DEBUG, 1, " ========================= Checksum calculation ===========================");
	dprintk(verbose, DST_CA_DEBUG, 1, " String Length=[0x%02x]", length);
	dprintk(verbose, DST_CA_DEBUG, 1, " String=[");

	while (i < length) {
		dprintk(verbose, DST_CA_DEBUG, 0, " %02x", check_string[i]);
		checksum += check_string[i];
		i++;
	}
	dprintk(verbose, DST_CA_DEBUG, 0, " ]\n");
	dprintk(verbose, DST_CA_DEBUG, 1, "Sum=[%02x]\n", checksum);
	check_string[length] = ~checksum + 1;
	dprintk(verbose, DST_CA_DEBUG, 1, " Checksum=[%02x]", check_string[length]);
	dprintk(verbose, DST_CA_DEBUG, 1, " ==========================================================================");

	return 0;
}

static int dst_ci_command(struct dst_state* state, u8 * data, u8 *ca_string, u8 len, int read)
{
	u8 reply;

	dst_comm_init(state);
	msleep(65);

	if (write_dst(state, data, len)) {
		dprintk(verbose, DST_CA_INFO, 1, " Write not successful, trying to recover");
		dst_error_recovery(state);
		return -1;
	}
	if ((dst_pio_disable(state)) < 0) {
		dprintk(verbose, DST_CA_ERROR, 1, " DST PIO disable failed.");
		return -1;
	}
	if (read_dst(state, &reply, GET_ACK) < 0) {
		dprintk(verbose, DST_CA_INFO, 1, " Read not successful, trying to recover");
		dst_error_recovery(state);
		return -1;
	}
	if (read) {
		if (! dst_wait_dst_ready(state, LONG_DELAY)) {
			dprintk(verbose, DST_CA_NOTICE, 1, " 8820 not ready");
			return -1;
		}
		if (read_dst(state, ca_string, 128) < 0) {	/*	Try to make this dynamic	*/
			dprintk(verbose, DST_CA_INFO, 1, " Read not successful, trying to recover");
			dst_error_recovery(state);
			return -1;
		}
	}

	return 0;
}


static int dst_put_ci(struct dst_state *state, u8 *data, int len, u8 *ca_string, int read)
{
	u8 dst_ca_comm_err = 0;

	while (dst_ca_comm_err < RETRIES) {
		dst_comm_init(state);
		dprintk(verbose, DST_CA_NOTICE, 1, " Put Command");
		if (dst_ci_command(state, data, ca_string, len, read)) {	// If error
			dst_error_recovery(state);
			dst_ca_comm_err++; // work required here.
		}
		break;
	}

	return 0;
}



static int ca_get_app_info(struct dst_state *state)
{
	static u8 command[8] = {0x07, 0x40, 0x01, 0x00, 0x01, 0x00, 0x00, 0xff};

	put_checksum(&command[0], command[0]);
	if ((dst_put_ci(state, command, sizeof(command), state->messages, GET_REPLY)) < 0) {
		dprintk(verbose, DST_CA_ERROR, 1, " -->dst_put_ci FAILED !");
		return -1;
	}
	dprintk(verbose, DST_CA_INFO, 1, " -->dst_put_ci SUCCESS !");
	dprintk(verbose, DST_CA_INFO, 1, " ================================ CI Module Application Info ======================================");
	dprintk(verbose, DST_CA_INFO, 1, " Application Type=[%d], Application Vendor=[%d], Vendor Code=[%d]\n%s: Application info=[%s]",
		state->messages[7], (state->messages[8] << 8) | state->messages[9],
		(state->messages[10] << 8) | state->messages[11], __FUNCTION__, (char *)(&state->messages[12]));
	dprintk(verbose, DST_CA_INFO, 1, " ==================================================================================================");

	return 0;
}

static int ca_get_slot_caps(struct dst_state *state, struct ca_caps *p_ca_caps, void *arg)
{
	int i;
	u8 slot_cap[256];
	static u8 slot_command[8] = {0x07, 0x40, 0x02, 0x00, 0x02, 0x00, 0x00, 0xff};

	put_checksum(&slot_command[0], slot_command[0]);
	if ((dst_put_ci(state, slot_command, sizeof (slot_command), slot_cap, GET_REPLY)) < 0) {
		dprintk(verbose, DST_CA_ERROR, 1, " -->dst_put_ci FAILED !");
		return -1;
	}
	dprintk(verbose, DST_CA_NOTICE, 1, " -->dst_put_ci SUCCESS !");

	/*	Will implement the rest soon		*/

	dprintk(verbose, DST_CA_INFO, 1, " Slot cap = [%d]", slot_cap[7]);
	dprintk(verbose, DST_CA_INFO, 0, "===================================\n");
	for (i = 0; i < 8; i++)
		dprintk(verbose, DST_CA_INFO, 0, " %d", slot_cap[i]);
	dprintk(verbose, DST_CA_INFO, 0, "\n");

	p_ca_caps->slot_num = 1;
	p_ca_caps->slot_type = 1;
	p_ca_caps->descr_num = slot_cap[7];
	p_ca_caps->descr_type = 1;

	if (copy_to_user((struct ca_caps *)arg, p_ca_caps, sizeof (struct ca_caps)))
		return -EFAULT;

	return 0;
}

/*	Need some more work	*/
static int ca_get_slot_descr(struct dst_state *state, struct ca_msg *p_ca_message, void *arg)
{
	return -EOPNOTSUPP;
}


static int ca_get_slot_info(struct dst_state *state, struct ca_slot_info *p_ca_slot_info, void *arg)
{
	int i;
	static u8 slot_command[8] = {0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff};

	u8 *slot_info = state->rxbuffer;

	put_checksum(&slot_command[0], 7);
	if ((dst_put_ci(state, slot_command, sizeof (slot_command), slot_info, GET_REPLY)) < 0) {
		dprintk(verbose, DST_CA_ERROR, 1, " -->dst_put_ci FAILED !");
		return -1;
	}
	dprintk(verbose, DST_CA_INFO, 1, " -->dst_put_ci SUCCESS !");

	/*	Will implement the rest soon		*/

	dprintk(verbose, DST_CA_INFO, 1, " Slot info = [%d]", slot_info[3]);
	dprintk(verbose, DST_CA_INFO, 0, "===================================\n");
	for (i = 0; i < 8; i++)
		dprintk(verbose, DST_CA_INFO, 0, " %d", slot_info[i]);
	dprintk(verbose, DST_CA_INFO, 0, "\n");

	if (slot_info[4] & 0x80) {
		p_ca_slot_info->flags = CA_CI_MODULE_PRESENT;
		p_ca_slot_info->num = 1;
		p_ca_slot_info->type = CA_CI;
	} else if (slot_info[4] & 0x40) {
		p_ca_slot_info->flags = CA_CI_MODULE_READY;
		p_ca_slot_info->num = 1;
		p_ca_slot_info->type = CA_CI;
	} else
		p_ca_slot_info->flags = 0;

	if (copy_to_user((struct ca_slot_info *)arg, p_ca_slot_info, sizeof (struct ca_slot_info)))
		return -EFAULT;

	return 0;
}


static int ca_get_message(struct dst_state *state, struct ca_msg *p_ca_message, void *arg)
{
	u8 i = 0;
	u32 command = 0;

	if (copy_from_user(p_ca_message, (void *)arg, sizeof (struct ca_msg)))
		return -EFAULT;

	if (p_ca_message->msg) {
		dprintk(verbose, DST_CA_NOTICE, 1, " Message = [%02x %02x %02x]", p_ca_message->msg[0], p_ca_message->msg[1], p_ca_message->msg[2]);

		for (i = 0; i < 3; i++) {
			command = command | p_ca_message->msg[i];
			if (i < 2)
				command = command << 8;
		}
		dprintk(verbose, DST_CA_NOTICE, 1, " Command=[0x%x]", command);

		switch (command) {
		case CA_APP_INFO:
			memcpy(p_ca_message->msg, state->messages, 128);
			if (copy_to_user((void *)arg, p_ca_message, sizeof (struct ca_msg)) )
				return -EFAULT;
			break;
		}
	}

	return 0;
}

static int handle_dst_tag(struct dst_state *state, struct ca_msg *p_ca_message, struct ca_msg *hw_buffer, u32 length)
{
	if (state->dst_hw_cap & DST_TYPE_HAS_SESSION) {
		hw_buffer->msg[2] = p_ca_message->msg[1];	/*	MSB	*/
		hw_buffer->msg[3] = p_ca_message->msg[2];	/*	LSB	*/
	} else {
		if (length > 247) {
			dprintk(verbose, DST_CA_ERROR, 1, " Message too long ! *** Bailing Out *** !");
			return -1;
		}
		hw_buffer->msg[0] = (length & 0xff) + 7;
		hw_buffer->msg[1] = 0x40;
		hw_buffer->msg[2] = 0x03;
		hw_buffer->msg[3] = 0x00;
		hw_buffer->msg[4] = 0x03;
		hw_buffer->msg[5] = length & 0xff;
		hw_buffer->msg[6] = 0x00;
		/*
		 *	Need to compute length for EN50221 section 8.3.2, for the time being
		 *	assuming 8.3.2 is not applicable
		 */
		memcpy(&hw_buffer->msg[7], &p_ca_message->msg[4], length);
	}
	return 0;
}


static int write_to_8820(struct dst_state *state, struct ca_msg *hw_buffer, u8 length, u8 reply)
{
	if ((dst_put_ci(state, hw_buffer->msg, length, hw_buffer->msg, reply)) < 0) {
		dprintk(verbose, DST_CA_ERROR, 1, " DST-CI Command failed.");
		dprintk(verbose, DST_CA_NOTICE, 1, " Resetting DST.");
		rdc_reset_state(state);
		return -1;
	}
	dprintk(verbose, DST_CA_NOTICE, 1, " DST-CI Command succes.");

	return 0;
}

u32 asn_1_decode(u8 *asn_1_array)
{
	u8 length_field = 0, word_count = 0, count = 0;
	u32 length = 0;

	length_field = asn_1_array[0];
	dprintk(verbose, DST_CA_DEBUG, 1, " Length field=[%02x]", length_field);
	if (length_field < 0x80) {
		length = length_field & 0x7f;
		dprintk(verbose, DST_CA_DEBUG, 1, " Length=[%02x]\n", length);
	} else {
		word_count = length_field & 0x7f;
		for (count = 0; count < word_count; count++) {
			length = (length | asn_1_array[count + 1]) << 8;
			dprintk(verbose, DST_CA_DEBUG, 1, " Length=[%04x]", length);
		}
	}
	return length;
}

static int debug_string(u8 *msg, u32 length, u32 offset)
{
	u32 i;

	dprintk(verbose, DST_CA_DEBUG, 0, " String=[ ");
	for (i = offset; i < length; i++)
		dprintk(verbose, DST_CA_DEBUG, 0, "%02x ", msg[i]);
	dprintk(verbose, DST_CA_DEBUG, 0, "]\n");

	return 0;
}

static int ca_set_pmt(struct dst_state *state, struct ca_msg *p_ca_message, struct ca_msg *hw_buffer, u8 reply, u8 query)
{
	u32 length = 0;
	u8 tag_length = 8;

	length = asn_1_decode(&p_ca_message->msg[3]);
	dprintk(verbose, DST_CA_DEBUG, 1, " CA Message length=[%d]", length);
	debug_string(&p_ca_message->msg[4], length, 0); /*	length is excluding tag & length	*/

	memset(hw_buffer->msg, '\0', length);
	handle_dst_tag(state, p_ca_message, hw_buffer, length);
	put_checksum(hw_buffer->msg, hw_buffer->msg[0]);

	debug_string(hw_buffer->msg, (length + tag_length), 0); /*	tags too	*/
	write_to_8820(state, hw_buffer, (length + tag_length), reply);

	return 0;
}


/*	Board supports CA PMT reply ?		*/
static int dst_check_ca_pmt(struct dst_state *state, struct ca_msg *p_ca_message, struct ca_msg *hw_buffer)
{
	int ca_pmt_reply_test = 0;

	/*	Do test board			*/
	/*	Not there yet but soon		*/

	/*	CA PMT Reply capable		*/
	if (ca_pmt_reply_test) {
		if ((ca_set_pmt(state, p_ca_message, hw_buffer, 1, GET_REPLY)) < 0) {
			dprintk(verbose, DST_CA_ERROR, 1, " ca_set_pmt.. failed !");
			return -1;
		}

	/*	Process CA PMT Reply		*/
	/*	will implement soon		*/
		dprintk(verbose, DST_CA_ERROR, 1, " Not there yet");
	}
	/*	CA PMT Reply not capable	*/
	if (!ca_pmt_reply_test) {
		if ((ca_set_pmt(state, p_ca_message, hw_buffer, 0, NO_REPLY)) < 0) {
			dprintk(verbose, DST_CA_ERROR, 1, " ca_set_pmt.. failed !");
			return -1;
		}
		dprintk(verbose, DST_CA_NOTICE, 1, " ca_set_pmt.. success !");
	/*	put a dummy message		*/

	}
	return 0;
}

static int ca_send_message(struct dst_state *state, struct ca_msg *p_ca_message, void *arg)
{
	int i = 0;
	unsigned int ca_message_header_len;

	u32 command = 0;
	struct ca_msg *hw_buffer;

	if ((hw_buffer = (struct ca_msg *) kmalloc(sizeof (struct ca_msg), GFP_KERNEL)) == NULL) {
		dprintk(verbose, DST_CA_ERROR, 1, " Memory allocation failure");
		return -ENOMEM;
	}
	dprintk(verbose, DST_CA_DEBUG, 1, " ");

	if (copy_from_user(p_ca_message, (void *)arg, sizeof (struct ca_msg)))
		return -EFAULT;

	if (p_ca_message->msg) {
		ca_message_header_len = p_ca_message->length;	/*	Restore it back when you are done	*/
		/*	EN50221 tag	*/
		command = 0;

		for (i = 0; i < 3; i++) {
			command = command | p_ca_message->msg[i];
			if (i < 2)
				command = command << 8;
		}
		dprintk(verbose, DST_CA_DEBUG, 1, " Command=[0x%x]\n", command);

		switch (command) {
		case CA_PMT:
			dprintk(verbose, DST_CA_DEBUG, 1, "Command = SEND_CA_PMT");
			if ((ca_set_pmt(state, p_ca_message, hw_buffer, 0, 0)) < 0) {	// code simplification started
				dprintk(verbose, DST_CA_ERROR, 1, " -->CA_PMT Failed !");
				return -1;
			}
			dprintk(verbose, DST_CA_INFO, 1, " -->CA_PMT Success !");
			break;
		case CA_PMT_REPLY:
			dprintk(verbose, DST_CA_INFO, 1, "Command = CA_PMT_REPLY");
			/*      Have to handle the 2 basic types of cards here  */
			if ((dst_check_ca_pmt(state, p_ca_message, hw_buffer)) < 0) {
				dprintk(verbose, DST_CA_ERROR, 1, " -->CA_PMT_REPLY Failed !");
				return -1;
			}
			dprintk(verbose, DST_CA_INFO, 1, " -->CA_PMT_REPLY Success !");
			break;
		case CA_APP_INFO_ENQUIRY:		// only for debugging
			dprintk(verbose, DST_CA_INFO, 1, " Getting Cam Application information");

			if ((ca_get_app_info(state)) < 0) {
				dprintk(verbose, DST_CA_ERROR, 1, " -->CA_APP_INFO_ENQUIRY Failed !");
				return -1;
			}
			dprintk(verbose, DST_CA_INFO, 1, " -->CA_APP_INFO_ENQUIRY Success !");
			break;
		}
	}
	return 0;
}

static int dst_ca_ioctl(struct inode *inode, struct file *file, unsigned int cmd, void *arg)
{
	struct dvb_device* dvbdev = (struct dvb_device*) file->private_data;
	struct dst_state* state = (struct dst_state*) dvbdev->priv;
	struct ca_slot_info *p_ca_slot_info;
	struct ca_caps *p_ca_caps;
	struct ca_msg *p_ca_message;

	if ((p_ca_message = (struct ca_msg *) kmalloc(sizeof (struct ca_msg), GFP_KERNEL)) == NULL) {
		dprintk(verbose, DST_CA_ERROR, 1, " Memory allocation failure");
		return -ENOMEM;
	}
	if ((p_ca_slot_info = (struct ca_slot_info *) kmalloc(sizeof (struct ca_slot_info), GFP_KERNEL)) == NULL) {
		dprintk(verbose, DST_CA_ERROR, 1, " Memory allocation failure");
		return -ENOMEM;
	}
	if ((p_ca_caps = (struct ca_caps *) kmalloc(sizeof (struct ca_caps), GFP_KERNEL)) == NULL) {
		dprintk(verbose, DST_CA_ERROR, 1, " Memory allocation failure");
		return -ENOMEM;
	}
	/*	We have now only the standard ioctl's, the driver is upposed to handle internals.	*/
	switch (cmd) {
	case CA_SEND_MSG:
		dprintk(verbose, DST_CA_INFO, 1, " Sending message");
		if ((ca_send_message(state, p_ca_message, arg)) < 0) {
			dprintk(verbose, DST_CA_ERROR, 1, " -->CA_SEND_MSG Failed !");
			return -1;
		}
		break;
	case CA_GET_MSG:
		dprintk(verbose, DST_CA_INFO, 1, " Getting message");
		if ((ca_get_message(state, p_ca_message, arg)) < 0) {
			dprintk(verbose, DST_CA_ERROR, 1, " -->CA_GET_MSG Failed !");
			return -1;
		}
		dprintk(verbose, DST_CA_INFO, 1, " -->CA_GET_MSG Success !");
		break;
	case CA_RESET:
		dprintk(verbose, DST_CA_ERROR, 1, " Resetting DST");
		dst_error_bailout(state);
		msleep(4000);
		break;
	case CA_GET_SLOT_INFO:
		dprintk(verbose, DST_CA_INFO, 1, " Getting Slot info");
		if ((ca_get_slot_info(state, p_ca_slot_info, arg)) < 0) {
			dprintk(verbose, DST_CA_ERROR, 1, " -->CA_GET_SLOT_INFO Failed !");
			return -1;
		}
		dprintk(verbose, DST_CA_INFO, 1, " -->CA_GET_SLOT_INFO Success !");
		break;
	case CA_GET_CAP:
		dprintk(verbose, DST_CA_INFO, 1, " Getting Slot capabilities");
		if ((ca_get_slot_caps(state, p_ca_caps, arg)) < 0) {
			dprintk(verbose, DST_CA_ERROR, 1, " -->CA_GET_CAP Failed !");
			return -1;
		}
		dprintk(verbose, DST_CA_INFO, 1, " -->CA_GET_CAP Success !");
		break;
	case CA_GET_DESCR_INFO:
		dprintk(verbose, DST_CA_INFO, 1, " Getting descrambler description");
		if ((ca_get_slot_descr(state, p_ca_message, arg)) < 0) {
			dprintk(verbose, DST_CA_ERROR, 1, " -->CA_GET_DESCR_INFO Failed !");
			return -1;
		}
		dprintk(verbose, DST_CA_INFO, 1, " -->CA_GET_DESCR_INFO Success !");
		break;
	case CA_SET_DESCR:
		dprintk(verbose, DST_CA_INFO, 1, " Setting descrambler");
		if ((ca_set_slot_descr()) < 0) {
			dprintk(verbose, DST_CA_ERROR, 1, " -->CA_SET_DESCR Failed !");
			return -1;
		}
		dprintk(verbose, DST_CA_INFO, 1, " -->CA_SET_DESCR Success !");
		break;
	case CA_SET_PID:
		dprintk(verbose, DST_CA_INFO, 1, " Setting PID");
		if ((ca_set_pid()) < 0) {
			dprintk(verbose, DST_CA_ERROR, 1, " -->CA_SET_PID Failed !");
			return -1;
		}
		dprintk(verbose, DST_CA_INFO, 1, " -->CA_SET_PID Success !");
	default:
		return -EOPNOTSUPP;
	};

	return 0;
}

static int dst_ca_open(struct inode *inode, struct file *file)
{
	dprintk(verbose, DST_CA_DEBUG, 1, " Device opened [%p] ", file);
	try_module_get(THIS_MODULE);

	return 0;
}

static int dst_ca_release(struct inode *inode, struct file *file)
{
	dprintk(verbose, DST_CA_DEBUG, 1, " Device closed.");
	module_put(THIS_MODULE);

	return 0;
}

static int dst_ca_read(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
	int bytes_read = 0;

	dprintk(verbose, DST_CA_DEBUG, 1, " Device read.");

	return bytes_read;
}

static int dst_ca_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset)
{
	dprintk(verbose, DST_CA_DEBUG, 1, " Device write.");

	return 0;
}

static struct file_operations dst_ca_fops = {
	.owner = THIS_MODULE,
	.ioctl = (void *)dst_ca_ioctl,
	.open = dst_ca_open,
	.release = dst_ca_release,
	.read = dst_ca_read,
	.write = dst_ca_write
};

static struct dvb_device dvbdev_ca = {
	.priv = NULL,
	.users = 1,
	.readers = 1,
	.writers = 1,
	.fops = &dst_ca_fops
};

int dst_ca_attach(struct dst_state *dst, struct dvb_adapter *dvb_adapter)
{
	struct dvb_device *dvbdev;
	dprintk(verbose, DST_CA_ERROR, 1, "registering DST-CA device");
	dvb_register_device(dvb_adapter, &dvbdev, &dvbdev_ca, dst, DVB_DEVICE_CA);
	return 0;
}

EXPORT_SYMBOL(dst_ca_attach);

MODULE_DESCRIPTION("DST DVB-S/T/C Combo CA driver");
MODULE_AUTHOR("Manu Abraham");
MODULE_LICENSE("GPL");
