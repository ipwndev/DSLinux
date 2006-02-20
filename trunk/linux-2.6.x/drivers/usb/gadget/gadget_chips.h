/*
 * USB device controllers have lots of quirks.  Use these macros in
 * gadget drivers or other code that needs to deal with them, and which
 * autoconfigures instead of using early binding to the hardware.
 *
 * This could eventually work like the ARM mach_is_*() stuff, driven by
 * some config file that gets updated as new hardware is supported.
 * (And avoiding the runtime comparisons in typical one-choice cases.)
 *
 * NOTE:  some of these controller drivers may not be available yet.
 */
#ifdef CONFIG_USB_GADGET_NET2280
#define	gadget_is_net2280(g)	!strcmp("net2280", (g)->name)
#else
#define	gadget_is_net2280(g)	0
#endif

#ifdef CONFIG_USB_GADGET_DUMMY_HCD
#define	gadget_is_dummy(g)	!strcmp("dummy_udc", (g)->name)
#else
#define	gadget_is_dummy(g)	0
#endif

#ifdef CONFIG_USB_GADGET_PXA2XX
#define	gadget_is_pxa(g)	!strcmp("pxa2xx_udc", (g)->name)
#else
#define	gadget_is_pxa(g)	0
#endif

#ifdef CONFIG_USB_GADGET_GOKU
#define	gadget_is_goku(g)	!strcmp("goku_udc", (g)->name)
#else
#define	gadget_is_goku(g)	0
#endif

#ifdef CONFIG_USB_GADGET_SUPERH
#define	gadget_is_sh(g)		!strcmp("sh_udc", (g)->name)
#else
#define	gadget_is_sh(g)		0
#endif

#ifdef CONFIG_USB_GADGET_SA1100
#define	gadget_is_sa1100(g)	!strcmp("sa1100_udc", (g)->name)
#else
#define	gadget_is_sa1100(g)	0
#endif

#ifdef CONFIG_USB_GADGET_LH7A40X
#define	gadget_is_lh7a40x(g)	!strcmp("lh7a40x_udc", (g)->name)
#else
#define	gadget_is_lh7a40x(g)	0
#endif

#ifdef CONFIG_USB_GADGET_MQ11XX
#define	gadget_is_mq11xx(g)	!strcmp("mq11xx_udc", (g)->name)
#else
#define	gadget_is_mq11xx(g)	0
#endif

#ifdef CONFIG_USB_GADGET_OMAP
#define	gadget_is_omap(g)	!strcmp("omap_udc", (g)->name)
#else
#define	gadget_is_omap(g)	0
#endif

#ifdef CONFIG_USB_GADGET_N9604
#define	gadget_is_n9604(g)	!strcmp("n9604_udc", (g)->name)
#else
#define	gadget_is_n9604(g)	0
#endif

#ifdef CONFIG_USB_GADGET_PXA27X
#define	gadget_is_pxa27x(g)	!strcmp("pxa27x_udc", (g)->name)
#else
#define	gadget_is_pxa27x(g)	0
#endif

#ifdef CONFIG_USB_GADGET_S3C2410
#define gadget_is_s3c2410(g)    !strcmp("s3c2410_udc", (g)->name)
#else
#define gadget_is_s3c2410(g)    0
#endif

#ifdef CONFIG_USB_GADGET_AT91
#define gadget_is_at91(g)	!strcmp("at91_udc", (g)->name)
#else
#define gadget_is_at91(g)	0
#endif

#ifdef CONFIG_USB_GADGET_IMX
#define gadget_is_imx(g)	!strcmp("imx_udc", (g)->name)
#else
#define gadget_is_imx(g)	0
#endif

// CONFIG_USB_GADGET_SX2
// CONFIG_USB_GADGET_AU1X00
// ...


/**
 * usb_gadget_controller_number - support bcdDevice id convention
 * @gadget: the controller being driven
 *
 * Return a 2-digit BCD value associated with the peripheral controller,
 * suitable for use as part of a bcdDevice value, or a negative error code.
 *
 * NOTE:  this convention is purely optional, and has no meaning in terms of
 * any USB specification.  If you want to use a different convention in your
 * gadget driver firmware -- maybe a more formal revision ID -- feel free.
 *
 * Hosts see these bcdDevice numbers, and are allowed (but not encouraged!)
 * to change their behavior accordingly.  For example it might help avoiding
 * some chip bug.
 */
static inline int usb_gadget_controller_number(struct usb_gadget *gadget)
{
	if (gadget_is_net2280(gadget))
		return 0x01;
	else if (gadget_is_dummy(gadget))
		return 0x02;
	else if (gadget_is_pxa(gadget))
		return 0x03;
	else if (gadget_is_sh(gadget))
		return 0x04;
	else if (gadget_is_sa1100(gadget))
		return 0x05;
	else if (gadget_is_goku(gadget))
		return 0x06;
	else if (gadget_is_mq11xx(gadget))
		return 0x07;
	else if (gadget_is_omap(gadget))
		return 0x08;
	else if (gadget_is_lh7a40x(gadget))
		return 0x09;
	else if (gadget_is_n9604(gadget))
		return 0x10;
	else if (gadget_is_pxa27x(gadget))
		return 0x11;
	else if (gadget_is_s3c2410(gadget))
		return 0x12;
	else if (gadget_is_at91(gadget))
		return 0x13;
	else if (gadget_is_imx(gadget))
		return 0x14;
	return -ENOENT;
}
