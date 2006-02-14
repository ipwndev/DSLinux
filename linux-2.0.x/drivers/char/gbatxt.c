/****************************************************************************/

/*
 *	text.c -- text printing routines for GBA.
 *
 *	(C) Copyright 2001, Greg Ungerer (gerg@snapgear.com)
 *
 *
 *	Much code taken from:
 *	mcfserial.c -- serial driver for ColdFire internal UARTS.
 *
 *	Copyright (C) 1999 Greg Ungerer (gerg@snapgear.com)
 *	(C) Copyright 2000, Lineo Inc. (www.lineo.com) 
 *
 *	Based on code from 68332serial.c which was:
 *
 *	Copyright (C) 1995 David S. Miller (davem@caip.rutgers.edu)
 *	Copyright (C) 1998 TSHG
 */

/****************************************************************************/

#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/config.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/kernel.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/segment.h>
#include <asm/bitops.h>
#include <asm/delay.h>

#include "gbatxt.h"

/****************************************************************************/

char *gbatxt_drivername = "GBA text console, version 1.0\n";

/****************************************************************************/

#define	GBA_CTRLBASE	0x04000000		/* Register IO base */
#define	GBA_PALBASE	0x05000000		/* Pallete RAM base */
#define	GBA_VIDBASE	0x06000000		/* Video RAM base */

/****************************************************************************/

/*
 *	Screen dimensions. Need to allow for screen overshoot regions too.
 *	Also keep track of current cursor postion (x,y).
 */
#define	GBA_XMAX	32
#define	GBA_XLEN	30
#define	GBA_YMAX	32
#define	GBA_YLEN	20

int	gbatxt_x;
int	gbatxt_y;

/****************************************************************************/

int gbatxt_console_inited = 0;

DECLARE_TASK_QUEUE(gbatxt_tq_serial);

/*
 *	Local TTY driver data structures.
 */
struct tty_driver	gbatxt_serial_driver;
struct tty_driver	gbatxt_callout_driver;
static int		gbatxt_serial_refcount;

/* serial subtype definitions */
#define SERIAL_TYPE_NORMAL	1
#define SERIAL_TYPE_CALLOUT	2
  
/* number of characters left in xmit buffer before we ask for more */
#define WAKEUP_CHARS 256

/*
 *	Debugging...
 */
//#undef SERIAL_DEBUG_OPEN
//#undef SERIAL_DEBUG_FLOW
//#define SERIAL_DEBUG_OPEN	1
//#define SERIAL_DEBUG_FLOW	1

#define _INLINE_ inline

/*
 *	Configuration table, UARTs to look for at startup.
 */
static struct gbatxt_serial gbatxt_table[] = {
  { 0, 0, 0,   ASYNC_BOOT_AUTOCONF },  /* ttyS0 */
};


#define	NR_PORTS	(sizeof(gbatxt_table) / sizeof(struct gbatxt_serial))

static struct tty_struct	*gbatxt_serial_table[NR_PORTS];
static struct termios		*gbatxt_serial_termios[NR_PORTS];
static struct termios		*gbatxt_serial_termios_locked[NR_PORTS];

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#ifdef CONFIG_MAGIC_SYSRQ
/*
 *	Magic system request keys. Used for debugging...
 */
extern int	magic_sysrq_key(int ch);
#endif

/****************************************************************************/

void gbatxt_console_print(char *s);
void gbatxt_console_putc(int c);

/****************************************************************************/

/*
 *	Faked, no signals to set on LCD screen :-)
 */

static void gbatxt_setsignals(struct gbatxt_serial *info, int dtr, int rts)
{
}

/****************************************************************************/

/*
 *	Fake serial port signals. Probably don't even need to bother...
 */

static int gbatxt_getsignals(struct gbatxt_serial *info)
{
	return(TIOCM_DTR | TIOCM_RTS);
}

/****************************************************************************/

static void gbatxt_stop(struct tty_struct *tty)
{
	/* Nothing needed, output is not buffered. */
}

/****************************************************************************/

static void gbatxt_start(struct tty_struct *tty)
{
	/* Nothing needed, output is not buffered. */
}

/****************************************************************************/

/*
 *	Nothing really needed here now. Maintain for simplicity.
 */

static void gbatxt_change_speed(struct gbatxt_serial *info)
{
	unsigned int	cflag;

	if (!info->tty || !info->tty->termios)
		return;
	cflag = info->tty->termios->c_cflag;

	if (cflag & CLOCAL)
		info->flags &= ~ASYNC_CHECK_CD;
	else
		info->flags |= ASYNC_CHECK_CD;

	gbatxt_setsignals(info, 1, -1);
	return;
}

/****************************************************************************/

#if 0

/*
 * This routine is used by the interrupt handler to schedule
 * processing in the software interrupt portion of the driver.
 */
static _INLINE_ void gbatxt_sched_event(struct gbatxt_serial *info, int event)
{
	info->event |= 1 << event;
	queue_task_irq_off(&info->tqueue, &mcf_tq_serial);
	mark_bh(CM206_BH);
}

static _INLINE_ void receive_chars(struct gbatxt_serial *info, struct pt_regs *regs, unsigned short rx)
{
	volatile unsigned char	*uartp;
	struct tty_struct	*tty = info->tty;
	unsigned char		status, ch;

	if (!tty)
		return;

#if defined(CONFIG_LEDMAN)
	ledman_cmd(LEDMAN_CMD_SET, info->line ? LEDMAN_COM2_RX : LEDMAN_COM1_RX);
#endif

	uartp = (volatile unsigned char *) info->addr;

	while ((status = uartp[MCFUART_USR]) & MCFUART_USR_RXREADY) {

		if (tty->flip.count >= TTY_FLIPBUF_SIZE)
			break;

		ch = uartp[MCFUART_URB];
		info->stats.rx++;

#ifdef CONFIG_MAGIC_SYSRQ
		if (gbatxt_console_inited && (info->line == gbatxt_console_port)) {
			if (magic_sysrq_key(ch))
				continue;
		}
#endif

		tty->flip.count++;
		if (status & MCFUART_USR_RXERR)
			uartp[MCFUART_UCR] = MCFUART_UCR_CMDRESETERR;
		if (status & MCFUART_USR_RXBREAK) {
			info->stats.rxbreak++;
			*tty->flip.flag_buf_ptr++ = TTY_BREAK;
		} else if (status & MCFUART_USR_RXPARITY) {
			info->stats.rxparity++;
			*tty->flip.flag_buf_ptr++ = TTY_PARITY;
		} else if (status & MCFUART_USR_RXOVERRUN) {
			info->stats.rxoverrun++;
			*tty->flip.flag_buf_ptr++ = TTY_OVERRUN;
		} else if (status & MCFUART_USR_RXFRAMING) {
			info->stats.rxframing++;
			*tty->flip.flag_buf_ptr++ = TTY_FRAME;
		} else {
			*tty->flip.flag_buf_ptr++ = 0;
		}
		*tty->flip.char_buf_ptr++ = ch;
	}

	queue_task_irq_off(&tty->flip.tqueue, &tq_timer);
	return;
}

static _INLINE_ void transmit_chars(struct gbatxt_serial *info)
{
	volatile unsigned char	*uartp;

#if defined(CONFIG_LEDMAN)
	ledman_cmd(LEDMAN_CMD_SET, info->line ? LEDMAN_COM2_TX : LEDMAN_COM1_TX);
#endif

	uartp = (volatile unsigned char *) info->addr;

	if ((info->xmit_cnt <= 0) || info->tty->stopped) {
		info->imr &= ~MCFUART_UIR_TXREADY;
		uartp[MCFUART_UIMR] = info->imr;
		return;
	}

	while (uartp[MCFUART_USR] & MCFUART_USR_TXREADY) {
		uartp[MCFUART_UTB] = info->xmit_buf[info->xmit_tail++];
		info->xmit_tail = info->xmit_tail & (SERIAL_XMIT_SIZE-1);
		info->stats.tx++;
		if (--info->xmit_cnt <= 0)
			break;
	}

	if (info->xmit_cnt < WAKEUP_CHARS)
		gbatxt_sched_event(info, RS_EVENT_WRITE_WAKEUP);
	return;
}

/*
 * This is the serial driver's generic interrupt routine
 */
void gbatxt_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	struct gbatxt_serial	*info;
	unsigned char		isr;

	info = &gbatxt_table[(irq - IRQBASE)];
	isr = (((volatile unsigned char *)info->addr)[MCFUART_UISR]) & info->imr;

	if (isr & MCFUART_UIR_RXREADY)
		receive_chars(info, regs, isr);
	if (isr & MCFUART_UIR_TXREADY)
		transmit_chars(info);
#if 0
	if (isr & MCFUART_UIR_DELTABREAK) {
		printk("%s(%d): delta break!\n", __FILE__, __LINE__);
		receive_chars(info, regs, isr);
	}
#endif

	return;
}

#endif

/****************************************************************************/

#if 0
/*
 * This routine is used to handle the "bottom half" processing for the
 * serial driver, known also the "software interrupt" processing.
 * This processing is done at the kernel interrupt level, after the
 * gbatxt_interrupt() has returned, BUT WITH INTERRUPTS TURNED ON.  This
 * is where time-consuming activities which can not be done in the
 * interrupt driver proper are done; the interrupt driver schedules
 * them using gbatxt_sched_event(), and they get done here.
 */
static void do_serial_bh(void)
{
	run_task_queue(&mcf_tq_serial);
}
#endif

/****************************************************************************/

static void do_softint(void *private_)
{
	struct gbatxt_serial	*info = (struct gbatxt_serial *) private_;
	struct tty_struct	*tty;
	
	tty = info->tty;
	if (!tty)
		return;

	if (clear_bit(RS_EVENT_WRITE_WAKEUP, &info->event)) {
		if ((tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
		    tty->ldisc.write_wakeup)
			(tty->ldisc.write_wakeup)(tty);
		wake_up_interruptible(&tty->write_wait);
	}
}

/****************************************************************************/

/*
 *	Change of state on a DCD line.
 */
void gbatxt_modem_change(struct gbatxt_serial *info, int dcd)
{
	if (info->count == 0)
		return;

	if (info->flags & ASYNC_CHECK_CD) {
		if (dcd) {
			wake_up_interruptible(&info->open_wait);
		} else if (!((info->flags & ASYNC_CALLOUT_ACTIVE) &&
		    (info->flags & ASYNC_CALLOUT_NOHUP))) {
			queue_task_irq_off(&info->tqueue_hangup, &tq_scheduler);
		}
	}
}


/****************************************************************************/

#if 0

unsigned short	gbatxt_ppstatus;

/*
 * This subroutine is called when the RS_TIMER goes off. It is used
 * to monitor the state of the DCD lines - since they have no edge
 * sensors and interrupt generators.
 */
static void gbatxt_timer(void)
{
	unsigned short	ppstatus, dcdval;
	int		i;

	ppstatus = *((volatile unsigned short *) (MCF_MBAR + MCFSIM_PADAT)) &
		(NETtel_DCD0 | NETtel_DCD1);

	if (ppstatus != gbatxt_ppstatus) {
		for (i = 0; (i < 2); i++) {
			dcdval = (i ? NETtel_DCD1 : NETtel_DCD0);
			if ((ppstatus & dcdval) != (gbatxt_ppstatus & dcdval)) {
				gbatxt_modem_change(&gbatxt_table[i],
					((ppstatus & dcdval) ? 0 : 1));
			}
		}
	}
	gbatxt_ppstatus = ppstatus;

	/* Re-arm timer */
	timer_table[RS_TIMER].expires = jiffies + HZ/25;
	timer_active |= 1 << RS_TIMER;
}

#endif

/****************************************************************************/

/*
 * This routine is called from the scheduler tqueue when the interrupt
 * routine has signalled that a hangup has occurred.  The path of
 * hangup processing is:
 *
 * 	serial interrupt routine -> (scheduler tqueue) ->
 * 	do_serial_hangup() -> tty->hangup() -> gbatxt_hangup()
 * 
 */
static void do_serial_hangup(void *private_)
{
	struct gbatxt_serial	*info = (struct gbatxt_serial *) private_;
	struct tty_struct	*tty;
	
	tty = info->tty;
	if (!tty)
		return;

	tty_hangup(tty);
}

/****************************************************************************/

static int startup(struct gbatxt_serial * info)
{
	unsigned long		flags;
	
	if (info->flags & ASYNC_INITIALIZED)
		return 0;

	if (!info->xmit_buf) {
		info->xmit_buf = (unsigned char *) get_free_page(GFP_KERNEL);
		if (!info->xmit_buf)
			return -ENOMEM;
	}

	save_flags(flags); cli();

#if 0
	/*
	 * Set up poll timer. It is used to check DCD status.
	 */
	if ((timer_active & (1 << RS_TIMER)) == 0) {
		timer_table[RS_TIMER].expires = jiffies + HZ/25;
		timer_active |= 1 << RS_TIMER;
	}
#endif

#ifdef SERIAL_DEBUG_OPEN
	printk("starting up ttyS%d (irq %d)...\n", info->line, info->irq);
#endif

	gbatxt_setsignals(info, 1, 1);

	if (info->tty)
		clear_bit(TTY_IO_ERROR, &info->tty->flags);
	info->xmit_cnt = info->xmit_head = info->xmit_tail = 0;

	/*
	 * and set the speed of the serial port
	 */
	gbatxt_change_speed(info);

	info->flags |= ASYNC_INITIALIZED;
	restore_flags(flags);
	return 0;
}

/****************************************************************************/

/*
 * This routine will shutdown a serial port; interrupts are disabled, and
 * DTR is dropped if the hangup on close termio flag is on.
 */

static void shutdown(struct gbatxt_serial * info)
{
	unsigned long		flags;

	if (!(info->flags & ASYNC_INITIALIZED))
		return;

#ifdef SERIAL_DEBUG_OPEN
	printk("Shutting down serial port %d (irq %d)....\n", info->line,
	       info->irq);
#endif
	
	save_flags(flags); cli(); /* Disable interrupts */

	if (!info->tty || (info->tty->termios->c_cflag & HUPCL))
		gbatxt_setsignals(info, 0, 0);

	if (info->xmit_buf) {
		free_page((unsigned long) info->xmit_buf);
		info->xmit_buf = 0;
	}

	if (info->tty)
		set_bit(TTY_IO_ERROR, &info->tty->flags);
	
	info->flags &= ~ASYNC_INITIALIZED;
	restore_flags(flags);
}

/****************************************************************************/

static void gbatxt_flush_chars(struct tty_struct *tty)
{
	struct gbatxt_serial	*info = (struct gbatxt_serial *)tty->driver_data;

	if (info->xmit_cnt <= 0 || tty->stopped || tty->hw_stopped ||
	    !info->xmit_buf)
		return;
}

/****************************************************************************/

static int gbatxt_write(struct tty_struct * tty, int from_user,
		    const unsigned char *buf, int count)
{
	struct gbatxt_serial	*info=(struct gbatxt_serial *)tty->driver_data;
	unsigned char		*bp;
	int			total;

#if 0
	printk("%s(%d): gbatxt_write(tty=%x,from_user=%d,buf=%x,count=%d)\n",
		__FILE__, __LINE__, tty, from_user, buf, count);
#endif

	if (!tty || !info)
		return 0;

	/*
	 *	This is really cheating, we should use the correct access
	 *	routines for getting at user space data. But on uClinux I can
	 *	get away with this...
	 */
	bp = (unsigned char *) buf;
	for (total = count; (count > 0); count--)
		gbatxt_console_putc(*bp++);

	info->xmit_head = 0;
	info->xmit_cnt = 0;
	return(total);
}

/****************************************************************************/

static int gbatxt_write_room(struct tty_struct *tty)
{
	struct gbatxt_serial *info = (struct gbatxt_serial *)tty->driver_data;
	int	ret;
				
	ret = SERIAL_XMIT_SIZE - info->xmit_cnt - 1;
	if (ret < 0)
		ret = 0;
	return ret;
}

/****************************************************************************/

static int gbatxt_chars_in_buffer(struct tty_struct *tty)
{
	struct gbatxt_serial *info = (struct gbatxt_serial *)tty->driver_data;
				
	return info->xmit_cnt;
}

/****************************************************************************/

static void gbatxt_flush_buffer(struct tty_struct *tty)
{
	struct gbatxt_serial	*info = (struct gbatxt_serial *)tty->driver_data;
	unsigned long		flags;
				
	save_flags(flags); cli();
	info->xmit_cnt = info->xmit_head = info->xmit_tail = 0;
	restore_flags(flags);

	wake_up_interruptible(&tty->write_wait);
	if ((tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
	    tty->ldisc.write_wakeup)
		(tty->ldisc.write_wakeup)(tty);
}

/****************************************************************************/

static void gbatxt_throttle(struct tty_struct * tty)
{
	/* No way to throttle input here. */
}

/****************************************************************************/

static void gbatxt_unthrottle(struct tty_struct * tty)
{
	/* No need to restart input that we can't stop :-) */
}

/****************************************************************************/

static int get_serial_info(struct gbatxt_serial * info,
			   struct serial_struct * retinfo)
{
	struct serial_struct tmp;
  
	if (!retinfo)
		return -EFAULT;
	memset(&tmp, 0, sizeof(tmp));
	tmp.type = info->type;
	tmp.line = info->line;
	tmp.port = info->addr;
	tmp.irq = info->irq;
	tmp.flags = info->flags;
	tmp.baud_base = info->baud_base;
	tmp.close_delay = info->close_delay;
	tmp.closing_wait = info->closing_wait;
	tmp.custom_divisor = info->custom_divisor;
	memcpy_tofs(retinfo,&tmp,sizeof(*retinfo));
	return 0;
}

/****************************************************************************/

static int set_serial_info(struct gbatxt_serial * info,
			   struct serial_struct * new_info)
{
	struct serial_struct new_serial;
	struct gbatxt_serial old_info;
	int 			retval = 0;

	if (!new_info)
		return -EFAULT;
	memcpy_fromfs(&new_serial,new_info,sizeof(new_serial));
	old_info = *info;

	if (!suser()) {
		if ((new_serial.baud_base != info->baud_base) ||
		    (new_serial.type != info->type) ||
		    (new_serial.close_delay != info->close_delay) ||
		    ((new_serial.flags & ~ASYNC_USR_MASK) !=
		     (info->flags & ~ASYNC_USR_MASK)))
			return -EPERM;
		info->flags = ((info->flags & ~ASYNC_USR_MASK) |
			       (new_serial.flags & ASYNC_USR_MASK));
		info->custom_divisor = new_serial.custom_divisor;
		goto check_and_exit;
	}

	if (info->count > 1)
		return -EBUSY;

	/*
	 * OK, past this point, all the error checking has been done.
	 * At this point, we start making changes.....
	 */

	info->baud_base = new_serial.baud_base;
	info->flags = ((info->flags & ~ASYNC_FLAGS) |
			(new_serial.flags & ASYNC_FLAGS));
	info->type = new_serial.type;
	info->close_delay = new_serial.close_delay;
	info->closing_wait = new_serial.closing_wait;

check_and_exit:
	retval = startup(info);
	return retval;
}

/****************************************************************************/

/*
 * get_lsr_info - get line status register info
 */

static int get_lsr_info(struct gbatxt_serial * info, unsigned int *value)
{
	unsigned char	status;

	/* Always return transmitter empty */
	status = TIOCSER_TEMT;
	put_user(status,value);
	return 0;
}

/****************************************************************************/

static void send_break(	struct gbatxt_serial * info, int duration)
{
	/* Not a serial device, no BREAK required. */
}

/****************************************************************************/

static int gbatxt_ioctl(struct tty_struct *tty, struct file * file,
		    unsigned int cmd, unsigned long arg)
{
	struct gbatxt_serial * info = (struct gbatxt_serial *)tty->driver_data;
	unsigned int val;
	int retval, error;
	int dtr, rts;

	if ((cmd != TIOCGSERIAL) && (cmd != TIOCSSERIAL) &&
	    (cmd != TIOCSERCONFIG) && (cmd != TIOCSERGWILD)  &&
	    (cmd != TIOCSERSWILD) && (cmd != TIOCSERGSTRUCT)) {
		if (tty->flags & (1 << TTY_IO_ERROR))
		    return -EIO;
	}
	
	switch (cmd) {
		case TCSBRK:	/* SVID version: non-zero arg --> no break */
			retval = tty_check_change(tty);
			if (retval)
				return retval;
			tty_wait_until_sent(tty, 0);
			if (!arg)
				send_break(info, HZ/4);	/* 1/4 second */
			return 0;
		case TCSBRKP:	/* support for POSIX tcsendbreak() */
			retval = tty_check_change(tty);
			if (retval)
				return retval;
			tty_wait_until_sent(tty, 0);
			send_break(info, arg ? arg*(HZ/10) : HZ/4);
			return 0;
		case TIOCGSOFTCAR:
			error = verify_area(VERIFY_WRITE, (void *) arg,sizeof(long));
			if (error)
				return error;
			put_fs_long(C_CLOCAL(tty) ? 1 : 0,
				    (unsigned long *) arg);
			return 0;
		case TIOCSSOFTCAR:
			arg = get_fs_long((unsigned long *) arg);
			tty->termios->c_cflag =
				((tty->termios->c_cflag & ~CLOCAL) |
				 (arg ? CLOCAL : 0));
			return 0;
		case TIOCGSERIAL:
			error = verify_area(VERIFY_WRITE, (void *) arg,
						sizeof(struct serial_struct));
			if (error)
				return error;
			return get_serial_info(info,
					       (struct serial_struct *) arg);
		case TIOCSSERIAL:
			return set_serial_info(info,
					       (struct serial_struct *) arg);
		case TIOCSERGETLSR: /* Get line status register */
			error = verify_area(VERIFY_WRITE, (void *) arg,
				sizeof(unsigned int));
			if (error)
				return error;
			else
			    return get_lsr_info(info, (unsigned int *) arg);

		case TIOCSERGSTRUCT:
			error = verify_area(VERIFY_WRITE, (void *) arg,
						sizeof(struct gbatxt_serial));
			if (error)
				return error;
			memcpy_tofs((struct gbatxt_serial *) arg,
				    info, sizeof(struct gbatxt_serial));
			return 0;
			
		case TIOCMGET:
			if ((error = verify_area(VERIFY_WRITE, (void *) arg,
                            sizeof(unsigned int))))
                                return(error);
			val = gbatxt_getsignals(info);
			put_user(val, (unsigned int *) arg);
			break;

                case TIOCMBIS:
			if ((error = verify_area(VERIFY_WRITE, (void *) arg,
                            sizeof(unsigned int))))
				return(error);
			val = get_user((unsigned int *) arg);
			rts = (val & TIOCM_RTS) ? 1 : -1;
			dtr = (val & TIOCM_DTR) ? 1 : -1;
			gbatxt_setsignals(info, dtr, rts);
			break;

                case TIOCMBIC:
			if ((error = verify_area(VERIFY_WRITE, (void *) arg,
                            sizeof(unsigned int))))
				return(error);
			val = get_user((unsigned int *) arg);
			rts = (val & TIOCM_RTS) ? 0 : -1;
			dtr = (val & TIOCM_DTR) ? 0 : -1;
			gbatxt_setsignals(info, dtr, rts);
			break;

                case TIOCMSET:
			if ((error = verify_area(VERIFY_WRITE, (void *) arg,
                            sizeof(unsigned int))))
				return(error);
			val = get_user((unsigned int *) arg);
			rts = (val & TIOCM_RTS) ? 1 : 0;
			dtr = (val & TIOCM_DTR) ? 1 : 0;
			gbatxt_setsignals(info, dtr, rts);
			break;

		default:
			return -ENOIOCTLCMD;
		}
	return 0;
}

/****************************************************************************/

static void gbatxt_set_termios(struct tty_struct *tty, struct termios *old_termios)
{
	struct gbatxt_serial *info = (struct gbatxt_serial *)tty->driver_data;

	if (tty->termios->c_cflag == old_termios->c_cflag)
		return;

	gbatxt_change_speed(info);

	if ((old_termios->c_cflag & CRTSCTS) &&
	    !(tty->termios->c_cflag & CRTSCTS)) {
		tty->hw_stopped = 0;
		gbatxt_setsignals(info, -1, 1);
		gbatxt_start(tty);
	}
}

/****************************************************************************/

/*
 * ------------------------------------------------------------
 * gbatxt_close()
 * 
 * This routine is called when the serial port gets closed.  First, we
 * wait for the last remaining data to be sent.  Then, we unlink its
 * S structure from the interrupt chain if necessary, and we free
 * that IRQ if nothing is left in the chain.
 * ------------------------------------------------------------
 */

static void gbatxt_close(struct tty_struct *tty, struct file * filp)
{
	struct gbatxt_serial	*info = (struct gbatxt_serial *)tty->driver_data;
	unsigned long		flags;

	if (!info)
		return;
	
	save_flags(flags); cli();
	
	if (tty_hung_up_p(filp)) {
		restore_flags(flags);
		return;
	}
	
#ifdef SERIAL_DEBUG_OPEN
	printk("gbatxt_close ttyS%d, count = %d\n", info->line, info->count);
#endif
	if ((tty->count == 1) && (info->count != 1)) {
		/*
		 * Uh, oh.  tty->count is 1, which means that the tty
		 * structure will be freed.  Info->count should always
		 * be one in these conditions.  If it's greater than
		 * one, we've got real problems, since it means the
		 * serial port won't be shutdown.
		 */
		printk("gbatxt_close: bad serial port count; tty->count is 1, "
		       "info->count is %d\n", info->count);
		info->count = 1;
	}
	if (--info->count < 0) {
		printk("gbatxt_close: bad serial port count for ttyS%d: %d\n",
		       info->line, info->count);
		info->count = 0;
	}
	if (info->count) {
		restore_flags(flags);
		return;
	}
	info->flags |= ASYNC_CLOSING;

	/*
	 * Save the termios structure, since this port may have
	 * separate termios for callout and dialin.
	 */
	if (info->flags & ASYNC_NORMAL_ACTIVE)
		info->normal_termios = *tty->termios;
	if (info->flags & ASYNC_CALLOUT_ACTIVE)
		info->callout_termios = *tty->termios;

	/*
	 * Now we wait for the transmit buffer to clear; and we notify 
	 * the line discipline to only process XON/XOFF characters.
	 */
	tty->closing = 1;
	if (info->closing_wait != ASYNC_CLOSING_WAIT_NONE)
		tty_wait_until_sent(tty, info->closing_wait);

#if 0
	/*
	 * At this point we stop accepting input.  To do this, we
	 * disable the receive line status interrupts, and tell the
	 * interrupt driver to stop checking the data ready bit in the
	 * line status register.
	 */
	info->imr &= ~MCFUART_UIR_RXREADY;
	uartp = (volatile unsigned char *) info->addr;
	uartp[MCFUART_UIMR] = info->imr;
#endif

#if 0
	/* FIXME: do we need to keep this enabled for console?? */
	if (gbatxt_console_inited && (gbatxt_console_port == info->line)) {
		/* Do not disable the UART */ ;
	} else
#endif
	shutdown(info);
	if (tty->driver.flush_buffer)
		tty->driver.flush_buffer(tty);
	if (tty->ldisc.flush_buffer)
		tty->ldisc.flush_buffer(tty);
	tty->closing = 0;
	info->event = 0;
	info->tty = 0;
	if (tty->ldisc.num != ldiscs[N_TTY].num) {
		if (tty->ldisc.close)
			(tty->ldisc.close)(tty);
		tty->ldisc = ldiscs[N_TTY];
		tty->termios->c_line = N_TTY;
		if (tty->ldisc.open)
			(tty->ldisc.open)(tty);
	}
	if (info->blocked_open) {
		if (info->close_delay) {
			current->state = TASK_INTERRUPTIBLE;
			current->timeout = jiffies + info->close_delay;
			schedule();
		}
		wake_up_interruptible(&info->open_wait);
	}
	info->flags &= ~(ASYNC_NORMAL_ACTIVE|ASYNC_CALLOUT_ACTIVE|
			 ASYNC_CLOSING);
	wake_up_interruptible(&info->close_wait);
	restore_flags(flags);
}

/****************************************************************************/

/*
 * gbatxt_hangup() --- called by tty_hangup() when a hangup is signaled.
 */

void gbatxt_hangup(struct tty_struct *tty)
{
	struct gbatxt_serial * info = (struct gbatxt_serial *)tty->driver_data;
	
	gbatxt_flush_buffer(tty);
	shutdown(info);
	info->event = 0;
	info->count = 0;
	info->flags &= ~(ASYNC_NORMAL_ACTIVE|ASYNC_CALLOUT_ACTIVE);
	info->tty = 0;
	wake_up_interruptible(&info->open_wait);
}

/****************************************************************************/

/*
 * ------------------------------------------------------------
 * gbatxt_open() and friends
 * ------------------------------------------------------------
 */

static int block_til_ready(struct tty_struct *tty, struct file * filp,
			   struct gbatxt_serial *info)
{
	struct wait_queue wait = { current, NULL };
	int		retval;
	int		do_clocal = 0;

	/*
	 * If the device is in the middle of being closed, then block
	 * until it's done, and then try again.
	 */
	if (info->flags & ASYNC_CLOSING) {
		interruptible_sleep_on(&info->close_wait);
#ifdef SERIAL_DO_RESTART
		if (info->flags & ASYNC_HUP_NOTIFY)
			return -EAGAIN;
		else
			return -ERESTARTSYS;
#else
		return -EAGAIN;
#endif
	}

	/*
	 * If this is a callout device, then just make sure the normal
	 * device isn't being used.
	 */
	if (tty->driver.subtype == SERIAL_TYPE_CALLOUT) {
		if (info->flags & ASYNC_NORMAL_ACTIVE)
			return -EBUSY;
		if ((info->flags & ASYNC_CALLOUT_ACTIVE) &&
		    (info->flags & ASYNC_SESSION_LOCKOUT) &&
		    (info->session != current->session))
		    return -EBUSY;
		if ((info->flags & ASYNC_CALLOUT_ACTIVE) &&
		    (info->flags & ASYNC_PGRP_LOCKOUT) &&
		    (info->pgrp != current->pgrp))
		    return -EBUSY;
		info->flags |= ASYNC_CALLOUT_ACTIVE;
		return 0;
	}
	
	/*
	 * If non-blocking mode is set, or the port is not enabled,
	 * then make the check up front and then exit.
	 */
	if ((filp->f_flags & O_NONBLOCK) ||
	    (tty->flags & (1 << TTY_IO_ERROR))) {
		if (info->flags & ASYNC_CALLOUT_ACTIVE)
			return -EBUSY;
		info->flags |= ASYNC_NORMAL_ACTIVE;
		return 0;
	}

	if (info->flags & ASYNC_CALLOUT_ACTIVE) {
		if (info->normal_termios.c_cflag & CLOCAL)
			do_clocal = 1;
	} else {
		if (tty->termios->c_cflag & CLOCAL)
			do_clocal = 1;
	}
	
	/*
	 * Block waiting for the carrier detect and the line to become
	 * free (i.e., not in use by the callout).  While we are in
	 * this loop, info->count is dropped by one, so that
	 * gbatxt_close() knows when to free things.  We restore it upon
	 * exit, either normal or abnormal.
	 */
	retval = 0;
	add_wait_queue(&info->open_wait, &wait);
#ifdef SERIAL_DEBUG_OPEN
	printk("block_til_ready before block: ttyS%d, count = %d\n",
	       info->line, info->count);
#endif
	info->count--;
	info->blocked_open++;
	while (1) {
		cli();
		if (!(info->flags & ASYNC_CALLOUT_ACTIVE))
			gbatxt_setsignals(info, 1, 1);
		sti();
		current->state = TASK_INTERRUPTIBLE;
		if (tty_hung_up_p(filp) ||
		    !(info->flags & ASYNC_INITIALIZED)) {
#ifdef SERIAL_DO_RESTART
			if (info->flags & ASYNC_HUP_NOTIFY)
				retval = -EAGAIN;
			else
				retval = -ERESTARTSYS;	
#else
			retval = -EAGAIN;
#endif
			break;
		}
		if (!(info->flags & ASYNC_CALLOUT_ACTIVE) &&
		    !(info->flags & ASYNC_CLOSING) && do_clocal)
			break;
		if (current->signal & ~current->blocked) {
			retval = -ERESTARTSYS;
			break;
		}
#ifdef SERIAL_DEBUG_OPEN
		printk("block_til_ready blocking: ttyS%d, count = %d\n",
		       info->line, info->count);
#endif
		schedule();
	}
	current->state = TASK_RUNNING;
	remove_wait_queue(&info->open_wait, &wait);
	if (!tty_hung_up_p(filp))
		info->count++;
	info->blocked_open--;
#ifdef SERIAL_DEBUG_OPEN
	printk("block_til_ready after blocking: ttyS%d, count = %d\n",
	       info->line, info->count);
#endif
	if (retval)
		return retval;
	info->flags |= ASYNC_NORMAL_ACTIVE;
	return 0;
}	

/****************************************************************************/

int gbatxt_open(struct tty_struct *tty, struct file * filp)
{
	struct gbatxt_serial	*info;
	int 			retval, line;

	line = MINOR(tty->device) - tty->driver.minor_start;
	if ((line < 0) || (line >= NR_PORTS))
		return -ENODEV;
	info = gbatxt_table + line;
#ifdef SERIAL_DEBUG_OPEN
	printk("gbatxt_open %s%d, count = %d\n", tty->driver.name, info->line,
	       info->count);
#endif

	info->count++;
	tty->driver_data = info;
	info->tty = tty;

	/*
	 * Start up serial port
	 */
	retval = startup(info);
	if (retval)
		return retval;

	retval = block_til_ready(tty, filp, info);
	if (retval) {
#ifdef SERIAL_DEBUG_OPEN
		printk("gbatxt_open returning after block_til_ready with %d\n",
		       retval);
#endif
		return retval;
	}

	if ((info->count == 1) && (info->flags & ASYNC_SPLIT_TERMIOS)) {
		if (tty->driver.subtype == SERIAL_TYPE_NORMAL)
			*tty->termios = info->normal_termios;
		else 
			*tty->termios = info->callout_termios;
		gbatxt_change_speed(info);
	}

	info->session = current->session;
	info->pgrp = current->pgrp;

#ifdef SERIAL_DEBUG_OPEN
	printk("gbatxt_open ttyS%d successful...\n", info->line);
#endif
	return 0;
}

/****************************************************************************/

/*
 *	Set up the internal interrupt stuff.
 */

static void gbatxt_irqinit(struct gbatxt_serial *info)
{
#if 0
	if (request_irq(info->irq, gbatxt_interrupt, SA_INTERRUPT,
	    "ColdFire UART", NULL)) {
		printk("SERIAL: Unable to attach ColdFire UART %d interrupt "
			"vector=%d\n", info->line, info->irq);
	}
#endif
}

/****************************************************************************/

/*
 *	Serial stats reporting...
 */

int gbatxt_readproc(char *buffer)
{
	struct gbatxt_serial	*info;
	char			str[20];
	int			len, sigs, i;

	len = sprintf(buffer, gbatxt_drivername);
	for (i = 0; (i < NR_PORTS); i++) {
		info = &gbatxt_table[i];
		len += sprintf((buffer + len), "%d: port:%x irq=%d baud:%d ",
			i, info->addr, info->irq, info->baud);
		if (info->stats.rx || info->stats.tx)
			len += sprintf((buffer + len), "tx:%d rx:%d ",
			info->stats.tx, info->stats.rx);
		if (info->stats.rxframing)
			len += sprintf((buffer + len), "fe:%d ",
			info->stats.rxframing);
		if (info->stats.rxparity)
			len += sprintf((buffer + len), "pe:%d ",
			info->stats.rxparity);
		if (info->stats.rxbreak)
			len += sprintf((buffer + len), "brk:%d ",
			info->stats.rxbreak);
		if (info->stats.rxoverrun)
			len += sprintf((buffer + len), "oe:%d ",
			info->stats.rxoverrun);

		str[0] = str[1] = 0;
		if ((sigs = gbatxt_getsignals(info))) {
			if (sigs & TIOCM_RTS)
				strcat(str, "|RTS");
			if (sigs & TIOCM_CTS)
				strcat(str, "|CTS");
			if (sigs & TIOCM_DTR)
				strcat(str, "|DTR");
			if (sigs & TIOCM_CD)
				strcat(str, "|CD");
		}

		len += sprintf((buffer + len), "%s\n", &str[1]);
	}

	return(len);
}

/****************************************************************************/

/* Finally, routines used to initialize the serial driver. */

static void show_serial_version(void)
{
	printk(gbatxt_drivername);
}

/****************************************************************************/

int gbatxt_init(void)
{
	struct gbatxt_serial	*info;
	unsigned long		flags;
	int			i;

#if 0
	/* Setup base handler, and timer table. */
	init_bh(CM206_BH, do_serial_bh);
	timer_table[RS_TIMER].fn = gbatxt_timer;
	timer_table[RS_TIMER].expires = 0;
#endif

	show_serial_version();

	/* Initialize the tty_driver structure */
	memset(&gbatxt_serial_driver, 0, sizeof(struct tty_driver));
	gbatxt_serial_driver.magic = TTY_DRIVER_MAGIC;
	gbatxt_serial_driver.name = "ttyS";
	gbatxt_serial_driver.major = TTY_MAJOR;
	gbatxt_serial_driver.minor_start = 64;
	gbatxt_serial_driver.num = NR_PORTS;
	gbatxt_serial_driver.type = TTY_DRIVER_TYPE_SERIAL;
	gbatxt_serial_driver.subtype = SERIAL_TYPE_NORMAL;
	gbatxt_serial_driver.init_termios = tty_std_termios;

	gbatxt_serial_driver.init_termios.c_cflag =
		B9600 | CS8 | CREAD | HUPCL | CLOCAL;
	gbatxt_serial_driver.flags = TTY_DRIVER_REAL_RAW;
	gbatxt_serial_driver.refcount = &gbatxt_serial_refcount;
	gbatxt_serial_driver.table = gbatxt_serial_table;
	gbatxt_serial_driver.termios = gbatxt_serial_termios;
	gbatxt_serial_driver.termios_locked = gbatxt_serial_termios_locked;

	gbatxt_serial_driver.open = gbatxt_open;
	gbatxt_serial_driver.close = gbatxt_close;
	gbatxt_serial_driver.write = gbatxt_write;
	gbatxt_serial_driver.flush_chars = gbatxt_flush_chars;
	gbatxt_serial_driver.write_room = gbatxt_write_room;
	gbatxt_serial_driver.chars_in_buffer = gbatxt_chars_in_buffer;
	gbatxt_serial_driver.flush_buffer = gbatxt_flush_buffer;
	gbatxt_serial_driver.ioctl = gbatxt_ioctl;
	gbatxt_serial_driver.throttle = gbatxt_throttle;
	gbatxt_serial_driver.unthrottle = gbatxt_unthrottle;
	gbatxt_serial_driver.set_termios = gbatxt_set_termios;
	gbatxt_serial_driver.stop = gbatxt_stop;
	gbatxt_serial_driver.start = gbatxt_start;
	gbatxt_serial_driver.hangup = gbatxt_hangup;

	/*
	 * The callout device is just like normal device except for
	 * major number and the subtype code.
	 */
	gbatxt_callout_driver = gbatxt_serial_driver;
	gbatxt_callout_driver.name = "cua";
	gbatxt_callout_driver.major = TTYAUX_MAJOR;
	gbatxt_callout_driver.subtype = SERIAL_TYPE_CALLOUT;

	if (tty_register_driver(&gbatxt_serial_driver))
		panic("Couldn't register serial driver\n");
	if (tty_register_driver(&gbatxt_callout_driver))
		panic("Couldn't register callout driver\n");
	
	save_flags(flags); cli();

	/*
	 *	Configure all the attached serial ports.
	 */
	for (i = 0, info = gbatxt_table; (i < NR_PORTS); i++, info++) {
		info->magic = SERIAL_MAGIC;
		info->line = i;
		info->tty = 0;
		info->custom_divisor = 16;
		info->close_delay = 50;
		info->closing_wait = 3000;
		info->event = 0;
		info->count = 0;
		info->blocked_open = 0;
		info->tqueue.routine = do_softint;
		info->tqueue.data = info;
		info->tqueue_hangup.routine = do_serial_hangup;
		info->tqueue_hangup.data = info;
		info->callout_termios = gbatxt_callout_driver.init_termios;
		info->normal_termios = gbatxt_serial_driver.init_termios;
		info->open_wait = 0;
		info->close_wait = 0;

		gbatxt_setsignals(info, 0, 0);
		gbatxt_irqinit(info);

		printk("%s%d", gbatxt_serial_driver.name, info->line);
		printk(" is a GBA console\n");
	}

	restore_flags(flags);
	return 0;
}

/****************************************************************************/
/*                          System Console                                  */
/****************************************************************************/

struct charmap {
	unsigned char	bitmap[8];
};

const struct charmap	gbatxt_charmap[] = {
/*00*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*01*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*02*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*03*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*04*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*05*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*06*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*07*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*08*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*09*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*0a*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*0b*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*0c*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*0d*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*0e*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*0f*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*10*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*11*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*12*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*13*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*14*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*15*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*16*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*17*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*18*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*19*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*1a*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*1b*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*1c*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*1d*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*1e*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*1f*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*20*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*21*/	{ { 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x10, 0x00 } },
/*22*/	{ { 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*23*/	{ { 0x28, 0x28, 0xfe, 0x28, 0xfe, 0x28, 0x28, 0x00 } },
/*24*/	{ { 0x10, 0x7c, 0x80, 0x7c, 0x02, 0x7c, 0x10, 0x00 } },
/*25*/	{ { 0xc2, 0xc4, 0x08, 0x10, 0x20, 0x46, 0x86, 0x00 } },
/*26*/	{ { 0x60, 0x90, 0x60, 0x90, 0x88, 0x84, 0x7a, 0x00 } },
/*27*/	{ { 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*28*/	{ { 0x08, 0x10, 0x20, 0x20, 0x20, 0x10, 0x08, 0x00 } },
/*29*/	{ { 0x20, 0x10, 0x08, 0x08, 0x08, 0x10, 0x20, 0x00 } },
/*2a*/	{ { 0x44, 0x28, 0x10, 0xfe, 0x10, 0x28, 0x44, 0x00 } },
/*2b*/	{ { 0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00, 0x00 } },
/*2c*/	{ { 0x00, 0x00, 0x00, 0x00, 0x18, 0x08, 0x10, 0x00 } },
/*2d*/	{ { 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00 } },
/*2e*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00 } },
/*2f*/	{ { 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00 } },
/*30*/	{ { 0x7c, 0x86, 0x8a, 0x92, 0xa2, 0xc2, 0x7c, 0x00 } },
/*31*/	{ { 0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00 } },
/*32*/	{ { 0x7c, 0x82, 0x04, 0x08, 0x10, 0x20, 0xfe, 0x00 } },
/*33*/	{ { 0x7c, 0x82, 0x02, 0x3c, 0x02, 0x82, 0x7c, 0x00 } },
/*34*/	{ { 0x40, 0x40, 0x84, 0x84, 0xfe, 0x04, 0x04, 0x00 } },
/*35*/	{ { 0xfe, 0x80, 0x80, 0xfc, 0x02, 0x82, 0x7c, 0x00 } },
/*36*/	{ { 0x10, 0x20, 0x40, 0xfc, 0x82, 0x82, 0x7c, 0x00 } },
/*37*/	{ { 0xfe, 0x02, 0x04, 0x08, 0x10, 0x10, 0x10, 0x00 } },
/*38*/	{ { 0x7c, 0x82, 0x82, 0x7c, 0x82, 0x82, 0x7c, 0x00 } },
/*39*/	{ { 0x7c, 0x82, 0x82, 0x7e, 0x04, 0x08, 0x10, 0x00 } },
/*3a*/	{ { 0x00, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00 } },
/*3b*/	{ { 0x00, 0x00, 0x10, 0x00, 0x10, 0x10, 0x20, 0x00 } },
/*3c*/	{ { 0x04, 0x08, 0x10, 0x20, 0x10, 0x08, 0x04, 0x00 } },
/*3d*/	{ { 0x00, 0x00, 0x7c, 0x00, 0x7c, 0x00, 0x00, 0x00 } },
/*3e*/	{ { 0x40, 0x20, 0x10, 0x08, 0x10, 0x20, 0x40, 0x00 } },
/*3f*/	{ { 0x7c, 0x82, 0x04, 0x08, 0x10, 0x00, 0x10, 0x00 } },
/*40*/	{ { 0x7c, 0x82, 0xbe, 0xa2, 0xbe, 0x80, 0x7e, 0x00 } },
/*41*/	{ { 0x10, 0x28, 0x44, 0x82, 0xfe, 0x82, 0x82, 0x00 } },
/*42*/	{ { 0xfc, 0x82, 0x82, 0xfc, 0x82, 0x82, 0xfc, 0x00 } },
/*43*/	{ { 0x7c, 0x82, 0x80, 0x80, 0x80, 0x82, 0x7c, 0x00 } },
/*44*/	{ { 0xfc, 0x82, 0x82, 0x82, 0x82, 0x82, 0xfc, 0x00 } },
/*45*/	{ { 0xfe, 0x80, 0x80, 0xfc, 0x80, 0x80, 0xfe, 0x00 } },
/*46*/	{ { 0xfe, 0x80, 0x80, 0xfc, 0x80, 0x80, 0x80, 0x00 } },
/*47*/	{ { 0x7c, 0x82, 0x80, 0x8e, 0x82, 0x82, 0x7e, 0x00 } },
/*48*/	{ { 0x82, 0x82, 0x82, 0xfe, 0x82, 0x82, 0x82, 0x00 } },
/*49*/	{ { 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00 } },
/*4a*/	{ { 0x02, 0x02, 0x02, 0x02, 0x02, 0x82, 0x7c, 0x00 } },
/*4b*/	{ { 0x82, 0x8c, 0xb0, 0xc0, 0xb0, 0x8c, 0x82, 0x00 } },
/*4c*/	{ { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xfe, 0x00 } },
/*4d*/	{ { 0x82, 0xc6, 0xaa, 0x92, 0x82, 0x82, 0x82, 0x00 } },
/*4e*/	{ { 0x82, 0xc2, 0xa2, 0x92, 0x8a, 0x86, 0x82, 0x00 } },
/*4f*/	{ { 0x7c, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00 } },
/*50*/	{ { 0xfc, 0x82, 0x82, 0xfc, 0x80, 0x80, 0x80, 0x00 } },
/*51*/	{ { 0x7c, 0x82, 0x82, 0x82, 0x82, 0x84, 0x7a, 0x00 } },
/*52*/	{ { 0xfc, 0x82, 0x82, 0xfc, 0x88, 0x84, 0x82, 0x00 } },
/*53*/	{ { 0x7c, 0x82, 0x80, 0x7c, 0x02, 0x82, 0x7c, 0x00 } },
/*54*/	{ { 0xfe, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00 } },
/*55*/	{ { 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00 } },
/*56*/	{ { 0x82, 0x82, 0x82, 0x82, 0x44, 0x28, 0x10, 0x00 } },
/*57*/	{ { 0x82, 0x82, 0x82, 0x92, 0x92, 0x54, 0x28, 0x00 } },
/*58*/	{ { 0x82, 0x44, 0x28, 0x10, 0x28, 0x44, 0x82, 0x00 } },
/*59*/	{ { 0x82, 0x82, 0x44, 0x28, 0x10, 0x10, 0x10, 0x00 } },
/*5a*/	{ { 0xfe, 0x04, 0x08, 0x10, 0x20, 0x40, 0xfe, 0x00 } },
/*5b*/	{ { 0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x00 } },
/*5c*/	{ { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00 } },
/*5d*/	{ { 0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x38, 0x00 } },
/*5e*/	{ { 0x10, 0x28, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*5f*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00 } },
/*60*/	{ { 0x20, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*61*/	{ { 0x00, 0x00, 0x7e, 0x82, 0x82, 0x86, 0x7a, 0x00 } },
/*62*/	{ { 0x80, 0x80, 0xfc, 0x82, 0x82, 0x82, 0xfc, 0x00 } },
/*63*/	{ { 0x00, 0x00, 0x7e, 0x80, 0x80, 0x80, 0x7e, 0x00 } },
/*64*/	{ { 0x02, 0x02, 0x7e, 0x82, 0x82, 0x82, 0x7e, 0x00 } },
/*65*/	{ { 0x00, 0x00, 0x7c, 0x82, 0xfe, 0x80, 0x7c, 0x00 } },
/*66*/	{ { 0x0e, 0x10, 0x7c, 0x10, 0x10, 0x10, 0x10, 0x00 } },
/*67*/	{ { 0x00, 0x00, 0x7e, 0x82, 0x7e, 0x02, 0x7c, 0x00 } },
/*68*/	{ { 0x80, 0x80, 0xfc, 0x82, 0x82, 0x82, 0x82, 0x00 } },
/*69*/	{ { 0x10, 0x00, 0x30, 0x10, 0x10, 0x10, 0x38, 0x00 } },
/*6a*/	{ { 0x08, 0x00, 0x18, 0x08, 0x08, 0x08, 0x70, 0x00 } },
/*6b*/	{ { 0x80, 0x80, 0x86, 0x98, 0xe0, 0x98, 0x86, 0x00 } },
/*6c*/	{ { 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00 } },
/*6d*/	{ { 0x00, 0x00, 0xec, 0x92, 0x92, 0x92, 0x82, 0x00 } },
/*6e*/	{ { 0x00, 0x00, 0xbc, 0xc2, 0x82, 0x82, 0x82, 0x00 } },
/*6f*/	{ { 0x00, 0x00, 0x7c, 0x82, 0x82, 0x82, 0x7c, 0x00 } },
/*70*/	{ { 0x00, 0x00, 0xfc, 0x82, 0xfc, 0x80, 0x80, 0x00 } },
/*71*/	{ { 0x00, 0x00, 0x7e, 0x82, 0x7e, 0x02, 0x02, 0x00 } },
/*72*/	{ { 0x00, 0x00, 0xbe, 0xc0, 0x80, 0x80, 0x80, 0x00 } },
/*73*/	{ { 0x00, 0x00, 0x7e, 0x80, 0x7c, 0x02, 0xfc, 0x00 } },
/*74*/	{ { 0x10, 0x10, 0x7c, 0x10, 0x10, 0x10, 0x0c, 0x00 } },
/*75*/	{ { 0x00, 0x00, 0x82, 0x82, 0x82, 0x86, 0x7a, 0x00 } },
/*76*/	{ { 0x00, 0x00, 0x82, 0x82, 0x44, 0x28, 0x10, 0x00 } },
/*77*/	{ { 0x00, 0x00, 0x92, 0x92, 0x92, 0x92, 0x6c, 0x00 } },
/*78*/	{ { 0x00, 0x00, 0x82, 0x44, 0x38, 0x44, 0x82, 0x00 } },
/*79*/	{ { 0x00, 0x00, 0x82, 0x82, 0x7e, 0x02, 0x7c, 0x00 } },
/*7a*/	{ { 0x00, 0x00, 0xfe, 0x0c, 0x30, 0xc0, 0xfe, 0x00 } },
/*7b*/	{ { 0x0c, 0x10, 0x10, 0x20, 0x10, 0x10, 0x0c, 0x00 } },
/*7c*/	{ { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00 } },
/*7d*/	{ { 0x60, 0x10, 0x10, 0x08, 0x10, 0x10, 0x60, 0x00 } },
/*7e*/	{ { 0x34, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
/*7f*/	{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
};

/*
 *	Create text tiles in tile map. this is a simple bit conversion
 *	from the base character map table.
 */

void gbatxt_inittext(void)
{
	unsigned short	*vp;
	struct charmap	*cp;
	unsigned char	bits;
	int		i, j, r, b;

	vp = (unsigned short *) GBA_VIDBASE;

	for (j = 0; (j < 2); j++) {
		cp = (struct charmap *) &gbatxt_charmap[0];
		for (i = 0; (i < 128); i++, cp++) {
			for (r = 0; (r < 8); r++) {
				bits = cp->bitmap[r];
				for (b = 0; (b < 8); b += 2) {
					*vp++ = ((bits & (0x80>>b)) ? 1 : 0) |
					    ((bits & (0x40>>b)) ? 0x100 : 0);
				}
			}
		}
	}
}

/****************************************************************************/

/*
 *	Initialize the GBA screen into mode 0. That is a simple tiled
 *	mode, nice and easy to do conventional text.
 */

void gbatxt_console_init(void)
{
	volatile unsigned short	*pp;
	int			i;

	/* Enable mode 0 */
	*((unsigned short *) (GBA_CTRLBASE + 0x00)) = 0x0100;	/*MODE0|BG0*/
	*((unsigned short *) (GBA_CTRLBASE + 0x08)) = 0x1080;

	/* Default palete, everything is white :-) */
	pp = (volatile unsigned short *) GBA_PALBASE;
	for (i = 255; i; i--)
		pp[i] = 0x7fff;
	pp[0] = 0;

	gbatxt_x = 0;
	gbatxt_y = 0;

	gbatxt_inittext();
	gbatxt_console_inited++;
}

/****************************************************************************/

void gbatxt_setxy(int x, int y)
{
	if ((x >= 0) && (x < GBA_XLEN) && (y >= 0) && (y < GBA_YLEN)) {
		gbatxt_x = x;
		gbatxt_y = y;
	}
}

/****************************************************************************/

int gbatxt_getx(void)
{
	return(gbatxt_x);
}

int gbatxt_gety(void)
{
	return(gbatxt_y);
}

/****************************************************************************/

/*
 *	Scroll one screen line. There is probably a better/quicker
 *	way to do this. But this will do for now...
 */

void gbatxt_scroll(void)
{
	unsigned short	*sp, *dp;
	int		len;

	dp = (unsigned short *) (GBA_VIDBASE + 0x8000);
	sp = dp + GBA_XMAX;
	len = (GBA_XMAX * (GBA_YLEN - 1));

	while (len--)
		*dp++ = *sp++;
	for (len = GBA_XMAX; len; len--)
		*dp++ = 0;
	
}

/****************************************************************************/

void gbatxt_console_putc(int c)
{
	unsigned short	*vp;

	vp = (unsigned short *) (GBA_VIDBASE + 0x8000);
	vp += (gbatxt_y * GBA_XMAX) + gbatxt_x;
	*vp = c;

	gbatxt_x++;
	if ((gbatxt_x >= GBA_XLEN) || (c == '\n')) {
		gbatxt_x = 0;
		gbatxt_y++;
		if (gbatxt_y >= GBA_YLEN) {
			gbatxt_scroll();
			gbatxt_y = GBA_YLEN - 1;
		}
	}
	if (c == '\r')
		gbatxt_x = 0;
}

/****************************************************************************/

void gbatxt_console_print(char *s)
{
	for (; *s; s++)
		gbatxt_console_putc(*s);	
}

/****************************************************************************/
