/*
 *
 * (c) 2004 Gerd Knorr <kraxel@bytesex.org> [SuSE Labs]
 *
 *  Extended 3 / 2005 by Hartmut Hackmann to support various
 *  cards with the tda10046 DVB-T channel decoder
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/init.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/suspend.h>


#include "saa7134-reg.h"
#include "saa7134.h"

#ifdef HAVE_MT352
# include "mt352.h"
# include "mt352_priv.h" /* FIXME */
#endif
#ifdef HAVE_TDA1004X
# include "tda1004x.h"
#endif

MODULE_AUTHOR("Gerd Knorr <kraxel@bytesex.org> [SuSE Labs]");
MODULE_LICENSE("GPL");

static unsigned int antenna_pwr = 0;

module_param(antenna_pwr, int, 0444);
MODULE_PARM_DESC(antenna_pwr,"enable antenna power (Pinnacle 300i)");

/* ------------------------------------------------------------------ */

#ifdef HAVE_MT352
static int pinnacle_antenna_pwr(struct saa7134_dev *dev, int on)
{
	u32 ok;

	if (!on) {
		saa_setl(SAA7134_GPIO_GPMODE0 >> 2,     (1 << 26));
		saa_clearl(SAA7134_GPIO_GPSTATUS0 >> 2, (1 << 26));
		return 0;
	}

	saa_setl(SAA7134_GPIO_GPMODE0 >> 2,     (1 << 26));
	saa_setl(SAA7134_GPIO_GPSTATUS0 >> 2,   (1 << 26));
	udelay(10);

	saa_setl(SAA7134_GPIO_GPMODE0 >> 2,     (1 << 28));
	saa_clearl(SAA7134_GPIO_GPSTATUS0 >> 2, (1 << 28));
	udelay(10);
	saa_setl(SAA7134_GPIO_GPSTATUS0 >> 2,   (1 << 28));
	udelay(10);
	ok = saa_readl(SAA7134_GPIO_GPSTATUS0) & (1 << 27);
	printk("%s: %s %s\n", dev->name, __FUNCTION__,
	       ok ? "on" : "off");

	if (!ok)
		saa_clearl(SAA7134_GPIO_GPSTATUS0 >> 2,   (1 << 26));
	return ok;
}

static int mt352_pinnacle_init(struct dvb_frontend* fe)
{
	static u8 clock_config []  = { CLOCK_CTL,  0x3d, 0x28 };
	static u8 reset []         = { RESET,      0x80 };
	static u8 adc_ctl_1_cfg [] = { ADC_CTL_1,  0x40 };
	static u8 agc_cfg []       = { AGC_TARGET, 0x28, 0xa0 };
	static u8 capt_range_cfg[] = { CAPT_RANGE, 0x31 };
	static u8 fsm_ctl_cfg[]    = { 0x7b,       0x04 };
	static u8 gpp_ctl_cfg []   = { GPP_CTL,    0x0f };
	static u8 scan_ctl_cfg []  = { SCAN_CTL,   0x0d };
	static u8 irq_cfg []       = { INTERRUPT_EN_0, 0x00, 0x00, 0x00, 0x00 };
	struct saa7134_dev *dev= fe->dvb->priv;

	printk("%s: %s called\n",dev->name,__FUNCTION__);

	mt352_write(fe, clock_config,   sizeof(clock_config));
	udelay(200);
	mt352_write(fe, reset,          sizeof(reset));
	mt352_write(fe, adc_ctl_1_cfg,  sizeof(adc_ctl_1_cfg));
	mt352_write(fe, agc_cfg,        sizeof(agc_cfg));
	mt352_write(fe, capt_range_cfg, sizeof(capt_range_cfg));
	mt352_write(fe, gpp_ctl_cfg,    sizeof(gpp_ctl_cfg));

	mt352_write(fe, fsm_ctl_cfg,    sizeof(fsm_ctl_cfg));
	mt352_write(fe, scan_ctl_cfg,   sizeof(scan_ctl_cfg));
	mt352_write(fe, irq_cfg,        sizeof(irq_cfg));
	return 0;
}

static int mt352_pinnacle_pll_set(struct dvb_frontend* fe,
				  struct dvb_frontend_parameters* params,
				  u8* pllbuf)
{
	static int on  = TDA9887_PRESENT | TDA9887_PORT2_INACTIVE;
	static int off = TDA9887_PRESENT | TDA9887_PORT2_ACTIVE;
	struct saa7134_dev *dev = fe->dvb->priv;
	struct v4l2_frequency f;

	/* set frequency (mt2050) */
	f.tuner     = 0;
	f.type      = V4L2_TUNER_DIGITAL_TV;
	f.frequency = params->frequency / 1000 * 16 / 1000;
	saa7134_i2c_call_clients(dev,TDA9887_SET_CONFIG,&on);
	saa7134_i2c_call_clients(dev,VIDIOC_S_FREQUENCY,&f);
	saa7134_i2c_call_clients(dev,TDA9887_SET_CONFIG,&off);

	pinnacle_antenna_pwr(dev, antenna_pwr);

	/* mt352 setup */
	mt352_pinnacle_init(fe);
	pllbuf[0] = 0xc2;
	pllbuf[1] = 0x00;
	pllbuf[2] = 0x00;
	pllbuf[3] = 0x80;
	pllbuf[4] = 0x00;
	return 0;
}

static struct mt352_config pinnacle_300i = {
	.demod_address = 0x3c >> 1,
	.adc_clock     = 20333,
	.if2           = 36150,
	.no_tuner      = 1,
	.demod_init    = mt352_pinnacle_init,
	.pll_set       = mt352_pinnacle_pll_set,
};
#endif

/* ------------------------------------------------------------------ */

#ifdef HAVE_TDA1004X
static int philips_tu1216_pll_init(struct dvb_frontend *fe)
{
	struct saa7134_dev *dev = fe->dvb->priv;
	static u8 tu1216_init[] = { 0x0b, 0xf5, 0x85, 0xab };
	struct i2c_msg tuner_msg = {.addr = 0x60,.flags = 0,.buf = tu1216_init,.len = sizeof(tu1216_init) };

	/* setup PLL configuration */
	if (i2c_transfer(&dev->i2c_adap, &tuner_msg, 1) != 1)
		return -EIO;
	msleep(1);

	return 0;
}

static int philips_tu1216_pll_set(struct dvb_frontend *fe, struct dvb_frontend_parameters *params)
{
	struct saa7134_dev *dev = fe->dvb->priv;
	u8 tuner_buf[4];
	struct i2c_msg tuner_msg = {.addr = 0x60,.flags = 0,.buf = tuner_buf,.len =
			sizeof(tuner_buf) };
	int tuner_frequency = 0;
	u8 band, cp, filter;

	/* determine charge pump */
	tuner_frequency = params->frequency + 36166000;
	if (tuner_frequency < 87000000)
		return -EINVAL;
	else if (tuner_frequency < 130000000)
		cp = 3;
	else if (tuner_frequency < 160000000)
		cp = 5;
	else if (tuner_frequency < 200000000)
		cp = 6;
	else if (tuner_frequency < 290000000)
		cp = 3;
	else if (tuner_frequency < 420000000)
		cp = 5;
	else if (tuner_frequency < 480000000)
		cp = 6;
	else if (tuner_frequency < 620000000)
		cp = 3;
	else if (tuner_frequency < 830000000)
		cp = 5;
	else if (tuner_frequency < 895000000)
		cp = 7;
	else
		return -EINVAL;

	/* determine band */
	if (params->frequency < 49000000)
		return -EINVAL;
	else if (params->frequency < 161000000)
		band = 1;
	else if (params->frequency < 444000000)
		band = 2;
	else if (params->frequency < 861000000)
		band = 4;
	else
		return -EINVAL;

	/* setup PLL filter */
	switch (params->u.ofdm.bandwidth) {
	case BANDWIDTH_6_MHZ:
		filter = 0;
		break;

	case BANDWIDTH_7_MHZ:
		filter = 0;
		break;

	case BANDWIDTH_8_MHZ:
		filter = 1;
		break;

	default:
		return -EINVAL;
	}

	/* calculate divisor
	 * ((36166000+((1000000/6)/2)) + Finput)/(1000000/6)
	 */
	tuner_frequency = (((params->frequency / 1000) * 6) + 217496) / 1000;

	/* setup tuner buffer */
	tuner_buf[0] = (tuner_frequency >> 8) & 0x7f;
	tuner_buf[1] = tuner_frequency & 0xff;
	tuner_buf[2] = 0xca;
	tuner_buf[3] = (cp << 5) | (filter << 3) | band;

	if (i2c_transfer(&dev->i2c_adap, &tuner_msg, 1) != 1)
		return -EIO;

	msleep(1);
	return 0;
}

static int philips_tu1216_request_firmware(struct dvb_frontend *fe,
					   const struct firmware **fw, char *name)
{
	struct saa7134_dev *dev = fe->dvb->priv;
	return request_firmware(fw, name, &dev->pci->dev);
}

static struct tda1004x_config philips_tu1216_config = {

	.demod_address = 0x8,
	.invert        = 1,
	.invert_oclk   = 1,
	.xtal_freq     = TDA10046_XTAL_4M,
	.agc_config    = TDA10046_AGC_DEFAULT,
	.if_freq       = TDA10046_FREQ_3617,
	.pll_init      = philips_tu1216_pll_init,
	.pll_set       = philips_tu1216_pll_set,
	.pll_sleep     = NULL,
	.request_firmware = philips_tu1216_request_firmware,
};

/* ------------------------------------------------------------------ */


static int philips_fmd1216_pll_init(struct dvb_frontend *fe)
{
	struct saa7134_dev *dev = fe->dvb->priv;
	/* this message is to set up ATC and ALC */
	static u8 fmd1216_init[] = { 0x0b, 0xdc, 0x9c, 0xa0 };
	struct i2c_msg tuner_msg = {.addr = 0x61,.flags = 0,.buf = fmd1216_init,.len = sizeof(fmd1216_init) };

	if (i2c_transfer(&dev->i2c_adap, &tuner_msg, 1) != 1)
		return -EIO;
	msleep(1);

	return 0;
}

static void philips_fmd1216_analog(struct dvb_frontend *fe)
{
	struct saa7134_dev *dev = fe->dvb->priv;
	/* this message actually turns the tuner back to analog mode */
	static u8 fmd1216_init[] = { 0x0b, 0xdc, 0x9c, 0x60 };
	struct i2c_msg tuner_msg = {.addr = 0x61,.flags = 0,.buf = fmd1216_init,.len = sizeof(fmd1216_init) };

	i2c_transfer(&dev->i2c_adap, &tuner_msg, 1);
	msleep(1);
	fmd1216_init[2] = 0x86;
	fmd1216_init[3] = 0x54;
	i2c_transfer(&dev->i2c_adap, &tuner_msg, 1);
	msleep(1);
}

static int philips_fmd1216_pll_set(struct dvb_frontend *fe, struct dvb_frontend_parameters *params)
{
	struct saa7134_dev *dev = fe->dvb->priv;
	u8 tuner_buf[4];
	struct i2c_msg tuner_msg = {.addr = 0x61,.flags = 0,.buf = tuner_buf,.len =
			sizeof(tuner_buf) };
	int tuner_frequency = 0;
	int divider = 0;
	u8 band, mode, cp;

	/* determine charge pump */
	tuner_frequency = params->frequency + 36130000;
	if (tuner_frequency < 87000000)
		return -EINVAL;
	/* low band */
	else if (tuner_frequency < 180000000) {
		band = 1;
		mode = 7;
		cp   = 0;
	} else if (tuner_frequency < 195000000) {
		band = 1;
		mode = 6;
		cp   = 1;
	/* mid band	*/
	} else if (tuner_frequency < 366000000) {
		if (params->u.ofdm.bandwidth == BANDWIDTH_8_MHZ) {
			band = 10;
		} else {
			band = 2;
		}
		mode = 7;
		cp   = 0;
	} else if (tuner_frequency < 478000000) {
		if (params->u.ofdm.bandwidth == BANDWIDTH_8_MHZ) {
			band = 10;
		} else {
			band = 2;
		}
		mode = 6;
		cp   = 1;
	/* high band */
	} else if (tuner_frequency < 662000000) {
		if (params->u.ofdm.bandwidth == BANDWIDTH_8_MHZ) {
			band = 12;
		} else {
			band = 4;
		}
		mode = 7;
		cp   = 0;
	} else if (tuner_frequency < 840000000) {
		if (params->u.ofdm.bandwidth == BANDWIDTH_8_MHZ) {
			band = 12;
		} else {
			band = 4;
		}
		mode = 6;
		cp   = 1;
	} else {
		if (params->u.ofdm.bandwidth == BANDWIDTH_8_MHZ) {
			band = 12;
		} else {
			band = 4;
		}
		mode = 7;
		cp   = 1;

	}
	/* calculate divisor */
	/* ((36166000 + Finput) / 166666) rounded! */
	divider = (tuner_frequency + 83333) / 166667;

	/* setup tuner buffer */
	tuner_buf[0] = (divider >> 8) & 0x7f;
	tuner_buf[1] = divider & 0xff;
	tuner_buf[2] = 0x80 | (cp << 6) | (mode  << 3) | 4;
	tuner_buf[3] = 0x40 | band;

	if (i2c_transfer(&dev->i2c_adap, &tuner_msg, 1) != 1)
		return -EIO;
	return 0;
}

#ifdef HAVE_TDA1004X
static struct tda1004x_config medion_cardbus = {
	.demod_address = 0x08,
	.invert        = 1,
	.invert_oclk   = 0,
	.xtal_freq     = TDA10046_XTAL_16M,
	.agc_config    = TDA10046_AGC_IFO_AUTO_NEG,
	.if_freq       = TDA10046_FREQ_3613,
	.pll_init      = philips_fmd1216_pll_init,
	.pll_set       = philips_fmd1216_pll_set,
	.pll_sleep	   = philips_fmd1216_analog,
	.request_firmware = NULL,
};
#endif

/* ------------------------------------------------------------------ */

struct tda827x_data {
	u32 lomax;
	u8  spd;
	u8  bs;
	u8  bp;
	u8  cp;
	u8  gc3;
	u8 div1p5;
};

static struct tda827x_data tda827x_dvbt[] = {
	{ .lomax =  62000000, .spd = 3, .bs = 2, .bp = 0, .cp = 0, .gc3 = 3, .div1p5 = 1},
	{ .lomax =  66000000, .spd = 3, .bs = 3, .bp = 0, .cp = 0, .gc3 = 3, .div1p5 = 1},
	{ .lomax =  76000000, .spd = 3, .bs = 1, .bp = 0, .cp = 0, .gc3 = 3, .div1p5 = 0},
	{ .lomax =  84000000, .spd = 3, .bs = 2, .bp = 0, .cp = 0, .gc3 = 3, .div1p5 = 0},
	{ .lomax =  93000000, .spd = 3, .bs = 2, .bp = 0, .cp = 0, .gc3 = 1, .div1p5 = 0},
	{ .lomax =  98000000, .spd = 3, .bs = 3, .bp = 0, .cp = 0, .gc3 = 1, .div1p5 = 0},
	{ .lomax = 109000000, .spd = 3, .bs = 3, .bp = 1, .cp = 0, .gc3 = 1, .div1p5 = 0},
	{ .lomax = 123000000, .spd = 2, .bs = 2, .bp = 1, .cp = 0, .gc3 = 1, .div1p5 = 1},
	{ .lomax = 133000000, .spd = 2, .bs = 3, .bp = 1, .cp = 0, .gc3 = 1, .div1p5 = 1},
	{ .lomax = 151000000, .spd = 2, .bs = 1, .bp = 1, .cp = 0, .gc3 = 1, .div1p5 = 0},
	{ .lomax = 154000000, .spd = 2, .bs = 2, .bp = 1, .cp = 0, .gc3 = 1, .div1p5 = 0},
	{ .lomax = 181000000, .spd = 2, .bs = 2, .bp = 1, .cp = 0, .gc3 = 0, .div1p5 = 0},
	{ .lomax = 185000000, .spd = 2, .bs = 2, .bp = 2, .cp = 0, .gc3 = 1, .div1p5 = 0},
	{ .lomax = 217000000, .spd = 2, .bs = 3, .bp = 2, .cp = 0, .gc3 = 1, .div1p5 = 0},
	{ .lomax = 244000000, .spd = 1, .bs = 2, .bp = 2, .cp = 0, .gc3 = 1, .div1p5 = 1},
	{ .lomax = 265000000, .spd = 1, .bs = 3, .bp = 2, .cp = 0, .gc3 = 1, .div1p5 = 1},
	{ .lomax = 302000000, .spd = 1, .bs = 1, .bp = 2, .cp = 0, .gc3 = 1, .div1p5 = 0},
	{ .lomax = 324000000, .spd = 1, .bs = 2, .bp = 2, .cp = 0, .gc3 = 1, .div1p5 = 0},
	{ .lomax = 370000000, .spd = 1, .bs = 2, .bp = 3, .cp = 0, .gc3 = 1, .div1p5 = 0},
	{ .lomax = 454000000, .spd = 1, .bs = 3, .bp = 3, .cp = 0, .gc3 = 1, .div1p5 = 0},
	{ .lomax = 493000000, .spd = 0, .bs = 2, .bp = 3, .cp = 0, .gc3 = 1, .div1p5 = 1},
	{ .lomax = 530000000, .spd = 0, .bs = 3, .bp = 3, .cp = 0, .gc3 = 1, .div1p5 = 1},
	{ .lomax = 554000000, .spd = 0, .bs = 1, .bp = 3, .cp = 0, .gc3 = 1, .div1p5 = 0},
	{ .lomax = 604000000, .spd = 0, .bs = 1, .bp = 4, .cp = 0, .gc3 = 0, .div1p5 = 0},
	{ .lomax = 696000000, .spd = 0, .bs = 2, .bp = 4, .cp = 0, .gc3 = 0, .div1p5 = 0},
	{ .lomax = 740000000, .spd = 0, .bs = 2, .bp = 4, .cp = 1, .gc3 = 0, .div1p5 = 0},
	{ .lomax = 820000000, .spd = 0, .bs = 3, .bp = 4, .cp = 0, .gc3 = 0, .div1p5 = 0},
	{ .lomax = 865000000, .spd = 0, .bs = 3, .bp = 4, .cp = 1, .gc3 = 0, .div1p5 = 0},
	{ .lomax =         0, .spd = 0, .bs = 0, .bp = 0, .cp = 0, .gc3 = 0, .div1p5 = 0}
};

static int philips_tda827x_pll_init(struct dvb_frontend *fe)
{
	return 0;
}

static int philips_tda827x_pll_set(struct dvb_frontend *fe, struct dvb_frontend_parameters *params)
{
	struct saa7134_dev *dev = fe->dvb->priv;
	u8 tuner_buf[14];

	struct i2c_msg tuner_msg = {.addr = 0x60,.flags = 0,.buf = tuner_buf,
		                        .len = sizeof(tuner_buf) };
	int i, tuner_freq, if_freq;
	u32 N;
	switch (params->u.ofdm.bandwidth) {
	case BANDWIDTH_6_MHZ:
		if_freq = 4000000;
		break;
	case BANDWIDTH_7_MHZ:
		if_freq = 4500000;
		break;
	default:		   /* 8 MHz or Auto */
		if_freq = 5000000;
		break;
	}
	tuner_freq = params->frequency + if_freq;

	i = 0;
	while (tda827x_dvbt[i].lomax < tuner_freq) {
		if(tda827x_dvbt[i + 1].lomax == 0)
			break;
		i++;
	}

	N = ((tuner_freq + 125000) / 250000) << (tda827x_dvbt[i].spd + 2);
	tuner_buf[0] = 0;
	tuner_buf[1] = (N>>8) | 0x40;
	tuner_buf[2] = N & 0xff;
	tuner_buf[3] = 0;
	tuner_buf[4] = 0x52;
	tuner_buf[5] = (tda827x_dvbt[i].spd << 6) + (tda827x_dvbt[i].div1p5 << 5) +
				   (tda827x_dvbt[i].bs << 3) + tda827x_dvbt[i].bp;
	tuner_buf[6] = (tda827x_dvbt[i].gc3 << 4) + 0x8f;
	tuner_buf[7] = 0xbf;
	tuner_buf[8] = 0x2a;
	tuner_buf[9] = 0x05;
	tuner_buf[10] = 0xff;
	tuner_buf[11] = 0x00;
	tuner_buf[12] = 0x00;
	tuner_buf[13] = 0x40;

	tuner_msg.len = 14;
	if (i2c_transfer(&dev->i2c_adap, &tuner_msg, 1) != 1)
		return -EIO;

	msleep(500);
	/* correct CP value */
	tuner_buf[0] = 0x30;
	tuner_buf[1] = 0x50 + tda827x_dvbt[i].cp;
	tuner_msg.len = 2;
	i2c_transfer(&dev->i2c_adap, &tuner_msg, 1);

	return 0;
}

static void philips_tda827x_pll_sleep(struct dvb_frontend *fe)
{
	struct saa7134_dev *dev = fe->dvb->priv;
	static u8 tda827x_sleep[] = { 0x30, 0xd0};
	struct i2c_msg tuner_msg = {.addr = 0x60,.flags = 0,.buf = tda827x_sleep,
	                            .len = sizeof(tda827x_sleep) };
	i2c_transfer(&dev->i2c_adap, &tuner_msg, 1);
}

static struct tda1004x_config tda827x_lifeview_config = {
	.demod_address = 0x08,
	.invert        = 1,
	.invert_oclk   = 0,
	.xtal_freq     = TDA10046_XTAL_16M,
	.agc_config    = TDA10046_AGC_TDA827X,
	.if_freq       = TDA10046_FREQ_045,
	.pll_init      = philips_tda827x_pll_init,
	.pll_set       = philips_tda827x_pll_set,
	.pll_sleep	   = philips_tda827x_pll_sleep,
	.request_firmware = NULL,
};
#endif

/* ------------------------------------------------------------------ */

static int dvb_init(struct saa7134_dev *dev)
{
	/* init struct videobuf_dvb */
	dev->ts.nr_bufs    = 32;
	dev->ts.nr_packets = 32*4;
	dev->dvb.name = dev->name;
	videobuf_queue_init(&dev->dvb.dvbq, &saa7134_ts_qops,
			    dev->pci, &dev->slock,
			    V4L2_BUF_TYPE_VIDEO_CAPTURE,
			    V4L2_FIELD_ALTERNATE,
			    sizeof(struct saa7134_buf),
			    dev);

	switch (dev->board) {
#ifdef HAVE_MT352
	case SAA7134_BOARD_PINNACLE_300I_DVBT_PAL:
		printk("%s: pinnacle 300i dvb setup\n",dev->name);
		dev->dvb.frontend = mt352_attach(&pinnacle_300i,
						 &dev->i2c_adap);
		break;
#endif
#ifdef HAVE_TDA1004X
	case SAA7134_BOARD_MD7134:
		dev->dvb.frontend = tda10046_attach(&medion_cardbus,
						    &dev->i2c_adap);
		break;
	case SAA7134_BOARD_PHILIPS_TOUGH:
		dev->dvb.frontend = tda10046_attach(&philips_tu1216_config,
						    &dev->i2c_adap);
		break;
	case SAA7134_BOARD_FLYDVBTDUO:
		dev->dvb.frontend = tda10046_attach(&tda827x_lifeview_config,
						    &dev->i2c_adap);
		break;
	case SAA7134_BOARD_THYPHOON_DVBT_DUO_CARDBUS:
		dev->dvb.frontend = tda10046_attach(&tda827x_lifeview_config,
						    &dev->i2c_adap);
		break;
#endif
	default:
		printk("%s: Huh? unknown DVB card?\n",dev->name);
		break;
	}

	if (NULL == dev->dvb.frontend) {
		printk("%s: frontend initialization failed\n",dev->name);
		return -1;
	}

	/* register everything else */
	return videobuf_dvb_register(&dev->dvb, THIS_MODULE, dev);
}

static int dvb_fini(struct saa7134_dev *dev)
{
	static int on  = TDA9887_PRESENT | TDA9887_PORT2_INACTIVE;

	switch (dev->board) {
	case SAA7134_BOARD_PINNACLE_300I_DVBT_PAL:
		/* otherwise we don't detect the tuner on next insmod */
		saa7134_i2c_call_clients(dev,TDA9887_SET_CONFIG,&on);
		break;
	};
	videobuf_dvb_unregister(&dev->dvb);
	return 0;
}

static struct saa7134_mpeg_ops dvb_ops = {
	.type          = SAA7134_MPEG_DVB,
	.init          = dvb_init,
	.fini          = dvb_fini,
};

static int __init dvb_register(void)
{
	return saa7134_ts_register(&dvb_ops);
}

static void __exit dvb_unregister(void)
{
	saa7134_ts_unregister(&dvb_ops);
}

module_init(dvb_register);
module_exit(dvb_unregister);

/* ------------------------------------------------------------------ */
/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
