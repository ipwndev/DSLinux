/*
 * adm9240.c	Part of lm_sensors, Linux kernel modules for hardware
 * 		monitoring
 *
 * Copyright (C) 1999	Frodo Looijaard <frodol@dds.nl>
 *			Philip Edelbrock <phil@netroedge.com>
 * Copyright (C) 2003	Michiel Rook <michiel@grendelproject.nl>
 * Copyright (C) 2005	Grant Coady <gcoady@gmail.com> with valuable
 * 				guidance from Jean Delvare
 *
 * Driver supports	Analog Devices		ADM9240
 *			Dallas Semiconductor	DS1780
 *			National Semiconductor	LM81
 *
 * ADM9240 is the reference, DS1780 and LM81 are register compatibles
 *
 * Voltage	Six inputs are scaled by chip, VID also reported
 * Temperature	Chip temperature to 0.5'C, maximum and max_hysteris
 * Fans		2 fans, low speed alarm, automatic fan clock divider
 * Alarms	16-bit map of active alarms
 * Analog Out	0..1250 mV output
 *
 * Chassis Intrusion: clear CI latch with 'echo 1 > chassis_clear'
 *
 * Test hardware: Intel SE440BX-2 desktop motherboard --Grant
 *
 * LM81 extended temp reading not implemented
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-vid.h>
#include <linux/err.h>

/* Addresses to scan */
static unsigned short normal_i2c[] = { 0x2c, 0x2d, 0x2e, 0x2f,
					I2C_CLIENT_END };

/* Insmod parameters */
I2C_CLIENT_INSMOD_3(adm9240, ds1780, lm81);

/* ADM9240 registers */
#define ADM9240_REG_MAN_ID		0x3e
#define ADM9240_REG_DIE_REV		0x3f
#define ADM9240_REG_CONFIG		0x40

#define ADM9240_REG_IN(nr)		(0x20 + (nr))   /* 0..5 */
#define ADM9240_REG_IN_MAX(nr)		(0x2b + (nr) * 2)
#define ADM9240_REG_IN_MIN(nr)		(0x2c + (nr) * 2)
#define ADM9240_REG_FAN(nr)		(0x28 + (nr))   /* 0..1 */
#define ADM9240_REG_FAN_MIN(nr)		(0x3b + (nr))
#define ADM9240_REG_INT(nr)		(0x41 + (nr))
#define ADM9240_REG_INT_MASK(nr)	(0x43 + (nr))
#define ADM9240_REG_TEMP		0x27
#define ADM9240_REG_TEMP_HIGH		0x39
#define ADM9240_REG_TEMP_HYST		0x3a
#define ADM9240_REG_ANALOG_OUT		0x19
#define ADM9240_REG_CHASSIS_CLEAR	0x46
#define ADM9240_REG_VID_FAN_DIV		0x47
#define ADM9240_REG_I2C_ADDR		0x48
#define ADM9240_REG_VID4		0x49
#define ADM9240_REG_TEMP_CONF		0x4b

/* generalised scaling with integer rounding */
static inline int SCALE(long val, int mul, int div)
{
	if (val < 0)
		return (val * mul - div / 2) / div;
	else
		return (val * mul + div / 2) / div;
}

/* adm9240 internally scales voltage measurements */
static const u16 nom_mv[] = { 2500, 2700, 3300, 5000, 12000, 2700 };

static inline unsigned int IN_FROM_REG(u8 reg, int n)
{
	return SCALE(reg, nom_mv[n], 192);
}

static inline u8 IN_TO_REG(unsigned long val, int n)
{
	return SENSORS_LIMIT(SCALE(val, 192, nom_mv[n]), 0, 255);
}

/* temperature range: -40..125, 127 disables temperature alarm */
static inline s8 TEMP_TO_REG(long val)
{
	return SENSORS_LIMIT(SCALE(val, 1, 1000), -40, 127);
}

/* two fans, each with low fan speed limit */
static inline unsigned int FAN_FROM_REG(u8 reg, u8 div)
{
	if (!reg) /* error */
		return -1;

	if (reg == 255)
		return 0;

	return SCALE(1350000, 1, reg * div);
}

/* analog out 0..1250mV */
static inline u8 AOUT_TO_REG(unsigned long val)
{
	return SENSORS_LIMIT(SCALE(val, 255, 1250), 0, 255);
}

static inline unsigned int AOUT_FROM_REG(u8 reg)
{
	return SCALE(reg, 1250, 255);
}

static int adm9240_attach_adapter(struct i2c_adapter *adapter);
static int adm9240_detect(struct i2c_adapter *adapter, int address, int kind);
static void adm9240_init_client(struct i2c_client *client);
static int adm9240_detach_client(struct i2c_client *client);
static struct adm9240_data *adm9240_update_device(struct device *dev);

/* driver data */
static struct i2c_driver adm9240_driver = {
	.owner		= THIS_MODULE,
	.name		= "adm9240",
	.id		= I2C_DRIVERID_ADM9240,
	.flags		= I2C_DF_NOTIFY,
	.attach_adapter	= adm9240_attach_adapter,
	.detach_client	= adm9240_detach_client,
};

/* per client data */
struct adm9240_data {
	enum chips type;
	struct i2c_client client;
	struct class_device *class_dev;
	struct semaphore update_lock;
	char valid;
	unsigned long last_updated_measure;
	unsigned long last_updated_config;

	u8 in[6];		/* ro	in0_input */
	u8 in_max[6];		/* rw	in0_max */
	u8 in_min[6];		/* rw	in0_min */
	u8 fan[2];		/* ro	fan1_input */
	u8 fan_min[2];		/* rw	fan1_min */
	u8 fan_div[2];		/* rw	fan1_div, read-only accessor */
	s16 temp;		/* ro	temp1_input, 9-bit sign-extended */
	s8 temp_high;		/* rw	temp1_max */
	s8 temp_hyst;		/* rw	temp1_max_hyst */
	u16 alarms;		/* ro	alarms */
	u8 aout;		/* rw	aout_output */
	u8 vid;			/* ro	vid */
	u8 vrm;			/* --	vrm set on startup, no accessor */
};

/* i2c byte read/write interface */
static int adm9240_read_value(struct i2c_client *client, u8 reg)
{
	return i2c_smbus_read_byte_data(client, reg);
}

static int adm9240_write_value(struct i2c_client *client, u8 reg, u8 value)
{
	return i2c_smbus_write_byte_data(client, reg, value);
}

/*** sysfs accessors ***/

/* temperature */
#define show_temp(value, scale)					\
static ssize_t show_##value(struct device *dev,			\
			    struct device_attribute *attr,	\
			    char *buf)				\
{								\
	struct adm9240_data *data = adm9240_update_device(dev);	\
	return sprintf(buf, "%d\n", data->value * scale);	\
}
show_temp(temp_high, 1000);
show_temp(temp_hyst, 1000);
show_temp(temp, 500); /* 0.5'C per bit */

#define set_temp(value, reg)					\
static ssize_t set_##value(struct device *dev, 			\
			   struct device_attribute *attr,	\
			   const char *buf, size_t count)	\
{								\
	struct i2c_client *client = to_i2c_client(dev);		\
	struct adm9240_data *data = adm9240_update_device(dev);	\
	long temp = simple_strtoul(buf, NULL, 10);		\
								\
	down(&data->update_lock);				\
	data->value = TEMP_TO_REG(temp);			\
	adm9240_write_value(client, reg, data->value);		\
	up(&data->update_lock);					\
	return count;						\
}

set_temp(temp_high, ADM9240_REG_TEMP_HIGH);
set_temp(temp_hyst, ADM9240_REG_TEMP_HYST);

static DEVICE_ATTR(temp1_max, S_IWUSR | S_IRUGO,
		show_temp_high, set_temp_high);
static DEVICE_ATTR(temp1_max_hyst, S_IWUSR | S_IRUGO,
		show_temp_hyst, set_temp_hyst);
static DEVICE_ATTR(temp1_input, S_IRUGO, show_temp, NULL);

/* voltage */
static ssize_t show_in(struct device *dev, char *buf, int nr)
{
	struct adm9240_data *data = adm9240_update_device(dev);
	return sprintf(buf, "%d\n", IN_FROM_REG(data->in[nr], nr));
}

static ssize_t show_in_min(struct device *dev, char *buf, int nr)
{
	struct adm9240_data *data = adm9240_update_device(dev);
	return sprintf(buf, "%d\n", IN_FROM_REG(data->in_min[nr], nr));
}

static ssize_t show_in_max(struct device *dev, char *buf, int nr)
{
	struct adm9240_data *data = adm9240_update_device(dev);
	return sprintf(buf, "%d\n", IN_FROM_REG(data->in_max[nr], nr));
}

static ssize_t set_in_min(struct device *dev, const char *buf,
		size_t count, int nr)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct adm9240_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);

	down(&data->update_lock);
	data->in_min[nr] = IN_TO_REG(val, nr);
	adm9240_write_value(client, ADM9240_REG_IN_MIN(nr), data->in_min[nr]);
	up(&data->update_lock);
	return count;
}

static ssize_t set_in_max(struct device *dev, const char *buf,
		size_t count, int nr)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct adm9240_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);

	down(&data->update_lock);
	data->in_max[nr] = IN_TO_REG(val, nr);
	adm9240_write_value(client, ADM9240_REG_IN_MAX(nr), data->in_max[nr]);
	up(&data->update_lock);
	return count;
}

#define show_in_offset(offset)						\
static ssize_t show_in##offset(struct device *dev,			\
			       struct device_attribute *attr,		\
			       char *buf)				\
{									\
	return show_in(dev, buf, offset);				\
}									\
static DEVICE_ATTR(in##offset##_input, S_IRUGO, show_in##offset, NULL);	\
static ssize_t show_in##offset##_min(struct device *dev,		\
				     struct device_attribute *attr,	\
				     char *buf)				\
{									\
	return show_in_min(dev, buf, offset);				\
}									\
static ssize_t show_in##offset##_max(struct device *dev,		\
				     struct device_attribute *attr,	\
				     char *buf)				\
{									\
	return show_in_max(dev, buf, offset);				\
}									\
static ssize_t								\
set_in##offset##_min(struct device *dev,				\
		     struct device_attribute *attr, const char *buf,	\
		     size_t count)					\
{									\
	return set_in_min(dev, buf, count, offset);			\
}									\
static ssize_t								\
set_in##offset##_max(struct device *dev,				\
		     struct device_attribute *attr, const char *buf,	\
		     size_t count)					\
{									\
	return set_in_max(dev, buf, count, offset);			\
}									\
static DEVICE_ATTR(in##offset##_min, S_IRUGO | S_IWUSR,			\
		show_in##offset##_min, set_in##offset##_min);		\
static DEVICE_ATTR(in##offset##_max, S_IRUGO | S_IWUSR,			\
		show_in##offset##_max, set_in##offset##_max);

show_in_offset(0);
show_in_offset(1);
show_in_offset(2);
show_in_offset(3);
show_in_offset(4);
show_in_offset(5);

/* fans */
static ssize_t show_fan(struct device *dev, char *buf, int nr)
{
	struct adm9240_data *data = adm9240_update_device(dev);
	return sprintf(buf, "%d\n", FAN_FROM_REG(data->fan[nr],
				1 << data->fan_div[nr]));
}

static ssize_t show_fan_min(struct device *dev, char *buf, int nr)
{
	struct adm9240_data *data = adm9240_update_device(dev);
	return sprintf(buf, "%d\n", FAN_FROM_REG(data->fan_min[nr],
				1 << data->fan_div[nr]));
}

static ssize_t show_fan_div(struct device *dev, char *buf, int nr)
{
	struct adm9240_data *data = adm9240_update_device(dev);
	return sprintf(buf, "%d\n", 1 << data->fan_div[nr]);
}

/* write new fan div, callers must hold data->update_lock */
static void adm9240_write_fan_div(struct i2c_client *client, int nr,
		u8 fan_div)
{
	u8 reg, old, shift = (nr + 2) * 2;

	reg = adm9240_read_value(client, ADM9240_REG_VID_FAN_DIV);
	old = (reg >> shift) & 3;
	reg &= ~(3 << shift);
	reg |= (fan_div << shift);
	adm9240_write_value(client, ADM9240_REG_VID_FAN_DIV, reg);
	dev_dbg(&client->dev, "fan%d clock divider changed from %u "
			"to %u\n", nr + 1, 1 << old, 1 << fan_div);
}

/* 
 * set fan speed low limit:
 *
 * - value is zero: disable fan speed low limit alarm
 *
 * - value is below fan speed measurement range: enable fan speed low
 *   limit alarm to be asserted while fan speed too slow to measure
 *
 * - otherwise: select fan clock divider to suit fan speed low limit,
 *   measurement code may adjust registers to ensure fan speed reading
 */
static ssize_t set_fan_min(struct device *dev, const char *buf,
		size_t count, int nr)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct adm9240_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtoul(buf, NULL, 10);
	u8 new_div;

	down(&data->update_lock);

	if (!val) {
		data->fan_min[nr] = 255;
		new_div = data->fan_div[nr];

		dev_dbg(&client->dev, "fan%u low limit set disabled\n",
				nr + 1);

	} else if (val < 1350000 / (8 * 254)) {
		new_div = 3;
		data->fan_min[nr] = 254;

		dev_dbg(&client->dev, "fan%u low limit set minimum %u\n",
				nr + 1, FAN_FROM_REG(254, 1 << new_div));

	} else {
		unsigned int new_min = 1350000 / val;

		new_div = 0;
		while (new_min > 192 && new_div < 3) {
			new_div++;
			new_min /= 2;
		}
		if (!new_min) /* keep > 0 */
			new_min++;

		data->fan_min[nr] = new_min;

		dev_dbg(&client->dev, "fan%u low limit set fan speed %u\n",
				nr + 1, FAN_FROM_REG(new_min, 1 << new_div));
	}

	if (new_div != data->fan_div[nr]) {
		data->fan_div[nr] = new_div;
		adm9240_write_fan_div(client, nr, new_div);
	}
	adm9240_write_value(client, ADM9240_REG_FAN_MIN(nr),
			data->fan_min[nr]);

	up(&data->update_lock);
	return count;
}

#define show_fan_offset(offset)						\
static ssize_t show_fan_##offset (struct device *dev,			\
				  struct device_attribute *attr,	\
				  char *buf)				\
{									\
return show_fan(dev, buf, offset - 1);					\
}									\
static ssize_t show_fan_##offset##_div (struct device *dev,		\
					struct device_attribute *attr,	\
					char *buf)			\
{									\
return show_fan_div(dev, buf, offset - 1);				\
}									\
static ssize_t show_fan_##offset##_min (struct device *dev,		\
					struct device_attribute *attr,	\
					char *buf)			\
{									\
return show_fan_min(dev, buf, offset - 1);				\
}									\
static ssize_t set_fan_##offset##_min (struct device *dev, 		\
				       struct device_attribute *attr,	\
				       const char *buf, size_t count)	\
{									\
return set_fan_min(dev, buf, count, offset - 1);			\
}									\
static DEVICE_ATTR(fan##offset##_input, S_IRUGO, 			\
		show_fan_##offset, NULL);				\
static DEVICE_ATTR(fan##offset##_div, S_IRUGO, 				\
		show_fan_##offset##_div, NULL);				\
static DEVICE_ATTR(fan##offset##_min, S_IRUGO | S_IWUSR, 		\
		show_fan_##offset##_min, set_fan_##offset##_min);

show_fan_offset(1);
show_fan_offset(2);

/* alarms */
static ssize_t show_alarms(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct adm9240_data *data = adm9240_update_device(dev);
	return sprintf(buf, "%u\n", data->alarms);
}
static DEVICE_ATTR(alarms, S_IRUGO, show_alarms, NULL);

/* vid */
static ssize_t show_vid(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct adm9240_data *data = adm9240_update_device(dev);
	return sprintf(buf, "%d\n", vid_from_reg(data->vid, data->vrm));
}
static DEVICE_ATTR(cpu0_vid, S_IRUGO, show_vid, NULL);

/* analog output */
static ssize_t show_aout(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct adm9240_data *data = adm9240_update_device(dev);
	return sprintf(buf, "%d\n", AOUT_FROM_REG(data->aout));
}

static ssize_t set_aout(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct adm9240_data *data = i2c_get_clientdata(client);
	unsigned long val = simple_strtol(buf, NULL, 10);

	down(&data->update_lock);
	data->aout = AOUT_TO_REG(val);
	adm9240_write_value(client, ADM9240_REG_ANALOG_OUT, data->aout);
	up(&data->update_lock);
	return count;
}
static DEVICE_ATTR(aout_output, S_IRUGO | S_IWUSR, show_aout, set_aout);

/* chassis_clear */
static ssize_t chassis_clear(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	unsigned long val = simple_strtol(buf, NULL, 10);

	if (val == 1) {
		adm9240_write_value(client, ADM9240_REG_CHASSIS_CLEAR, 0x80);
		dev_dbg(&client->dev, "chassis intrusion latch cleared\n");
	}
	return count;
}
static DEVICE_ATTR(chassis_clear, S_IWUSR, NULL, chassis_clear);


/*** sensor chip detect and driver install ***/

static int adm9240_detect(struct i2c_adapter *adapter, int address, int kind)
{
	struct i2c_client *new_client;
	struct adm9240_data *data;
	int err = 0;
	const char *name = "";
	u8 man_id, die_rev;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		goto exit;

	if (!(data = kmalloc(sizeof(struct adm9240_data), GFP_KERNEL))) {
		err = -ENOMEM;
		goto exit;
	}
	memset(data, 0, sizeof(struct adm9240_data));

	new_client = &data->client;
	i2c_set_clientdata(new_client, data);
	new_client->addr = address;
	new_client->adapter = adapter;
	new_client->driver = &adm9240_driver;
	new_client->flags = 0;

	if (kind == 0) {
		kind = adm9240;
	}

	if (kind < 0) {

		/* verify chip: reg address should match i2c address */
		if (adm9240_read_value(new_client, ADM9240_REG_I2C_ADDR)
				!= address) {
			dev_err(&adapter->dev, "detect fail: address match, "
					"0x%02x\n", address);
			goto exit_free;
		}

		/* check known chip manufacturer */
		man_id = adm9240_read_value(new_client, ADM9240_REG_MAN_ID);

		if (man_id == 0x23) {
			kind = adm9240;
		} else if (man_id == 0xda) {
			kind = ds1780;
		} else if (man_id == 0x01) {
			kind = lm81;
		} else {
			dev_err(&adapter->dev, "detect fail: unknown manuf, "
					"0x%02x\n", man_id);
			goto exit_free;
		}

		/* successful detect, print chip info */
		die_rev = adm9240_read_value(new_client, ADM9240_REG_DIE_REV);
		dev_info(&adapter->dev, "found %s revision %u\n",
				man_id == 0x23 ? "ADM9240" :
				man_id == 0xda ? "DS1780" : "LM81", die_rev);
	}

	/* either forced or detected chip kind */
	if (kind == adm9240) {
		name = "adm9240";
	} else if (kind == ds1780) {
		name = "ds1780";
	} else if (kind == lm81) {
		name = "lm81";
	}

	/* fill in the remaining client fields and attach */
	strlcpy(new_client->name, name, I2C_NAME_SIZE);
	data->type = kind;
	init_MUTEX(&data->update_lock);

	if ((err = i2c_attach_client(new_client)))
		goto exit_free;

	adm9240_init_client(new_client);

	/* populate sysfs filesystem */
	data->class_dev = hwmon_device_register(&new_client->dev);
	if (IS_ERR(data->class_dev)) {
		err = PTR_ERR(data->class_dev);
		goto exit_detach;
	}

	device_create_file(&new_client->dev, &dev_attr_in0_input);
	device_create_file(&new_client->dev, &dev_attr_in0_min);
	device_create_file(&new_client->dev, &dev_attr_in0_max);
	device_create_file(&new_client->dev, &dev_attr_in1_input);
	device_create_file(&new_client->dev, &dev_attr_in1_min);
	device_create_file(&new_client->dev, &dev_attr_in1_max);
	device_create_file(&new_client->dev, &dev_attr_in2_input);
	device_create_file(&new_client->dev, &dev_attr_in2_min);
	device_create_file(&new_client->dev, &dev_attr_in2_max);
	device_create_file(&new_client->dev, &dev_attr_in3_input);
	device_create_file(&new_client->dev, &dev_attr_in3_min);
	device_create_file(&new_client->dev, &dev_attr_in3_max);
	device_create_file(&new_client->dev, &dev_attr_in4_input);
	device_create_file(&new_client->dev, &dev_attr_in4_min);
	device_create_file(&new_client->dev, &dev_attr_in4_max);
	device_create_file(&new_client->dev, &dev_attr_in5_input);
	device_create_file(&new_client->dev, &dev_attr_in5_min);
	device_create_file(&new_client->dev, &dev_attr_in5_max);
	device_create_file(&new_client->dev, &dev_attr_temp1_max);
	device_create_file(&new_client->dev, &dev_attr_temp1_max_hyst);
	device_create_file(&new_client->dev, &dev_attr_temp1_input);
	device_create_file(&new_client->dev, &dev_attr_fan1_input);
	device_create_file(&new_client->dev, &dev_attr_fan1_div);
	device_create_file(&new_client->dev, &dev_attr_fan1_min);
	device_create_file(&new_client->dev, &dev_attr_fan2_input);
	device_create_file(&new_client->dev, &dev_attr_fan2_div);
	device_create_file(&new_client->dev, &dev_attr_fan2_min);
	device_create_file(&new_client->dev, &dev_attr_alarms);
	device_create_file(&new_client->dev, &dev_attr_aout_output);
	device_create_file(&new_client->dev, &dev_attr_chassis_clear);
	device_create_file(&new_client->dev, &dev_attr_cpu0_vid);

	return 0;

exit_detach:
	i2c_detach_client(new_client);
exit_free:
	kfree(data);
exit:
	return err;
}

static int adm9240_attach_adapter(struct i2c_adapter *adapter)
{
	if (!(adapter->class & I2C_CLASS_HWMON))
		return 0;
	return i2c_probe(adapter, &addr_data, adm9240_detect);
}

static int adm9240_detach_client(struct i2c_client *client)
{
	struct adm9240_data *data = i2c_get_clientdata(client);
	int err;

	hwmon_device_unregister(data->class_dev);

	if ((err = i2c_detach_client(client)))
		return err;

	kfree(data);
	return 0;
}

static void adm9240_init_client(struct i2c_client *client)
{
	struct adm9240_data *data = i2c_get_clientdata(client);
	u8 conf = adm9240_read_value(client, ADM9240_REG_CONFIG);
	u8 mode = adm9240_read_value(client, ADM9240_REG_TEMP_CONF) & 3;

	data->vrm = vid_which_vrm(); /* need this to report vid as mV */

	dev_info(&client->dev, "Using VRM: %d.%d\n", data->vrm / 10,
			data->vrm % 10);

	if (conf & 1) { /* measurement cycle running: report state */

		dev_info(&client->dev, "status: config 0x%02x mode %u\n",
				conf, mode);

	} else { /* cold start: open limits before starting chip */
		int i;

		for (i = 0; i < 6; i++)
		{
			adm9240_write_value(client,
					ADM9240_REG_IN_MIN(i), 0);
			adm9240_write_value(client,
					ADM9240_REG_IN_MAX(i), 255);
		}
		adm9240_write_value(client, ADM9240_REG_FAN_MIN(0), 255);
		adm9240_write_value(client, ADM9240_REG_FAN_MIN(1), 255);
		adm9240_write_value(client, ADM9240_REG_TEMP_HIGH, 127);
		adm9240_write_value(client, ADM9240_REG_TEMP_HYST, 127);

		/* start measurement cycle */
		adm9240_write_value(client, ADM9240_REG_CONFIG, 1);

		dev_info(&client->dev, "cold start: config was 0x%02x "
				"mode %u\n", conf, mode);
	}
}

static struct adm9240_data *adm9240_update_device(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct adm9240_data *data = i2c_get_clientdata(client);
	int i;

	down(&data->update_lock);

	/* minimum measurement cycle: 1.75 seconds */
	if (time_after(jiffies, data->last_updated_measure + (HZ * 7 / 4))
			|| !data->valid) {

		for (i = 0; i < 6; i++) /* read voltages */
		{
			data->in[i] = adm9240_read_value(client,
					ADM9240_REG_IN(i));
		}
		data->alarms = adm9240_read_value(client,
					ADM9240_REG_INT(0)) |
					adm9240_read_value(client,
					ADM9240_REG_INT(1)) << 8;

		/* read temperature: assume temperature changes less than
		 * 0.5'C per two measurement cycles thus ignore possible
		 * but unlikely aliasing error on lsb reading. --Grant */
		data->temp = ((adm9240_read_value(client,
					ADM9240_REG_TEMP) << 8) |
					adm9240_read_value(client,
					ADM9240_REG_TEMP_CONF)) / 128;

		for (i = 0; i < 2; i++) /* read fans */
		{
			data->fan[i] = adm9240_read_value(client,
					ADM9240_REG_FAN(i));

			/* adjust fan clock divider on overflow */
			if (data->valid && data->fan[i] == 255 &&
					data->fan_div[i] < 3) {

				adm9240_write_fan_div(client, i,
						++data->fan_div[i]);

				/* adjust fan_min if active, but not to 0 */
				if (data->fan_min[i] < 255 &&
						data->fan_min[i] >= 2)
					data->fan_min[i] /= 2;
			}
		}
		data->last_updated_measure = jiffies;
	}

	/* minimum config reading cycle: 300 seconds */
	if (time_after(jiffies, data->last_updated_config + (HZ * 300))
			|| !data->valid) {

		for (i = 0; i < 6; i++)
		{
			data->in_min[i] = adm9240_read_value(client,
					ADM9240_REG_IN_MIN(i));
			data->in_max[i] = adm9240_read_value(client,
					ADM9240_REG_IN_MAX(i));
		}
		for (i = 0; i < 2; i++)
		{
			data->fan_min[i] = adm9240_read_value(client,
					ADM9240_REG_FAN_MIN(i));
		}
		data->temp_high = adm9240_read_value(client,
				ADM9240_REG_TEMP_HIGH);
		data->temp_hyst = adm9240_read_value(client,
				ADM9240_REG_TEMP_HYST);

		/* read fan divs and 5-bit VID */
		i = adm9240_read_value(client, ADM9240_REG_VID_FAN_DIV);
		data->fan_div[0] = (i >> 4) & 3;
		data->fan_div[1] = (i >> 6) & 3;
		data->vid = i & 0x0f;
		data->vid |= (adm9240_read_value(client,
					ADM9240_REG_VID4) & 1) << 4;
		/* read analog out */
		data->aout = adm9240_read_value(client,
				ADM9240_REG_ANALOG_OUT);

		data->last_updated_config = jiffies;
		data->valid = 1;
	}
	up(&data->update_lock);
	return data;
}

static int __init sensors_adm9240_init(void)
{
	return i2c_add_driver(&adm9240_driver);
}

static void __exit sensors_adm9240_exit(void)
{
	i2c_del_driver(&adm9240_driver);
}

MODULE_AUTHOR("Michiel Rook <michiel@grendelproject.nl>, "
		"Grant Coady <gcoady@gmail.com> and others");
MODULE_DESCRIPTION("ADM9240/DS1780/LM81 driver");
MODULE_LICENSE("GPL");

module_init(sensors_adm9240_init);
module_exit(sensors_adm9240_exit);

