/*
 *
 * device driver for Conexant 2388x based TV cards
 * MPEG Transport Stream (DVB) routines
 *
 * (c) 2004 Chris Pascoe <c.pascoe@itee.uq.edu.au>
 * (c) 2004 Gerd Knorr <kraxel@bytesex.org> [SuSE Labs]
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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kthread.h>
#include <linux/file.h>
#include <linux/suspend.h>


#include "cx88.h"
#include "dvb-pll.h"

#ifdef HAVE_MT352
# include "mt352.h"
# include "mt352_priv.h"
#endif
#ifdef HAVE_CX22702
# include "cx22702.h"
#endif
#ifdef HAVE_OR51132
# include "or51132.h"
#endif
#ifdef HAVE_LGDT330X
# include "lgdt330x.h"
#endif

MODULE_DESCRIPTION("driver for cx2388x based DVB cards");
MODULE_AUTHOR("Chris Pascoe <c.pascoe@itee.uq.edu.au>");
MODULE_AUTHOR("Gerd Knorr <kraxel@bytesex.org> [SuSE Labs]");
MODULE_LICENSE("GPL");

static unsigned int debug = 0;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug,"enable debug messages [dvb]");

#define dprintk(level,fmt, arg...)	if (debug >= level) \
	printk(KERN_DEBUG "%s/2-dvb: " fmt, dev->core->name , ## arg)

/* ------------------------------------------------------------------ */

static int dvb_buf_setup(struct videobuf_queue *q,
			 unsigned int *count, unsigned int *size)
{
	struct cx8802_dev *dev = q->priv_data;

	dev->ts_packet_size  = 188 * 4;
	dev->ts_packet_count = 32;

	*size  = dev->ts_packet_size * dev->ts_packet_count;
	*count = 32;
	return 0;
}

static int dvb_buf_prepare(struct videobuf_queue *q, struct videobuf_buffer *vb,
			   enum v4l2_field field)
{
	struct cx8802_dev *dev = q->priv_data;
	return cx8802_buf_prepare(dev, (struct cx88_buffer*)vb);
}

static void dvb_buf_queue(struct videobuf_queue *q, struct videobuf_buffer *vb)
{
	struct cx8802_dev *dev = q->priv_data;
	cx8802_buf_queue(dev, (struct cx88_buffer*)vb);
}

static void dvb_buf_release(struct videobuf_queue *q, struct videobuf_buffer *vb)
{
	struct cx8802_dev *dev = q->priv_data;
	cx88_free_buffer(dev->pci, (struct cx88_buffer*)vb);
}

static struct videobuf_queue_ops dvb_qops = {
	.buf_setup    = dvb_buf_setup,
	.buf_prepare  = dvb_buf_prepare,
	.buf_queue    = dvb_buf_queue,
	.buf_release  = dvb_buf_release,
};

/* ------------------------------------------------------------------ */

#ifdef HAVE_MT352
static int dvico_fusionhdtv_demod_init(struct dvb_frontend* fe)
{
	static u8 clock_config []  = { CLOCK_CTL,  0x38, 0x39 };
	static u8 reset []         = { RESET,      0x80 };
	static u8 adc_ctl_1_cfg [] = { ADC_CTL_1,  0x40 };
	static u8 agc_cfg []       = { AGC_TARGET, 0x24, 0x20 };
	static u8 gpp_ctl_cfg []   = { GPP_CTL,    0x33 };
	static u8 capt_range_cfg[] = { CAPT_RANGE, 0x32 };

	mt352_write(fe, clock_config,   sizeof(clock_config));
	udelay(200);
	mt352_write(fe, reset,          sizeof(reset));
	mt352_write(fe, adc_ctl_1_cfg,  sizeof(adc_ctl_1_cfg));

	mt352_write(fe, agc_cfg,        sizeof(agc_cfg));
	mt352_write(fe, gpp_ctl_cfg,    sizeof(gpp_ctl_cfg));
	mt352_write(fe, capt_range_cfg, sizeof(capt_range_cfg));
	return 0;
}

static int dntv_live_dvbt_demod_init(struct dvb_frontend* fe)
{
	static u8 clock_config []  = { 0x89, 0x38, 0x39 };
	static u8 reset []         = { 0x50, 0x80 };
	static u8 adc_ctl_1_cfg [] = { 0x8E, 0x40 };
	static u8 agc_cfg []       = { 0x67, 0x10, 0x23, 0x00, 0xFF, 0xFF,
	                               0x00, 0xFF, 0x00, 0x40, 0x40 };
	static u8 dntv_extra[]     = { 0xB5, 0x7A };
	static u8 capt_range_cfg[] = { 0x75, 0x32 };

	mt352_write(fe, clock_config,   sizeof(clock_config));
	udelay(2000);
	mt352_write(fe, reset,          sizeof(reset));
	mt352_write(fe, adc_ctl_1_cfg,  sizeof(adc_ctl_1_cfg));

	mt352_write(fe, agc_cfg,        sizeof(agc_cfg));
	udelay(2000);
	mt352_write(fe, dntv_extra,     sizeof(dntv_extra));
	mt352_write(fe, capt_range_cfg, sizeof(capt_range_cfg));

	return 0;
}

static int mt352_pll_set(struct dvb_frontend* fe,
			 struct dvb_frontend_parameters* params,
			 u8* pllbuf)
{
	struct cx8802_dev *dev= fe->dvb->priv;

	pllbuf[0] = dev->core->pll_addr << 1;
	dvb_pll_configure(dev->core->pll_desc, pllbuf+1,
			  params->frequency,
			  params->u.ofdm.bandwidth);
	return 0;
}

static struct mt352_config dvico_fusionhdtv = {
	.demod_address = 0x0F,
	.demod_init    = dvico_fusionhdtv_demod_init,
	.pll_set       = mt352_pll_set,
};

static struct mt352_config dntv_live_dvbt_config = {
	.demod_address = 0x0f,
	.demod_init    = dntv_live_dvbt_demod_init,
	.pll_set       = mt352_pll_set,
};
#endif

#ifdef HAVE_CX22702
static struct cx22702_config connexant_refboard_config = {
	.demod_address = 0x43,
	.output_mode   = CX22702_SERIAL_OUTPUT,
	.pll_address   = 0x60,
	.pll_desc      = &dvb_pll_thomson_dtt7579,
};

static struct cx22702_config hauppauge_novat_config = {
	.demod_address = 0x43,
	.output_mode   = CX22702_SERIAL_OUTPUT,
	.pll_address   = 0x61,
	.pll_desc      = &dvb_pll_thomson_dtt759x,
};
#endif

#ifdef HAVE_OR51132
static int or51132_set_ts_param(struct dvb_frontend* fe,
				int is_punctured)
{
	struct cx8802_dev *dev= fe->dvb->priv;
	dev->ts_gen_cntrl = is_punctured ? 0x04 : 0x00;
	return 0;
}

static struct or51132_config pchdtv_hd3000 = {
	.demod_address    = 0x15,
	.pll_address      = 0x61,
	.pll_desc         = &dvb_pll_thomson_dtt7610,
	.set_ts_params    = or51132_set_ts_param,
};
#endif

#ifdef HAVE_LGDT330X
static int lgdt330x_pll_set(struct dvb_frontend* fe,
			    struct dvb_frontend_parameters* params)
{
	/* FIXME make this routine use the tuner-simple code.
	 * It could probably be shared with a number of ATSC
	 * frontends. Many share the same tuner with analog TV. */

	struct cx8802_dev *dev= fe->dvb->priv;
	struct cx88_core *core = dev->core;
	u8 buf[4];
	struct i2c_msg msg =
		{ .addr = dev->core->pll_addr, .flags = 0, .buf = buf, .len = 4 };
	int err;

	/* Put the analog decoder in standby to keep it quiet */
	cx88_call_i2c_clients (dev->core, TUNER_SET_STANDBY, NULL);

	dvb_pll_configure(core->pll_desc, buf, params->frequency, 0);
	dprintk(1, "%s: tuner at 0x%02x bytes: 0x%02x 0x%02x 0x%02x 0x%02x\n",
			__FUNCTION__, msg.addr, buf[0],buf[1],buf[2],buf[3]);
	if ((err = i2c_transfer(&core->i2c_adap, &msg, 1)) != 1) {
		printk(KERN_WARNING "cx88-dvb: %s error "
			   "(addr %02x <- %02x, err = %i)\n",
			   __FUNCTION__, buf[0], buf[1], err);
		if (err < 0)
			return err;
		else
			return -EREMOTEIO;
	}
	if (core->tuner_type == TUNER_LG_TDVS_H062F) {
		/* Set the Auxiliary Byte. */
		buf[2] &= ~0x20;
		buf[2] |= 0x18;
		buf[3] = 0x50;
		i2c_transfer(&core->i2c_adap, &msg, 1);
	}
	return 0;
}

static int lgdt330x_pll_rf_set(struct dvb_frontend* fe, int index)
{
	struct cx8802_dev *dev= fe->dvb->priv;
	struct cx88_core *core = dev->core;

	dprintk(1, "%s: index = %d\n", __FUNCTION__, index);
	if (index == 0)
		cx_clear(MO_GP0_IO, 8);
	else
		cx_set(MO_GP0_IO, 8);
	return 0;
}

static int lgdt330x_set_ts_param(struct dvb_frontend* fe, int is_punctured)
{
	struct cx8802_dev *dev= fe->dvb->priv;
	if (is_punctured)
		dev->ts_gen_cntrl |= 0x04;
	else
		dev->ts_gen_cntrl &= ~0x04;
	return 0;
}

static struct lgdt330x_config fusionhdtv_3_gold = {
	.demod_address    = 0x0e,
	.demod_chip       = LGDT3302,
	.serial_mpeg      = 0x04, /* TPSERIAL for 3302 in TOP_CONTROL */
	.pll_set          = lgdt330x_pll_set,
	.set_ts_params    = lgdt330x_set_ts_param,
};

static struct lgdt330x_config fusionhdtv_5_gold = {
	.demod_address    = 0x0e,
	.demod_chip       = LGDT3303,
	.serial_mpeg      = 0x40, /* TPSERIAL for 3303 in TOP_CONTROL */
	.pll_set          = lgdt330x_pll_set,
	.set_ts_params    = lgdt330x_set_ts_param,
};
#endif

static int dvb_register(struct cx8802_dev *dev)
{
	/* init struct videobuf_dvb */
	dev->dvb.name = dev->core->name;
	dev->ts_gen_cntrl = 0x0c;

	/* init frontend */
	switch (dev->core->board) {
#ifdef HAVE_CX22702
	case CX88_BOARD_HAUPPAUGE_DVB_T1:
		dev->dvb.frontend = cx22702_attach(&hauppauge_novat_config,
						   &dev->core->i2c_adap);
		break;
	case CX88_BOARD_TERRATEC_CINERGY_1400_DVB_T1:
	case CX88_BOARD_CONEXANT_DVB_T1:
		dev->dvb.frontend = cx22702_attach(&connexant_refboard_config,
						   &dev->core->i2c_adap);
		break;
#endif
#ifdef HAVE_MT352
	case CX88_BOARD_DVICO_FUSIONHDTV_DVB_T1:
		dev->core->pll_addr = 0x61;
		dev->core->pll_desc = &dvb_pll_lg_z201;
		dev->dvb.frontend = mt352_attach(&dvico_fusionhdtv,
						 &dev->core->i2c_adap);
		break;
	case CX88_BOARD_DVICO_FUSIONHDTV_DVB_T_PLUS:
		dev->core->pll_addr = 0x60;
		dev->core->pll_desc = &dvb_pll_thomson_dtt7579;
		dev->dvb.frontend = mt352_attach(&dvico_fusionhdtv,
						 &dev->core->i2c_adap);
		break;
	case CX88_BOARD_KWORLD_DVB_T:
	case CX88_BOARD_DNTV_LIVE_DVB_T:
	case CX88_BOARD_ADSTECH_DVB_T_PCI:
		dev->core->pll_addr = 0x61;
		dev->core->pll_desc = &dvb_pll_unknown_1;
		dev->dvb.frontend = mt352_attach(&dntv_live_dvbt_config,
						 &dev->core->i2c_adap);
		break;
#endif
#ifdef HAVE_OR51132
	case CX88_BOARD_PCHDTV_HD3000:
		dev->dvb.frontend = or51132_attach(&pchdtv_hd3000,
						 &dev->core->i2c_adap);
		break;
#endif
#ifdef HAVE_LGDT330X
	case CX88_BOARD_DVICO_FUSIONHDTV_3_GOLD_Q:
		dev->ts_gen_cntrl = 0x08;
		{
		/* Do a hardware reset of chip before using it. */
		struct cx88_core *core = dev->core;

		cx_clear(MO_GP0_IO, 1);
		mdelay(100);
		cx_set(MO_GP0_IO, 1);
		mdelay(200);

		/* Select RF connector callback */
		fusionhdtv_3_gold.pll_rf_set = lgdt330x_pll_rf_set;
		dev->core->pll_addr = 0x61;
		dev->core->pll_desc = &dvb_pll_microtune_4042;
		dev->dvb.frontend = lgdt330x_attach(&fusionhdtv_3_gold,
						    &dev->core->i2c_adap);
		}
		break;
	case CX88_BOARD_DVICO_FUSIONHDTV_3_GOLD_T:
		dev->ts_gen_cntrl = 0x08;
		{
		/* Do a hardware reset of chip before using it. */
		struct cx88_core *core = dev->core;

		cx_clear(MO_GP0_IO, 1);
		mdelay(100);
		cx_set(MO_GP0_IO, 9);
		mdelay(200);
		dev->core->pll_addr = 0x61;
		dev->core->pll_desc = &dvb_pll_thomson_dtt7611;
		dev->dvb.frontend = lgdt330x_attach(&fusionhdtv_3_gold,
						    &dev->core->i2c_adap);
		}
		break;
	case CX88_BOARD_DVICO_FUSIONHDTV_5_GOLD:
		dev->ts_gen_cntrl = 0x08;
		{
		/* Do a hardware reset of chip before using it. */
		struct cx88_core *core = dev->core;

		cx_clear(MO_GP0_IO, 1);
		mdelay(100);
		cx_set(MO_GP0_IO, 1);
		mdelay(200);
		dev->core->pll_addr = 0x61;
		dev->core->pll_desc = &dvb_pll_tdvs_tua6034;
		dev->dvb.frontend = lgdt330x_attach(&fusionhdtv_5_gold,
						    &dev->core->i2c_adap);
		}
		break;
#endif
	default:
		printk("%s: The frontend of your DVB/ATSC card isn't supported yet\n",
		       dev->core->name);
		break;
	}
	if (NULL == dev->dvb.frontend) {
		printk("%s: frontend initialization failed\n",dev->core->name);
		return -1;
	}

	if (dev->core->pll_desc) {
		dev->dvb.frontend->ops->info.frequency_min = dev->core->pll_desc->min;
		dev->dvb.frontend->ops->info.frequency_max = dev->core->pll_desc->max;
	}

	/* Put the analog decoder in standby to keep it quiet */
	cx88_call_i2c_clients (dev->core, TUNER_SET_STANDBY, NULL);

	/* register everything */
	return videobuf_dvb_register(&dev->dvb, THIS_MODULE, dev);
}

/* ----------------------------------------------------------- */

static int __devinit dvb_probe(struct pci_dev *pci_dev,
			       const struct pci_device_id *pci_id)
{
	struct cx8802_dev *dev;
	struct cx88_core  *core;
	int err;

	/* general setup */
	core = cx88_core_get(pci_dev);
	if (NULL == core)
		return -EINVAL;

	err = -ENODEV;
	if (!cx88_boards[core->board].dvb)
		goto fail_core;

	err = -ENOMEM;
	dev = kmalloc(sizeof(*dev),GFP_KERNEL);
	if (NULL == dev)
		goto fail_core;
	memset(dev,0,sizeof(*dev));
	dev->pci = pci_dev;
	dev->core = core;

	err = cx8802_init_common(dev);
	if (0 != err)
		goto fail_free;

	/* dvb stuff */
	printk("%s/2: cx2388x based dvb card\n", core->name);
	videobuf_queue_init(&dev->dvb.dvbq, &dvb_qops,
			    dev->pci, &dev->slock,
			    V4L2_BUF_TYPE_VIDEO_CAPTURE,
			    V4L2_FIELD_TOP,
			    sizeof(struct cx88_buffer),
			    dev);
	err = dvb_register(dev);
	if (0 != err)
		goto fail_fini;
	return 0;

 fail_fini:
	cx8802_fini_common(dev);
 fail_free:
	kfree(dev);
 fail_core:
	cx88_core_put(core,pci_dev);
	return err;
}

static void __devexit dvb_remove(struct pci_dev *pci_dev)
{
        struct cx8802_dev *dev = pci_get_drvdata(pci_dev);

	/* dvb */
	videobuf_dvb_unregister(&dev->dvb);

	/* common */
	cx8802_fini_common(dev);
	cx88_core_put(dev->core,dev->pci);
	kfree(dev);
}

static struct pci_device_id cx8802_pci_tbl[] = {
	{
		.vendor       = 0x14f1,
		.device       = 0x8802,
                .subvendor    = PCI_ANY_ID,
                .subdevice    = PCI_ANY_ID,
	},{
		/* --- end of list --- */
	}
};
MODULE_DEVICE_TABLE(pci, cx8802_pci_tbl);

static struct pci_driver dvb_pci_driver = {
        .name     = "cx88-dvb",
        .id_table = cx8802_pci_tbl,
        .probe    = dvb_probe,
        .remove   = __devexit_p(dvb_remove),
	.suspend  = cx8802_suspend_common,
	.resume   = cx8802_resume_common,
};

static int dvb_init(void)
{
	printk(KERN_INFO "cx2388x dvb driver version %d.%d.%d loaded\n",
	       (CX88_VERSION_CODE >> 16) & 0xff,
	       (CX88_VERSION_CODE >>  8) & 0xff,
	       CX88_VERSION_CODE & 0xff);
#ifdef SNAPSHOT
	printk(KERN_INFO "cx2388x: snapshot date %04d-%02d-%02d\n",
	       SNAPSHOT/10000, (SNAPSHOT/100)%100, SNAPSHOT%100);
#endif
	return pci_register_driver(&dvb_pci_driver);
}

static void dvb_fini(void)
{
	pci_unregister_driver(&dvb_pci_driver);
}

module_init(dvb_init);
module_exit(dvb_fini);

/*
 * Local variables:
 * c-basic-offset: 8
 * compile-command: "make DVB=1"
 * End:
 */
