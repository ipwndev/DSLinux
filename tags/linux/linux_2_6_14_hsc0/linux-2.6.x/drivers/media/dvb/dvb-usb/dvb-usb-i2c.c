/* dvb-usb-i2c.c is part of the DVB USB library.
 *
 * Copyright (C) 2004-5 Patrick Boettcher (patrick.boettcher@desy.de)
 * see dvb-usb-init.c for copyright information.
 *
 * This file contains functions for (de-)initializing an I2C adapter.
 */
#include "dvb-usb-common.h"

int dvb_usb_i2c_init(struct dvb_usb_device *d)
{
	int ret = 0;

	if (!(d->props.caps & DVB_USB_IS_AN_I2C_ADAPTER))
		return 0;

	if (d->props.i2c_algo == NULL) {
		err("no i2c algorithm specified");
		return -EINVAL;
	}

	strncpy(d->i2c_adap.name,d->desc->name,I2C_NAME_SIZE);
#ifdef I2C_ADAP_CLASS_TV_DIGITAL
	d->i2c_adap.class = I2C_ADAP_CLASS_TV_DIGITAL,
#else
	d->i2c_adap.class = I2C_CLASS_TV_DIGITAL,
#endif
	d->i2c_adap.algo      = d->props.i2c_algo;
	d->i2c_adap.algo_data = NULL;

	i2c_set_adapdata(&d->i2c_adap, d);

	if ((ret = i2c_add_adapter(&d->i2c_adap)) < 0)
		err("could not add i2c adapter");

	d->state |= DVB_USB_STATE_I2C;

	return ret;
}

int dvb_usb_i2c_exit(struct dvb_usb_device *d)
{
	if (d->state & DVB_USB_STATE_I2C)
		i2c_del_adapter(&d->i2c_adap);
	d->state &= ~DVB_USB_STATE_I2C;
	return 0;
}

int dvb_usb_pll_init_i2c(struct dvb_frontend *fe)
{
	struct dvb_usb_device *d = fe->dvb->priv;
	struct i2c_msg msg = { .addr = d->pll_addr, .flags = 0, .buf = d->pll_init, .len = 4 };
	int ret = 0;

	/* if there is nothing to initialize */
	if (d->pll_init[0] == 0x00 && d->pll_init[1] == 0x00 &&
		d->pll_init[2] == 0x00 && d->pll_init[3] == 0x00)
		return 0;

	if (d->tuner_pass_ctrl)
		d->tuner_pass_ctrl(fe,1,d->pll_addr);

	deb_pll("pll init: %x\n",d->pll_addr);
	deb_pll("pll-buf: %x %x %x %x\n",d->pll_init[0],d->pll_init[1],
			d->pll_init[2],d->pll_init[3]);

	if (i2c_transfer (&d->i2c_adap, &msg, 1) != 1) {
		err("tuner i2c write failed for pll_init.");
		ret = -EREMOTEIO;
	}
	msleep(1);

	if (d->tuner_pass_ctrl)
		d->tuner_pass_ctrl(fe,0,d->pll_addr);
	return ret;
}
EXPORT_SYMBOL(dvb_usb_pll_init_i2c);

int dvb_usb_pll_set(struct dvb_frontend *fe, struct dvb_frontend_parameters *fep, u8 b[5])
{
	struct dvb_usb_device *d = fe->dvb->priv;

	deb_pll("pll addr: %x, freq: %d %p\n",d->pll_addr,fep->frequency,d->pll_desc);

	b[0] = d->pll_addr << 1;
	dvb_pll_configure(d->pll_desc,&b[1],fep->frequency,fep->u.ofdm.bandwidth);

	deb_pll("pll-buf: %x %x %x %x %x\n",b[0],b[1],b[2],b[3],b[4]);

	return 0;
}
EXPORT_SYMBOL(dvb_usb_pll_set);

int dvb_usb_pll_set_i2c(struct dvb_frontend *fe, struct dvb_frontend_parameters *fep)
{
	struct dvb_usb_device *d = fe->dvb->priv;
	int ret = 0;
	u8 b[5];
	struct i2c_msg msg = { .addr = d->pll_addr, .flags = 0, .buf = &b[1], .len = 4 };

	dvb_usb_pll_set(fe,fep,b);

	if (d->tuner_pass_ctrl)
		d->tuner_pass_ctrl(fe,1,d->pll_addr);

	if (i2c_transfer (&d->i2c_adap, &msg, 1) != 1) {
		err("tuner i2c write failed for pll_set.");
		ret = -EREMOTEIO;
	}
	msleep(1);

	if (d->tuner_pass_ctrl)
		d->tuner_pass_ctrl(fe,0,d->pll_addr);

	return ret;
}
EXPORT_SYMBOL(dvb_usb_pll_set_i2c);
