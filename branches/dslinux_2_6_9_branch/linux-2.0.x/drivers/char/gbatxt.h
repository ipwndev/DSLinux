/****************************************************************************/

/*
 *	gbatxt.h -- definitions to support gbatxt driver
 *
 *	(C) Copyright 2001, Greg Ungerer (gerg@lineo.com)
 */

/****************************************************************************/
#ifndef _GBATXT_SERIAL_H
#define _GBATXT_SERIAL_H
/****************************************************************************/

#include <linux/config.h>
#include <linux/serial.h>

/*
 *	Define a local stats structure.
 */
struct mcf_stats {
	unsigned int	rx;
	unsigned int	tx;
	unsigned int	rxbreak;
	unsigned int	rxframing;
	unsigned int	rxparity;
	unsigned int	rxoverrun;
};


/*
 *	This is our internal structure for console device.
 *	Make it look and work like a serial port...
 */
struct gbatxt_serial {
	int			magic;
	unsigned int		addr;
	int			irq;
	int			flags; 		/* defined in tty.h */
	int			type; 		/* UART type */
	struct tty_struct 	*tty;
	unsigned int		baud;
	int			sigs;
	int			custom_divisor;
	int			baud_base;
	int			close_delay;
	unsigned short		closing_wait;
	unsigned short		closing_wait2;
	unsigned long		event;
	int			line;
	int			count;	    /* # of fd on device */
	int			blocked_open; /* # of blocked opens */
	long			session; /* Session of opening process */
	long			pgrp; /* pgrp of opening process */
	unsigned char 		*xmit_buf;
	int			xmit_head;
	int			xmit_tail;
	int			xmit_cnt;
	struct mcf_stats	stats;
	struct tq_struct	tqueue;
	struct tq_struct	tqueue_hangup;
	struct termios		normal_termios;
	struct termios		callout_termios;
	struct wait_queue	*open_wait;
	struct wait_queue	*close_wait;
};

/****************************************************************************/
#endif /* _GBATXT_SERIAL_H */
