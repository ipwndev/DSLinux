/*
 *  linux/drivers/char/tty_io.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * 'tty_io.c' gives an orthogonal feeling to tty's, be they consoles
 * or rs-channels. It also implements echoing, cooked mode etc.
 *
 * Kill-line thanks to John T Kohl, who also corrected VMIN = VTIME = 0.
 *
 * Modified by Theodore Ts'o, 9/14/92, to dynamically allocate the
 * tty_struct and tty_queue structures.  Previously there was an array
 * of 256 tty_struct's which was statically allocated, and the
 * tty_queue structures were allocated at boot time.  Both are now
 * dynamically allocated only when the tty is open.
 *
 * Also restructured routines so that there is more of a separation
 * between the high-level tty routines (tty_io.c and tty_ioctl.c) and
 * the low-level tty routines (serial.c, pty.c, console.c).  This
 * makes for cleaner and more compact code.  -TYT, 9/17/92 
 *
 * Modified by Fred N. van Kempen, 01/29/93, to add line disciplines
 * which can be dynamically activated and de-activated by the line
 * discipline handling modules (like SLIP).
 *
 * NOTE: pay no attention to the line discipline code (yet); its
 * interface is still subject to change in this version...
 * -- TYT, 1/31/92
 *
 * Added functionality to the OPOST tty handling.  No delays, but all
 * other bits should be there.
 *	-- Nick Holloway <alfie@dcs.warwick.ac.uk>, 27th May 1993.
 *
 * Rewrote canonical mode and added more termios flags.
 * 	-- julian@uhunix.uhcc.hawaii.edu (J. Cowley), 13Jan94
 *
 * Reorganized FASYNC support so mouse code can share it.
 *	-- ctm@ardi.com, 9Sep95
 *
 * New TIOCLINUX variants added.
 *	-- mj@k332.feld.cvut.cz, 19-Nov-95
 * 
 * Restrict vt switching via ioctl()
 *      -- grif@cs.ucr.edu, 5-Dec-95
 *
 * Rewrote init_dev and release_dev to eliminate races.
 *	-- Bill Hawes <whawes@star.net>, June 97
 *
 * Configurable CONSOLE, FRAMEBUFFER, and KEYBOARD
 *	-- Kenneth Albanowski <kjahds@kjahds.com>, '98
 */

#include <linux/config.h>
#include <linux/types.h>
#include <linux/major.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/fcntl.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/timer.h>
#include <linux/ctype.h>
#include <linux/kd.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/malloc.h>

#include <asm/segment.h>
#include <asm/system.h>
#include <asm/bitops.h>

#include <linux/scc.h>
#include <linux/fb.h>

#include "kbd_kern.h"
#include "vt_kern.h"
#include "selection.h"

#ifdef CONFIG_KERNELD
#include <linux/kerneld.h>
#endif

#ifdef CONFIG_M68360
#include <asm/quicc_cpm.h>
#endif
#define CONSOLE_DEV MKDEV(TTY_MAJOR,0)
#define TTY_DEV MKDEV(TTYAUX_MAJOR,0)

#undef TTY_DEBUG_HANGUP

#define TTY_PARANOIA_CHECK
#define CHECK_TTY_COUNT

extern void do_blank_screen(int nopowersave);
extern void set_vesa_blanking(const unsigned long arg);

struct termios tty_std_termios;		/* for the benefit of tty drivers  */
struct tty_driver *tty_drivers = NULL;	/* linked list of tty drivers */
struct tty_ldisc ldiscs[NR_LDISCS];	/* line disc dispatch table	*/

/*
 * fg_console is the current virtual console,
 * last_console is the last used one,
 * want_console is the console we want to switch to,
 * kmsg_redirect is the console for kernel messages,
 * redirect is the pseudo-tty that console output
 * is redirected to if asked by TIOCCONS.
 */
int fg_console = 0;
int last_console = 0;
int want_console = -1;
int kmsg_redirect = 0;
struct tty_struct * redirect = NULL;
struct wait_queue * keypress_wait = NULL;
char vt_dont_switch = 0;

static void initialize_tty_struct(struct tty_struct *tty);

static int tty_read(struct inode *, struct file *, char *, int);
static int tty_write(struct inode *, struct file *, const char *, int);
static int tty_select(struct inode *, struct file *, int, select_table *);
static int tty_open(struct inode *, struct file *);
static void tty_release(struct inode *, struct file *);
static int tty_ioctl(struct inode * inode, struct file * file,
		     unsigned int cmd, unsigned long arg);
static int tty_fasync(struct inode * inode, struct file * filp, int on);

extern void reset_palette(int currcons) ;
extern void set_palette(void) ;

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

/*
 * These two routines return the name of tty.  tty_name() should NOT
 * be used in interrupt drivers, since it's not re-entrant.  Use
 * _tty_name() instead.
 */
char *_tty_name(struct tty_struct *tty, char *buf)
{
	if (tty)
		sprintf(buf, "%s%d", tty->driver.name,
			MINOR(tty->device) - tty->driver.minor_start +
			tty->driver.name_base);
	else
		strcpy(buf, "NULL tty");
	return buf;
}

char *tty_name(struct tty_struct *tty)
{
	static char buf[64];

	return(_tty_name(tty, buf));
}

inline int tty_paranoia_check(struct tty_struct *tty, kdev_t device,
			      const char *routine)
{
#ifdef TTY_PARANOIA_CHECK
	static const char *badmagic =
		"Warning: bad magic number for tty struct (%s) in %s\n";
	static const char *badtty =
		"Warning: null TTY for (%s) in %s\n";

	if (!tty) {
		printk(badtty, kdevname(device), routine);
		return 1;
	}
	if (tty->magic != TTY_MAGIC) {
		printk(badmagic, kdevname(device), routine);
		return 1;
	}
#endif
	return 0;
}

static int check_tty_count(struct tty_struct *tty, const char *routine)
{
#ifdef CHECK_TTY_COUNT
	struct file *f;
	int i, count = 0;
	
	for (f = first_file, i=0; i<nr_files; i++, f = f->f_next) {
		if (!f->f_count)
			continue;
		if (f->private_data == tty) {
			count++;
		}
	}
	if (tty->driver.type == TTY_DRIVER_TYPE_PTY &&
	    tty->driver.subtype == PTY_TYPE_SLAVE &&
	    tty->link && tty->link->count)
		count++;
	if (tty->count != count) {
		printk("Warning: dev (%s) tty->count(%d) != #fd's(%d) in %s\n",
		       kdevname(tty->device), tty->count, count, routine);
		return count;
       }	
#endif
	return 0;
}

int tty_register_ldisc(int disc, struct tty_ldisc *new_ldisc)
{
	if (disc < N_TTY || disc >= NR_LDISCS)
		return -EINVAL;
	
	if (new_ldisc) {
		ldiscs[disc] = *new_ldisc;
		ldiscs[disc].flags |= LDISC_FLAG_DEFINED;
		ldiscs[disc].num = disc;
	} else
		memset(&ldiscs[disc], 0, sizeof(struct tty_ldisc));
	
	return 0;
}

/* Set the discipline of a tty line. */
static int tty_set_ldisc(struct tty_struct *tty, int ldisc)
{
	int	retval = 0;
	struct	tty_ldisc o_ldisc;

	if ((ldisc < N_TTY) || (ldisc >= NR_LDISCS))
		return -EINVAL;
#ifdef CONFIG_KERNELD
	/* Eduardo Blanco <ejbs@cs.cs.com.uy> */
	if (!(ldiscs[ldisc].flags & LDISC_FLAG_DEFINED)) {
		char modname [20];
		sprintf(modname, "tty-ldisc-%d", ldisc);
		request_module (modname);
	}
#endif
	if (!(ldiscs[ldisc].flags & LDISC_FLAG_DEFINED))
		return -EINVAL;

	if (tty->ldisc.num == ldisc)
		return 0;	/* We are already in the desired discipline */
	o_ldisc = tty->ldisc;

	tty_wait_until_sent(tty, 0);
	
	/* Shutdown the current discipline. */
	if (tty->ldisc.close)
		(tty->ldisc.close)(tty);

	/* Now set up the new line discipline. */
	tty->ldisc = ldiscs[ldisc];
	tty->termios->c_line = ldisc;
	if (tty->ldisc.open)
		retval = (tty->ldisc.open)(tty);
	if (retval < 0) {
		tty->ldisc = o_ldisc;
		tty->termios->c_line = tty->ldisc.num;
		if (tty->ldisc.open && (tty->ldisc.open(tty) < 0)) {
			tty->ldisc = ldiscs[N_TTY];
			tty->termios->c_line = N_TTY;
			if (tty->ldisc.open) {
				int r = tty->ldisc.open(tty);

				if (r < 0)
					panic("Couldn't open N_TTY ldisc for "
					      "%s --- error %d.",
					      tty_name(tty), r);
			}
		}
	}
	if (tty->ldisc.num != o_ldisc.num && tty->driver.set_ldisc)
		tty->driver.set_ldisc(tty);
	return retval;
}

/*
 * This routine returns a tty driver structure, given a device number
 */
struct tty_driver *get_tty_driver(kdev_t device)
{
	int	major, minor;
	struct tty_driver *p;
	
	minor = MINOR(device);
	major = MAJOR(device);

	for (p = tty_drivers; p; p = p->next) {
		if (p->major != major)
			continue;
		if (minor < p->minor_start)
			continue;
		if (minor >= p->minor_start + p->num)
			continue;
		return p;
	}
	return NULL;
}

/*
 * If we try to write to, or set the state of, a terminal and we're
 * not in the foreground, send a SIGTTOU.  If the signal is blocked or
 * ignored, go ahead and perform the operation.  (POSIX 7.2)
 */
int tty_check_change(struct tty_struct * tty)
{
	if (current->tty != tty)
		return 0;
	if (tty->pgrp <= 0) {
		printk("tty_check_change: tty->pgrp <= 0!\n");
		return 0;
	}
	if (current->pgrp == tty->pgrp)
		return 0;
	if (is_ignored(SIGTTOU))
		return 0;
	if (is_orphaned_pgrp(current->pgrp))
		return -EIO;
	(void) kill_pg(current->pgrp,SIGTTOU,1);
	return -ERESTARTSYS;
}

static int hung_up_tty_read(struct inode * inode, struct file * file, char * buf, int count)
{
	return 0;
}

static int hung_up_tty_write(struct inode * inode, struct file * file, const char * buf, int count)
{
	return -EIO;
}

static int hung_up_tty_select(struct inode * inode, struct file * filp, int sel_type, select_table * wait)
{
	return 1;
}

static int hung_up_tty_ioctl(struct inode * inode, struct file * file,
			     unsigned int cmd, unsigned long arg)
{
	return cmd == TIOCSPGRP ? -ENOTTY : -EIO;
}

static int tty_lseek(struct inode * inode, struct file * file, off_t offset, int orig)
{
	return -ESPIPE;
}

static struct file_operations tty_fops = {
	tty_lseek,
	tty_read,
	tty_write,
	NULL,		/* tty_readdir */
	tty_select,
	tty_ioctl,
	NULL,		/* tty_mmap */
	tty_open,
	tty_release,
	NULL,		/* tty_fsync */
	tty_fasync
};

static struct file_operations hung_up_tty_fops = {
	tty_lseek,
	hung_up_tty_read,
	hung_up_tty_write,
	NULL,		/* hung_up_tty_readdir */
	hung_up_tty_select,
	hung_up_tty_ioctl,
	NULL,		/* hung_up_tty_mmap */
	NULL,		/* hung_up_tty_open */
	tty_release,	/* hung_up_tty_release */
	NULL,		/* hung_up_tty_fsync  */
	NULL		/* hung_up_tty_fasync */
};

void do_tty_hangup(struct tty_struct * tty, struct file_operations *fops)
{
	int i;
	struct file * filp;
	struct task_struct *p;

	if (!tty)
		return;
	check_tty_count(tty, "do_tty_hangup");
	for (filp = first_file, i=0; i<nr_files; i++, filp = filp->f_next) {
		if (!filp->f_count)
			continue;
		if (filp->private_data != tty)
			continue;
		if (!filp->f_inode)
			continue;
		if (filp->f_inode->i_rdev == CONSOLE_DEV)
			continue;
		if (filp->f_op != &tty_fops)
			continue;
		tty_fasync(filp->f_inode, filp, 0);
		filp->f_op = fops;
	}
	
	if (tty->ldisc.flush_buffer)
		tty->ldisc.flush_buffer(tty);
	if (tty->driver.flush_buffer)
		tty->driver.flush_buffer(tty);
	if ((tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
	    tty->ldisc.write_wakeup)
		(tty->ldisc.write_wakeup)(tty);
	wake_up_interruptible(&tty->write_wait);
	wake_up_interruptible(&tty->read_wait);

	/*
	 * Shutdown the current line discipline, and reset it to
	 * N_TTY.
	 */
	if (tty->ldisc.num != ldiscs[N_TTY].num) {
		if (tty->ldisc.close)
			(tty->ldisc.close)(tty);
		tty->ldisc = ldiscs[N_TTY];
		tty->termios->c_line = N_TTY;
		if (tty->ldisc.open) {
			i = (tty->ldisc.open)(tty);
			if (i < 0)
				printk("do_tty_hangup: N_TTY open: error %d\n",
				       -i);
		}
	}
	
 	for_each_task(p) {
		if ((tty->session > 0) && (p->session == tty->session) &&
		    p->leader) {
			send_sig(SIGHUP,p,1);
			send_sig(SIGCONT,p,1);
			if (tty->pgrp > 0)
				p->tty_old_pgrp = tty->pgrp;
		}
		if (p->tty == tty)
			p->tty = NULL;
	}
	tty->flags = 0;
	tty->session = 0;
	tty->pgrp = -1;
	tty->ctrl_status = 0;
	if (tty->driver.flags & TTY_DRIVER_RESET_TERMIOS)
		*tty->termios = tty->driver.init_termios;
	if (tty->driver.hangup)
		(tty->driver.hangup)(tty);
}

void tty_hangup(struct tty_struct * tty)
{
#ifdef TTY_DEBUG_HANGUP
	printk("%s hangup...\n", tty_name(tty));
#endif
	do_tty_hangup(tty, &hung_up_tty_fops);
}

void tty_vhangup(struct tty_struct * tty)
{
#ifdef TTY_DEBUG_HANGUP
	printk("%s vhangup...\n", tty_name(tty));
#endif
	do_tty_hangup(tty, &hung_up_tty_fops);
}

int tty_hung_up_p(struct file * filp)
{
	return (filp->f_op == &hung_up_tty_fops);
}

/*
 * This function is typically called only by the session leader, when
 * it wants to disassociate itself from its controlling tty.
 *
 * It performs the following functions:
 * 	(1)  Sends a SIGHUP and SIGCONT to the foreground process group
 * 	(2)  Clears the tty from being controlling the session
 * 	(3)  Clears the controlling tty for all processes in the
 * 		session group.
 *
 * The argument on_exit is set to 1 if called when a process is
 * exiting; it is 0 if called by the ioctl TIOCNOTTY.
 */
void disassociate_ctty(int on_exit)
{
	struct tty_struct *tty = current->tty;
	struct task_struct *p;
	int tty_pgrp = -1;

	if (tty) {
		tty_pgrp = tty->pgrp;
		if (on_exit && tty->driver.type != TTY_DRIVER_TYPE_PTY)
			tty_vhangup(tty);
	} else {
		if (current->tty_old_pgrp) {
			kill_pg(current->tty_old_pgrp, SIGHUP, on_exit);
			kill_pg(current->tty_old_pgrp, SIGCONT, on_exit);
		}
		return;
	}
	if (tty_pgrp > 0) {
		kill_pg(tty_pgrp, SIGHUP, on_exit);
		if (!on_exit)
			kill_pg(tty_pgrp, SIGCONT, on_exit);
	}

	current->tty_old_pgrp = 0;
	tty->session = 0;
	tty->pgrp = -1;

	for_each_task(p)
	  	if (p->session == current->session)
			p->tty = NULL;
}

#ifdef CONFIG_CONSOLE
/*
 * Sometimes we want to wait until a particular VT has been activated. We
 * do it in a very simple manner. Everybody waits on a single queue and
 * get woken up at once. Those that are satisfied go on with their business,
 * while those not ready go back to sleep. Seems overkill to add a wait
 * to each vt just for this - usually this does nothing!
 */
static struct wait_queue *vt_activate_queue = NULL;

/*
 * Sleeps until a vt is activated, or the task is interrupted. Returns
 * 0 if activation, -1 if interrupted.
 */
int vt_waitactive(void)
{
	interruptible_sleep_on(&vt_activate_queue);
	return (current->signal & ~current->blocked) ? -1 : 0;
}

#define vt_wake_waitactive() wake_up(&vt_activate_queue)

void reset_vc(unsigned int new_console)
{
	vt_cons[new_console]->vc_mode = KD_TEXT;
	kbd_table[new_console].kbdmode = VC_XLATE;
	vt_cons[new_console]->vt_mode.mode = VT_AUTO;
	vt_cons[new_console]->vt_mode.waitv = 0;
	vt_cons[new_console]->vt_mode.relsig = 0;
	vt_cons[new_console]->vt_mode.acqsig = 0;
	vt_cons[new_console]->vt_mode.frsig = 0;
	vt_cons[new_console]->vt_pid = -1;
	vt_cons[new_console]->vt_newvt = -1;
	reset_palette (new_console) ;
}

/*
 * Performs the back end of a vt switch
 */
void complete_change_console(unsigned int new_console)
{
	unsigned char old_vc_mode;

        if ((new_console == fg_console) || (vt_dont_switch))
                return;
        if (!vc_cons_allocated(new_console))
                return;
	last_console = fg_console;

	/*
	 * If we're switching, we could be going from KD_GRAPHICS to
	 * KD_TEXT mode or vice versa, which means we need to blank or
	 * unblank the screen later.
	 */
	old_vc_mode = vt_cons[fg_console]->vc_mode;
	update_screen(new_console);

	/*
	 * If this new console is under process control, send it a signal
	 * telling it that it has acquired. Also check if it has died and
	 * clean up (similar to logic employed in change_console())
	 */
	if (vt_cons[new_console]->vt_mode.mode == VT_PROCESS)
	{
		/*
		 * Send the signal as privileged - kill_proc() will
		 * tell us if the process has gone or something else
		 * is awry
		 */
		if (kill_proc(vt_cons[new_console]->vt_pid,
			      vt_cons[new_console]->vt_mode.acqsig,
			      1) != 0)
		{
		/*
		 * The controlling process has died, so we revert back to
		 * normal operation. In this case, we'll also change back
		 * to KD_TEXT mode. I'm not sure if this is strictly correct
		 * but it saves the agony when the X server dies and the screen
		 * remains blanked due to KD_GRAPHICS! It would be nice to do
		 * this outside of VT_PROCESS but there is no single process
		 * to account for and tracking tty count may be undesirable.
		 */
		        reset_vc(new_console);
		}
	}

	/*
	 * We do this here because the controlling process above may have
	 * gone, and so there is now a new vc_mode
	 */
	if (old_vc_mode != vt_cons[new_console]->vc_mode)
	{
		if (vt_cons[new_console]->vc_mode == KD_TEXT)
			do_unblank_screen();
		else
			do_blank_screen(1);
	}

	/* Set the colour palette for this VT */
	if (vt_cons[new_console]->vc_mode == KD_TEXT)
		set_palette() ;
	
	/*
	 * Wake anyone waiting for their VT to activate
	 */
	vt_wake_waitactive();
	return;
}

/*
 * Performs the front-end of a vt switch
 */
void change_console(unsigned int new_console)
{
        if ((new_console == fg_console) || (vt_dont_switch))
                return;
        if (!vc_cons_allocated(new_console))
		return;

	/*
	 * If this vt is in process mode, then we need to handshake with
	 * that process before switching. Essentially, we store where that
	 * vt wants to switch to and wait for it to tell us when it's done
	 * (via VT_RELDISP ioctl).
	 *
	 * We also check to see if the controlling process still exists.
	 * If it doesn't, we reset this vt to auto mode and continue.
	 * This is a cheap way to track process control. The worst thing
	 * that can happen is: we send a signal to a process, it dies, and
	 * the switch gets "lost" waiting for a response; hopefully, the
	 * user will try again, we'll detect the process is gone (unless
	 * the user waits just the right amount of time :-) and revert the
	 * vt to auto control.
	 */
	if (vt_cons[fg_console]->vt_mode.mode == VT_PROCESS)
	{
		/*
		 * Send the signal as privileged - kill_proc() will
		 * tell us if the process has gone or something else
		 * is awry
		 */
		if (kill_proc(vt_cons[fg_console]->vt_pid,
			      vt_cons[fg_console]->vt_mode.relsig,
			      1) == 0)
		{
			/*
			 * It worked. Mark the vt to switch to and
			 * return. The process needs to send us a
			 * VT_RELDISP ioctl to complete the switch.
			 */
			vt_cons[fg_console]->vt_newvt = new_console;
			return;
		}

		/*
		 * The controlling process has died, so we revert back to
		 * normal operation. In this case, we'll also change back
		 * to KD_TEXT mode. I'm not sure if this is strictly correct
		 * but it saves the agony when the X server dies and the screen
		 * remains blanked due to KD_GRAPHICS! It would be nice to do
		 * this outside of VT_PROCESS but there is no single process
		 * to account for and tracking tty count may be undesirable.
		 */
		reset_vc(fg_console);

		/*
		 * Fall through to normal (VT_AUTO) handling of the switch...
		 */
	}

	/*
	 * Ignore all switches in KD_GRAPHICS+VT_AUTO mode
	 */
	if (vt_cons[fg_console]->vc_mode == KD_GRAPHICS)
		return;

	complete_change_console(new_console);
}
#endif /*CONFIG_CONSOLE*/

void wait_for_keypress(void)
{
	sleep_on(&keypress_wait);
}

void stop_tty(struct tty_struct *tty)
{
	if (tty->stopped)
		return;
	tty->stopped = 1;
	if (tty->link && tty->link->packet) {
		tty->ctrl_status &= ~TIOCPKT_START;
		tty->ctrl_status |= TIOCPKT_STOP;
		wake_up_interruptible(&tty->link->read_wait);
	}
	if (tty->driver.stop)
		(tty->driver.stop)(tty);
}

void start_tty(struct tty_struct *tty)
{
	if (!tty->stopped)
		return;
	tty->stopped = 0;
	if (tty->link && tty->link->packet) {
		tty->ctrl_status &= ~TIOCPKT_STOP;
		tty->ctrl_status |= TIOCPKT_START;
		wake_up_interruptible(&tty->link->read_wait);
	}
	if (tty->driver.start)
		(tty->driver.start)(tty);
	if ((tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
	    tty->ldisc.write_wakeup)
		(tty->ldisc.write_wakeup)(tty);
	wake_up_interruptible(&tty->write_wait);
}

static int tty_read(struct inode * inode, struct file * file, char * buf, int count)
{
	int i;
	struct tty_struct * tty;

	tty = (struct tty_struct *)file->private_data;
	if (tty_paranoia_check(tty, inode->i_rdev, "tty_read"))
		return -EIO;
	if (!tty || (tty->flags & (1 << TTY_IO_ERROR)))
		return -EIO;

	/* This check not only needs to be done before reading, but also
	   whenever read_chan() gets woken up after sleeping, so I've
	   moved it to there.  This should only be done for the N_TTY
	   line discipline, anyway.  Same goes for write_chan(). -- jlc. */
#if 0
	if ((inode->i_rdev != CONSOLE_DEV) && /* don't stop on /dev/console */
	    (tty->pgrp > 0) &&
	    (current->tty == tty) &&
	    (tty->pgrp != current->pgrp))
		if (is_ignored(SIGTTIN) || is_orphaned_pgrp(current->pgrp))
			return -EIO;
		else {
			(void) kill_pg(current->pgrp, SIGTTIN, 1);
			return -ERESTARTSYS;
		}
#endif
	if (tty->ldisc.read)
		/* XXX casts are for what kernel-wide prototypes should be. */
		i = (tty->ldisc.read)(tty,file,(unsigned char *)buf,(unsigned int)count);
	else
		i = -EIO;
	if (i > 0)
		inode->i_atime = CURRENT_TIME;
	return i;
}

/*
 * Split writes up in sane blocksizes to avoid
 * denial-of-service type attacks
 */
static inline int do_tty_write(
	int (*write)(struct tty_struct *, struct file *, const unsigned char *, unsigned int),
	struct inode *inode,
	struct tty_struct *tty,
	struct file *file,
	const unsigned char *buf,
	unsigned int count)
{
	int ret = 0, written = 0;

	for (;;) {
		unsigned int size = PAGE_SIZE*2;
		if (size > count)
			size = count;
		ret = write(tty, file, buf, size);
		if (ret <= 0)
			break;
		written += ret;
		buf += ret;
		count -= ret;
		if (!count)
			break;
		ret = -ERESTARTSYS;
		if (current->signal & ~current->blocked)
			break;
		if (need_resched)
			schedule();
	}
	if (written) {
		inode->i_mtime = CURRENT_TIME;
		ret = written;
	}
	return ret;
}


static int tty_write(struct inode * inode, struct file * file, const char * buf, int count)
{
	int is_console;
	struct tty_struct * tty;

	is_console = (inode->i_rdev == CONSOLE_DEV);

	if (is_console && redirect)
		tty = redirect;
	else
		tty = (struct tty_struct *)file->private_data;
	if (tty_paranoia_check(tty, inode->i_rdev, "tty_write"))
		return -EIO;
	if (!tty || !tty->driver.write || (tty->flags & (1 << TTY_IO_ERROR)))
		return -EIO;
#if 0
	if (!is_console && L_TOSTOP(tty) && (tty->pgrp > 0) &&
	    (current->tty == tty) && (tty->pgrp != current->pgrp)) {
		if (is_orphaned_pgrp(current->pgrp))
			return -EIO;
		if (!is_ignored(SIGTTOU)) {
			(void) kill_pg(current->pgrp, SIGTTOU, 1);
			return -ERESTARTSYS;
		}
	}
#endif
	if (!tty->ldisc.write)
		return -EIO;
	return do_tty_write(tty->ldisc.write,
		inode, tty, file,
		(const unsigned char *)buf,
		(unsigned int)count);
}

/* Semaphore to protect creating and releasing a tty */
static struct semaphore tty_sem = MUTEX;
static void down_tty_sem(int index)
{
	down(&tty_sem);
}
static void up_tty_sem(int index)
{
	up(&tty_sem);
}
static void release_mem(struct tty_struct *tty, int idx);

/*
 * Rewritten to remove races and properly clean up after a failed open.  
 * The new code protects the open with a semaphore, so it's really 
 * quite straightforward.  The semaphore locking can probably be
 * relaxed for the (most common) case of reopening a tty.
 */
static int init_dev(kdev_t device, struct tty_struct **ret_tty)
{
	struct tty_struct *tty, *o_tty;
	struct termios *tp, **tp_loc, *o_tp, **o_tp_loc;
	struct termios *ltp, **ltp_loc, *o_ltp, **o_ltp_loc;
	struct tty_driver *driver;	
	int retval;
	int idx;

	driver = get_tty_driver(device);

	if (!driver)
		return -ENODEV;

	idx = MINOR(device) - driver->minor_start;


	/* 
	 * Check whether we need to acquire the tty semaphore to avoid
	 * race conditions.  For now, play it safe.
	 */
	down_tty_sem(idx);

	/* check whether we're reopening an existing tty */
	tty = driver->table[idx];
	if(tty) goto fast_track;

	/*
	 * First time open is complex, especially for PTY devices.
	 * This code guarantees that either everything succeeds and the
	 * TTY is ready for operation, or else the table slots are vacated
	 * and the allocated memory released.  (Except that the termios 
	 * and locked termios may be retained.)
	 */

	o_tty = NULL;
	tp = o_tp = NULL;
	ltp = o_ltp = NULL;

	tty = (struct tty_struct*) get_free_page(GFP_KERNEL);
	if(!tty)
		goto fail_no_mem;
	initialize_tty_struct(tty);
	tty->device = device;
	tty->driver = *driver;

	tp_loc = &driver->termios[idx];
	if (!*tp_loc) {
		tp = (struct termios *) kmalloc(sizeof(struct termios),
						GFP_KERNEL);
		if (!tp)
			goto free_mem_out;
		*tp = driver->init_termios;
	}

	ltp_loc = &driver->termios_locked[idx];
	if (!*ltp_loc) {
		ltp = (struct termios *) kmalloc(sizeof(struct termios),
						 GFP_KERNEL);
		if (!ltp)
			goto free_mem_out;
		memset(ltp, 0, sizeof(struct termios));
	}

	if (driver->type == TTY_DRIVER_TYPE_PTY) {
		o_tty = (struct tty_struct *) get_free_page(GFP_KERNEL);
		if (!o_tty)
			goto free_mem_out;
		initialize_tty_struct(o_tty);
		o_tty->device = (kdev_t) MKDEV(driver->other->major,
					driver->other->minor_start + idx);
		o_tty->driver = *driver->other;

		o_tp_loc  = &driver->other->termios[idx];
		if (!*o_tp_loc) {
			o_tp = (struct termios *)
				kmalloc(sizeof(struct termios), GFP_KERNEL);
			if (!o_tp)
				goto free_mem_out;
			*o_tp = driver->other->init_termios;
		}

		o_ltp_loc = &driver->other->termios_locked[idx];
		if (!*o_ltp_loc) {
			o_ltp = (struct termios *)
				kmalloc(sizeof(struct termios), GFP_KERNEL);
			if (!o_ltp)
				goto free_mem_out;
			memset(o_ltp, 0, sizeof(struct termios));
		}

		/*
		 * Everything allocated ... set up the o_tty structure.
		 */
		driver->other->table[idx] = o_tty;
		if (!*o_tp_loc)
			*o_tp_loc = o_tp;
		if (!*o_ltp_loc)
			*o_ltp_loc = o_ltp;
		o_tty->termios = *o_tp_loc;
		o_tty->termios_locked = *o_ltp_loc;
		(*driver->other->refcount)++;
		if (driver->subtype == PTY_TYPE_MASTER)
			o_tty->count++;

		/* Establish the links in both directions */
		tty->link   = o_tty;
		o_tty->link = tty;
	}

	/* 
	 * All structures have been allocated, so now we install them.
	 * Failures after this point use release_mem to clean up, so 
	 * there's no need to null out the local pointers.
	 */
	driver->table[idx] = tty;	/* FIXME: this is broken and
	probably causes ^D bug. tty->private_date does not (yet) point
	to a console, if keypress comes now, await armagedon. 

	also, driver->table is accessed from interrupt for vt case,
	and this does not look like atomic access at all. */
	
	if (!*tp_loc)
		*tp_loc = tp;
	if (!*ltp_loc)
		*ltp_loc = ltp;
	tty->termios = *tp_loc;
	tty->termios_locked = *ltp_loc;
	(*driver->refcount)++;
	tty->count++;

	/* 
	 * Structures all installed ... call the ldisc open routines.
	 * If we fail here just call release_mem to clean up.  No need
	 * to decrement the use counts, as release_mem doesn't care.
	 */
	if (tty->ldisc.open) {
		retval = (tty->ldisc.open)(tty);
		if (retval)
			goto release_mem_out;
	}
	if (o_tty && o_tty->ldisc.open) {
		retval = (o_tty->ldisc.open)(o_tty);
		if (retval) {
			if (tty->ldisc.close)
				(tty->ldisc.close)(tty);
			goto release_mem_out;
		}
	}
	goto success;

	/*
	 * This fast open can be used if the tty is already open.
	 * No memory is allocated, and the only failures are from
	 * attempting to open a closing tty or attempting multiple
	 * opens on a pty master.
	 */
fast_track:
	retval = -EIO;
	if (test_bit(TTY_CLOSING, &tty->flags)) 
		goto end_init;
	if (driver->type == TTY_DRIVER_TYPE_PTY &&
	    driver->subtype == PTY_TYPE_MASTER) {
		/*
		 * special case for PTY masters: only one open permitted, 
		 * and the slave side open count is incremented as well.
		 */
		if (tty->count)
			goto end_init;
		tty->link->count++;
	}
	tty->count++;
	tty->driver = *driver; /* N.B. why do this every time?? */

success:
	retval = 0;
	*ret_tty = tty;
	
	/* All paths come through here to release the semaphore */
end_init:
	up_tty_sem(idx);
	return retval;

	/* Release locally allocated memory ... nothing placed in slots */
free_mem_out:
	if (o_tp)
		kfree_s(o_tp, sizeof(struct termios));
	if (o_tty)
		free_page((unsigned long) o_tty);
	if (ltp)
		kfree_s(ltp, sizeof(struct termios));
	if (tp)
		kfree_s(tp, sizeof(struct termios));
	free_page((unsigned long) tty);

fail_no_mem:
	retval = -ENOMEM;
	goto end_init;

	/* call the tty release_mem routine to clean out this slot */
release_mem_out:
	printk("init_dev: ldisc open failed, clearing slot %d\n", idx);
	release_mem(tty, idx);
	goto end_init;
}

/*
 * Releases memory associated with a tty structure, and clears out the
 * driver table slots.
 */
static void release_mem(struct tty_struct *tty, int idx)
{
	struct tty_struct *o_tty;
	struct termios *tp;

	if ((o_tty = tty->link) != NULL) {
		o_tty->driver.table[idx] = NULL;
		if (o_tty->driver.flags & TTY_DRIVER_RESET_TERMIOS) {
			tp = o_tty->driver.termios[idx];
			o_tty->driver.termios[idx] = NULL;
			kfree_s(tp, sizeof(struct termios));
		}
		o_tty->magic = 0;
		(*o_tty->driver.refcount)--;
		free_page((unsigned long) o_tty);
	}

	tty->driver.table[idx] = NULL;
	if (tty->driver.flags & TTY_DRIVER_RESET_TERMIOS) {
		tp = tty->driver.termios[idx];
		tty->driver.termios[idx] = NULL;
		kfree_s(tp, sizeof(struct termios));
	}
	tty->magic = 0;
	(*tty->driver.refcount)--;
	free_page((unsigned long) tty);
}

/*
 * Even releasing the tty structures is a tricky business.. We have
 * to be very careful that the structures are all released at the
 * same time, as interrupts might otherwise get the wrong pointers.
 */
static void release_dev(struct file * filp)
{
	struct tty_struct *tty, *o_tty;
	int	pty_master, tty_closing, o_tty_closing, do_sleep;
	int	idx;
	
	tty = (struct tty_struct *)filp->private_data;
	if (tty_paranoia_check(tty, filp->f_inode->i_rdev, "release_dev"))
		return;

	check_tty_count(tty, "release_dev");

	tty_fasync(filp->f_inode, filp, 0);

	idx = MINOR(tty->device) - tty->driver.minor_start;
	pty_master = (tty->driver.type == TTY_DRIVER_TYPE_PTY &&
		      tty->driver.subtype == PTY_TYPE_MASTER);
	o_tty = tty->link;

#ifdef TTY_PARANOIA_CHECK
	if (idx < 0 || idx >= tty->driver.num) {
		printk("release_dev: bad idx when trying to free (%s)\n",
		       kdevname(tty->device));
		return;
	}
	if (tty != tty->driver.table[idx]) {
		printk("release_dev: driver.table[%d] not tty for (%s)\n",
		       idx, kdevname(tty->device));
		return;
	}
	if (tty->termios != tty->driver.termios[idx]) {
		printk("release_dev: driver.termios[%d] not termios "
		       "for (%s)\n",
		       idx, kdevname(tty->device));
		return;
	}
	if (tty->termios_locked != tty->driver.termios_locked[idx]) {
		printk("release_dev: driver.termios_locked[%d] not "
		       "termios_locked for (%s)\n",
		       idx, kdevname(tty->device));
		return;
	}
#endif

#ifdef TTY_DEBUG_HANGUP
	printk("release_dev of %s (tty count=%d)...", tty_name(tty),
	       tty->count);
#endif

#ifdef TTY_PARANOIA_CHECK
	if (tty->driver.other) {
		if (o_tty != tty->driver.other->table[idx]) {
			printk("release_dev: other->table[%d] not o_tty for ("
			       "%s)\n",
			       idx, kdevname(tty->device));
			return;
		}
		if (o_tty->termios != tty->driver.other->termios[idx]) {
			printk("release_dev: other->termios[%d] not o_termios "
			       "for (%s)\n",
			       idx, kdevname(tty->device));
			return;
		}
		if (o_tty->termios_locked != 
		      tty->driver.other->termios_locked[idx]) {
			printk("release_dev: other->termios_locked[%d] not "
			       "o_termios_locked for (%s)\n",
			       idx, kdevname(tty->device));
			return;
		}
		if (o_tty->link != tty) {
			printk("release_dev: bad pty pointers\n");
			return;
		}
	}
#endif

	if (tty->driver.close)
		tty->driver.close(tty, filp);

	/*
	 * Sanity check: if tty->count is going to zero, there shouldn't be
	 * any waiters on tty->read_wait or tty->write_wait.  We test the
	 * wait queues and kick everyone out _before_ actually starting to
	 * close.  This ensures that we won't block while releasing the tty
	 * structure.
	 *
	 * The test for the o_tty closing is necessary, since the master and
	 * slave sides may close in any order.  If the slave side closes out
	 * first, its count will be one, since the master side holds an open.
	 * Thus this test wouldn't be triggered at the time the slave closes,
	 * so we do it now.
	 *
	 * Note that it's possible for the tty to be opened again while we're
	 * flushing out waiters.  By recalculating the closing flags before
	 * each iteration we avoid any problems.
	 */
	while (1) {
		tty_closing = tty->count <= 1;
		o_tty_closing = o_tty &&
			(o_tty->count <= (pty_master ? 1 : 0));
		do_sleep = 0;

		if (tty_closing) {
			if (waitqueue_active(&tty->read_wait)) {
				wake_up(&tty->read_wait);
				do_sleep++;
			}
			if (waitqueue_active(&tty->write_wait)) {
				wake_up(&tty->write_wait);
				do_sleep++;
			}
		}
		if (o_tty_closing) {
			if (waitqueue_active(&o_tty->read_wait)) {
				wake_up(&o_tty->read_wait);
				do_sleep++;
			}
			if (waitqueue_active(&o_tty->write_wait)) {
				wake_up(&o_tty->write_wait);
				do_sleep++;
			}
		}
		if (!do_sleep)
			break;

		printk("release_dev: %s: read/write wait queue active!\n",
		       tty_name(tty));
		schedule();
	}	

	/*
	 * The closing flags are now consistent with the open counts on 
	 * both sides, and we've completed the last operation that could 
	 * block, so it's safe to proceed with closing.
	 */

	if (pty_master) {
		if (--o_tty->count < 0) {
			printk("release_dev: bad pty slave count (%d) for %s\n",
			       o_tty->count, tty_name(o_tty));
			o_tty->count = 0;
		}
	}
	if (--tty->count < 0) {
		printk("release_dev: bad tty->count (%d) for %s\n",
		       tty->count, tty_name(tty));
		tty->count = 0;
	}

	/*
	 * Perform some housekeeping before deciding whether to return.
	 *
	 * Set the TTY_CLOSING flag if this was the last open.  In the
	 * case of a pty we may have to wait around for the other side
	 * to close, and TTY_CLOSING makes sure we can't be reopened.
	 */
	if(tty_closing)
		set_bit(TTY_CLOSING, &tty->flags);
	if(o_tty_closing)
		set_bit(TTY_CLOSING, &o_tty->flags);

	/*
	 * If _either_ side is closing, make sure there aren't any
	 * processes that still think tty or o_tty is their controlling
	 * tty.  Also, clear redirect if it points to either tty.
	 */
	if (tty_closing || o_tty_closing) {
		struct task_struct *p;

		for_each_task(p) {
			if (p->tty == tty || (o_tty && p->tty == o_tty))
				p->tty = NULL;
		}

		if (redirect == tty || (o_tty && redirect == o_tty))
			redirect = NULL;
	}

	/* check whether both sides are closing ... */
	if (!tty_closing || (o_tty && !o_tty_closing))
		return;
	filp->private_data = 0;
	
#ifdef TTY_DEBUG_HANGUP
	printk("freeing tty structure...");
#endif

	/*
	 * Shutdown the current line discipline, and reset it to N_TTY.
	 * N.B. why reset ldisc when we're releasing the memory??
	 */
	if (tty->ldisc.close)
		(tty->ldisc.close)(tty);
	tty->ldisc = ldiscs[N_TTY];
	tty->termios->c_line = N_TTY;
	if (o_tty) {
		if (o_tty->ldisc.close)
			(o_tty->ldisc.close)(o_tty);
		o_tty->ldisc = ldiscs[N_TTY];
	}
	
	/*
	 * Make sure that the tty's task queue isn't activated.  If it
	 * is, take it out of the linked list.  The tqueue isn't used by
	 * pty's, so skip the test for them.
	 */
	if (tty->driver.type != TTY_DRIVER_TYPE_PTY) {
		cli();
		if (tty->flip.tqueue.sync) {
			struct tq_struct *tq, *prev;

			for (tq=tq_timer, prev=0; tq; prev=tq, tq=tq->next) {
				if (tq == &tty->flip.tqueue) {
					if (prev)
						prev->next = tq->next;
					else
						tq_timer = tq->next;
					break;
				}
			}
		}
		sti();
	}

	/* 
	 * The release_mem function takes care of the details of clearing
	 * the slots and preserving the termios structure.
	 */
	release_mem(tty, idx);
}

/*
 * tty_open and tty_release keep up the tty count that contains the
 * number of opens done on a tty. We cannot use the inode-count, as
 * different inodes might point to the same tty.
 *
 * Open-counting is needed for pty masters, as well as for keeping
 * track of serial lines: DTR is dropped when the last close happens.
 * (This is not done solely through tty->count, now.  - Ted 1/27/92)
 *
 * The termios state of a pty is reset on first open so that
 * settings don't persist across reuse.
 */
static int tty_open(struct inode * inode, struct file * filp)
{
	struct tty_struct *tty;
	int minor;
	int noctty, retval;
	kdev_t device;

retry_open:
	noctty = filp->f_flags & O_NOCTTY;
	device = inode->i_rdev;
	if (device == TTY_DEV) {
		if (!current->tty)
			return -ENXIO;
		device = current->tty->device;
		/* noctty = 1; */
	}
	if (device == CONSOLE_DEV) {
		device = MKDEV(TTY_MAJOR, fg_console+1);
		noctty = 1;
	}
	minor = MINOR(device);
	retval = init_dev(device, &tty);
	if (retval)
		return retval;
	filp->private_data = tty;
	check_tty_count(tty, "tty_open");
	if (tty->driver.type == TTY_DRIVER_TYPE_PTY &&
	    tty->driver.subtype == PTY_TYPE_MASTER)
		noctty = 1;
#ifdef TTY_DEBUG_HANGUP
	printk("opening %s...", tty_name(tty));
#endif
	if (tty->driver.open)
		retval = tty->driver.open(tty, filp);
	else
		retval = -ENODEV;
	if (!retval && test_bit(TTY_EXCLUSIVE, &tty->flags) && !suser())
		retval = -EBUSY;

	if (retval) {
#ifdef TTY_DEBUG_HANGUP
		printk("error %d in opening %s...", retval, tty_name(tty));
#endif

		release_dev(filp);
		if (retval != -ERESTARTSYS)
			return retval;
		if (current->signal & ~current->blocked)
			return retval;
		schedule();
		/*
		 * Need to reset f_op in case a hangup happened.
		 */
		filp->f_op = &tty_fops;
		goto retry_open;
	}
	if (!noctty &&
	    current->leader &&
	    !current->tty &&
	    tty->session == 0) {
		current->tty = tty;
		current->tty_old_pgrp = 0;
		tty->session = current->session;
		tty->pgrp = current->pgrp;
	}
	return 0;
}

static void tty_release(struct inode * inode, struct file * filp)
{
	release_dev(filp);
}

static int tty_select(struct inode * inode, struct file * filp, int sel_type, select_table * wait)
{
	struct tty_struct * tty;

	tty = (struct tty_struct *)filp->private_data;
	if (tty_paranoia_check(tty, inode->i_rdev, "tty_select"))
		return 0;

	if (tty->ldisc.select)
		return (tty->ldisc.select)(tty, inode, filp, sel_type, wait);
	return 0;
}

/*
 * fasync_helper() is used by some character device drivers (mainly mice)
 * to set up the fasync queue. It returns negative on error, 0 if it did
 * no changes and positive if it added/deleted the entry.
 */
int fasync_helper(struct inode * inode, struct file * filp, int on, struct fasync_struct **fapp)
{
	struct fasync_struct *fa, **fp;
	unsigned long flags;

	for (fp = fapp; (fa = *fp) != NULL; fp = &fa->fa_next) {
		if (fa->fa_file == filp)
			break;
	}

	if (on) {
		if (fa)
			return 0;
		fa = (struct fasync_struct *)kmalloc(sizeof(struct fasync_struct), GFP_KERNEL);
		if (!fa)
			return -ENOMEM;
		fa->magic = FASYNC_MAGIC;
		fa->fa_file = filp;
		save_flags(flags);
		cli();
		fa->fa_next = *fapp;
		*fapp = fa;
		restore_flags(flags);
		return 1;
	}
	if (!fa)
		return 0;
	save_flags(flags);
	cli();
	*fp = fa->fa_next;
	restore_flags(flags);
	kfree(fa);
	return 1;
}

static int tty_fasync(struct inode * inode, struct file * filp, int on)
{
	struct tty_struct * tty;
	int retval;

	tty = (struct tty_struct *)filp->private_data;
	if (tty_paranoia_check(tty, inode->i_rdev, "tty_fasync"))
		return 0;
	
	retval = fasync_helper(inode, filp, on, &tty->fasync);
	if (retval <= 0)
		return retval;

	if (on) {
		if (!waitqueue_active(&tty->read_wait))
			tty->minimum_to_wake = 1;
		if (filp->f_owner.pid == 0) {
			filp->f_owner.pid = (-tty->pgrp) ? : current->pid;
			filp->f_owner.uid = current->uid;
			filp->f_owner.euid = current->euid;
		}
	} else {
		if (!tty->fasync && !waitqueue_active(&tty->read_wait))
			tty->minimum_to_wake = N_TTY_BUF_SIZE;
	}
	return 0;
}

#if 0
/*
 * XXX does anyone use this anymore?!?
 */
static int do_get_ps_info(unsigned long arg)
{
	struct tstruct {
		int flag;
		int present[NR_TASKS];
		struct task_struct tasks[NR_TASKS];
	};
	struct tstruct *ts = (struct tstruct *)arg;
	struct task_struct **p;
	char *c, *d;
	int i, n = 0;
	
	i = verify_area(VERIFY_WRITE, (void *)arg, sizeof(struct tstruct));
	if (i)
		return i;
	for (p = &FIRST_TASK ; p <= &LAST_TASK ; p++, n++)
		if (*p)
		{
			c = (char *)(*p);
			d = (char *)(ts->tasks+n);
			for (i=0 ; i<sizeof(struct task_struct) ; i++)
				put_user(*c++, d++);
			put_user(1, ts->present+n);
		}
		else	
			put_user(0, ts->present+n);
	return(0);			
}
#endif

static int tty_ioctl(struct inode * inode, struct file * file,
		     unsigned int cmd, unsigned long arg)
{
	int	retval;
	struct tty_struct * tty;
	struct tty_struct * real_tty;
	struct winsize tmp_ws;
	pid_t pgrp;
	unsigned char	ch;
	char	mbz = 0;
	
	tty = (struct tty_struct *)file->private_data;
	if (tty_paranoia_check(tty, inode->i_rdev, "tty_ioctl"))
		return -EINVAL;

	if (tty->driver.type == TTY_DRIVER_TYPE_PTY &&
	    tty->driver.subtype == PTY_TYPE_MASTER)
		real_tty = tty->link;
	else
		real_tty = tty;

	switch (cmd) {
		case TIOCSTI:
			if ((current->tty != tty) && !suser())
				return -EPERM;
			retval = verify_area(VERIFY_READ, (void *) arg, 1);
			if (retval)
				return retval;
			ch = get_user((char *) arg);
			tty->ldisc.receive_buf(tty, &ch, &mbz, 1);
			return 0;
		case TIOCGWINSZ:
			retval = verify_area(VERIFY_WRITE, (void *) arg,
					     sizeof (struct winsize));
			if (retval)
				return retval;
			memcpy_tofs((struct winsize *) arg, &tty->winsize,
				    sizeof (struct winsize));
			return 0;
		case TIOCSWINSZ:
			retval = verify_area(VERIFY_READ, (void *) arg,
					     sizeof (struct winsize));
			if (retval)
				return retval;			
			memcpy_fromfs(&tmp_ws, (struct winsize *) arg,
				      sizeof (struct winsize));
			if (memcmp(&tmp_ws, &tty->winsize,
				   sizeof(struct winsize))) {
				if (tty->pgrp > 0)
					kill_pg(tty->pgrp, SIGWINCH, 1);
				if ((real_tty->pgrp != tty->pgrp) &&
				    (real_tty->pgrp > 0))
					kill_pg(real_tty->pgrp, SIGWINCH, 1);
			}
			tty->winsize = tmp_ws;
			real_tty->winsize = tmp_ws;
			return 0;
		case TIOCCONS:
			if (tty->driver.type == TTY_DRIVER_TYPE_CONSOLE) {
				if (!suser())
					return -EPERM;
				redirect = NULL;
				return 0;
			}
			if (redirect)
				return -EBUSY;
			redirect = real_tty;
			return 0;
		case FIONBIO:
			retval = verify_area(VERIFY_READ, (void *) arg, sizeof(int));
			if (retval)
				return retval;
			arg = get_user((unsigned int *) arg);
			if (arg)
				file->f_flags |= O_NONBLOCK;
			else
				file->f_flags &= ~O_NONBLOCK;
			return 0;
		case TIOCEXCL:
			set_bit(TTY_EXCLUSIVE, &tty->flags);
			return 0;
		case TIOCNXCL:
			clear_bit(TTY_EXCLUSIVE, &tty->flags);
			return 0;
		case TIOCNOTTY:
			if (current->tty != tty)
				return -ENOTTY;
			if (current->leader)
				disassociate_ctty(0);
			current->tty = NULL;
			return 0;
		case TIOCSCTTY:
			if (current->leader &&
			    (current->session == tty->session))
				return 0;
			/*
			 * The process must be a session leader and
			 * not have a controlling tty already.
			 */
			if (!current->leader || current->tty)
				return -EPERM;
			if (tty->session > 0) {
				/*
				 * This tty is already the controlling
				 * tty for another session group!
				 */
				if ((arg == 1) && suser()) {
					/*
					 * Steal it away
					 */
					struct task_struct *p;

					for_each_task(p)
						if (p->tty == tty)
							p->tty = NULL;
				} else
					return -EPERM;
			}
			current->tty = tty;
			current->tty_old_pgrp = 0;
			tty->session = current->session;
			tty->pgrp = current->pgrp;
			return 0;
		case TIOCGPGRP:
			/*
			 * (tty == real_tty) is a cheap way of
			 * testing if the tty is NOT a master pty.
			 */
			if (tty == real_tty && current->tty != real_tty)
				return -ENOTTY;
			retval = verify_area(VERIFY_WRITE, (void *) arg,
					     sizeof (pid_t));
			if (retval)
				return retval;
			put_user(real_tty->pgrp, (pid_t *) arg);
			return 0;
		case TIOCSPGRP:
			retval = tty_check_change(real_tty);
			if (retval == -EIO)
				return -ENOTTY;
			if (retval)
				return retval;
			if (!current->tty ||
			    (current->tty != real_tty) ||
			    (real_tty->session != current->session))
				return -ENOTTY;
			pgrp = get_user((pid_t *) arg);
			if (pgrp < 0)
				return -EINVAL;
			if (session_of_pgrp(pgrp) != current->session)
				return -EPERM;
			real_tty->pgrp = pgrp;
			return 0;
		case TIOCGETD:
			retval = verify_area(VERIFY_WRITE, (void *) arg,
					     sizeof (int));
			if (retval)
				return retval;
			put_user(tty->ldisc.num, (int *) arg);
			return 0;
		case TIOCSETD:
			retval = tty_check_change(tty);
			if (retval)
				return retval;
			retval = verify_area(VERIFY_READ, (void *) arg,
					     sizeof (int));
			if (retval)
				return retval;
			arg = get_user((int *) arg);
			return tty_set_ldisc(tty, arg);
#ifdef CONFIG_CONSOLE
		case TIOCLINUX:
			if (tty->driver.type != TTY_DRIVER_TYPE_CONSOLE)
				return -EINVAL;
			if (current->tty != tty && !suser())
				return -EPERM;
			retval = verify_area(VERIFY_READ, (void *) arg, 1);
			if (retval)
				return retval;
			switch (retval = get_user((char *)arg))
			{
				case 0:
				case 8:
				case 9:
					printk("TIOCLINUX (0/8/9) ioctl is gone - use /dev/vcs\n");
					return -EINVAL;
#if 0
				case 1:
					printk("Deprecated TIOCLINUX (1) ioctl\n");
					return do_get_ps_info(arg);
#endif
				case 2:
					return set_selection(arg, tty, 1);
				case 3:
					return paste_selection(tty);
				case 4:
					do_unblank_screen();
					return 0;
				case 5:
					return sel_loadlut(arg);
				case 6:
			/*
			 * Make it possible to react to Shift+Mousebutton.
			 * Note that 'shift_state' is an undocumented
			 * kernel-internal variable; programs not closely
			 * related to the kernel should not use this.
			 */
					retval = verify_area(VERIFY_WRITE, (void *) arg, 1);
					if (retval)
						return retval;
					put_user(shift_state,(char *) arg);
					return 0;
				case 7:
					retval = verify_area(VERIFY_WRITE, (void *) arg, 1);
					if (retval)
						return retval;
					put_user(mouse_reporting(),(char *) arg);
					return 0;
				case 10:
					set_vesa_blanking(arg);
					return 0;
				case 11:	/* set kmsg redirect */
					if (!suser())
						return -EPERM;
					retval = verify_area(VERIFY_READ,
						(void *) arg+1, 1);
					if (retval)
						return retval;
					kmsg_redirect = get_user((char *)arg+1);
					return 0;
				case 12:	/* get fg_console */
					return fg_console;
				default: 
					return -EINVAL;
			}
#endif /*CONFIG_CONSOLE*/

		case TIOCTTYGSTRUCT:
			retval = verify_area(VERIFY_WRITE, (void *) arg,
						sizeof(struct tty_struct));
			if (retval)
				return retval;
			memcpy_tofs((struct tty_struct *) arg,
				    tty, sizeof(struct tty_struct));
			return 0;
		default:
			if (tty->driver.ioctl) {
				retval = (tty->driver.ioctl)(tty, file,
							     cmd, arg);
				if (retval != -ENOIOCTLCMD)
					return retval;
			}
			if (tty->ldisc.ioctl) {
				retval = (tty->ldisc.ioctl)(tty, file,
							    cmd, arg);
				if (retval != -ENOIOCTLCMD)
					return retval;
			}
			return -EINVAL;
		}
}


/*
 * This implements the "Secure Attention Key" ---  the idea is to
 * prevent trojan horses by killing all processes associated with this
 * tty when the user hits the "Secure Attention Key".  Required for
 * super-paranoid applications --- see the Orange Book for more details.
 * 
 * This code could be nicer; ideally it should send a HUP, wait a few
 * seconds, then send a INT, and then a KILL signal.  But you then
 * have to coordinate with the init process, since all processes associated
 * with the current tty must be dead before the new getty is allowed
 * to spawn.
 */
void do_SAK( struct tty_struct *tty)
{
#ifdef TTY_SOFT_SAK
	tty_hangup(tty);
#else
	struct task_struct **p;
	int session;
	int		i;
	struct file	*filp;
	
	if (!tty)
		return;
	session  = tty->session;
	if (tty->ldisc.flush_buffer)
		tty->ldisc.flush_buffer(tty);
	if (tty->driver.flush_buffer)
		tty->driver.flush_buffer(tty);
 	for (p = &LAST_TASK ; p > &FIRST_TASK ; --p) {
		if (!(*p))
			continue;
		if (((*p)->tty == tty) ||
		    ((session > 0) && ((*p)->session == session)))
			send_sig(SIGKILL, *p, 1);
		else if ((*p)->files) {
			for (i=0; i < NR_OPEN; i++) {
				filp = (*p)->files->fd[i];
				if (filp && (filp->f_op == &tty_fops) &&
				    (filp->private_data == tty)) {
					send_sig(SIGKILL, *p, 1);
					break;
				}
			}
		}
	}
#endif
}

/*
 * This routine is called out of the software interrupt to flush data
 * from the flip buffer to the line discipline.
 */
static void flush_to_ldisc(void *private_)
{
	struct tty_struct *tty = (struct tty_struct *) private_;
	unsigned char	*cp;
	char		*fp;
	int		count;

	if (tty->flip.buf_num) {
		cp = tty->flip.char_buf + TTY_FLIPBUF_SIZE;
		fp = tty->flip.flag_buf + TTY_FLIPBUF_SIZE;
		tty->flip.buf_num = 0;

		cli();
		tty->flip.char_buf_ptr = tty->flip.char_buf;
		tty->flip.flag_buf_ptr = tty->flip.flag_buf;
	} else {
		cp = tty->flip.char_buf;
		fp = tty->flip.flag_buf;
		tty->flip.buf_num = 1;

		cli();
		tty->flip.char_buf_ptr = tty->flip.char_buf + TTY_FLIPBUF_SIZE;
		tty->flip.flag_buf_ptr = tty->flip.flag_buf + TTY_FLIPBUF_SIZE;
	}
	count = tty->flip.count;
	tty->flip.count = 0;
	sti();
	
#if 0
	if (count > tty->max_flip_cnt)
		tty->max_flip_cnt = count;
#endif
	tty->ldisc.receive_buf(tty, cp, fp, count);
}

/*
 * This subroutine initializes a tty structure.
 */
static void initialize_tty_struct(struct tty_struct *tty)
{
	memset(tty, 0, sizeof(struct tty_struct));
	tty->magic = TTY_MAGIC;
	tty->ldisc = ldiscs[N_TTY];
	tty->pgrp = -1;
	tty->flip.char_buf_ptr = tty->flip.char_buf;
	tty->flip.flag_buf_ptr = tty->flip.flag_buf;
	tty->flip.tqueue.routine = flush_to_ldisc;
	tty->flip.tqueue.data = tty;
}

/*
 * The default put_char routine if the driver did not define one.
 */
void tty_default_put_char(struct tty_struct *tty, unsigned char ch)
{
	tty->driver.write(tty, 0, &ch, 1);
}

/*
 * Called by a tty driver to register itself.
 */
int tty_register_driver(struct tty_driver *driver)
{
	int error;

	if (driver->flags & TTY_DRIVER_INSTALLED)
		return 0;

	error = register_chrdev(driver->major, driver->name, &tty_fops);
	if (error < 0)
		return error;
	else if(driver->major == 0)
		driver->major = error;

	if (!driver->put_char)
		driver->put_char = tty_default_put_char;
	
	driver->prev = 0;
	driver->next = tty_drivers;
	if (tty_drivers) tty_drivers->prev = driver;
	tty_drivers = driver;
	return error;
}

/*
 * Called by a tty driver to unregister itself.
 */
int tty_unregister_driver(struct tty_driver *driver)
{
	int	retval;
	struct tty_driver *p;
	int	i, found = 0;
	const char *othername = NULL;
	struct termios *tp;
	
	if (*driver->refcount)
		return -EBUSY;

	for (p = tty_drivers; p; p = p->next) {
		if (p == driver)
			found++;
		else if (p->major == driver->major)
			othername = p->name;
	}
	
	if (!found)
		return -ENOENT;

	if (othername == NULL) {
		retval = unregister_chrdev(driver->major, driver->name);
		if (retval)
			return retval;
	} else
		register_chrdev(driver->major, othername, &tty_fops);

	if (driver->prev)
		driver->prev->next = driver->next;
	else
		tty_drivers = driver->next;
	
	if (driver->next)
		driver->next->prev = driver->prev;

	for (i = 0; i < driver->num; i++) {
		tp = driver->termios[i];
		if (tp != NULL) {
			kfree_s(tp, sizeof(struct termios));
			driver->termios[i] = NULL;
		}
		tp = driver->termios_locked[i];
		if (tp != NULL) {
			kfree_s(tp, sizeof(struct termios));
			driver->termios_locked[i] = NULL;
		}
	}
	return 0;
}


/*
 * Initialize the console device. This is called *early*, so
 * we can't necessarily depend on lots of kernel help here.
 * Just do some early initializations, and do the complex setup
 * later.
 */
long console_init(long kmem_start, long kmem_end)
{
	/* Setup the default TTY line discipline. */
	memset(ldiscs, 0, sizeof(ldiscs));
	(void) tty_register_ldisc(N_TTY, &tty_ldisc_N_TTY);

	/*
	 * Set up the standard termios.  Individual tty drivers may 
	 * deviate from this; this is used as a template.
	 */
	memset(&tty_std_termios, 0, sizeof(struct termios));
	memcpy(tty_std_termios.c_cc, INIT_C_CC, NCCS);
	tty_std_termios.c_iflag = ICRNL | IXON;
	tty_std_termios.c_oflag = OPOST | ONLCR;
	tty_std_termios.c_cflag = B38400 | CS8 | CREAD | HUPCL;
	tty_std_termios.c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK |
		ECHOCTL | ECHOKE | IEXTEN;

#ifdef CONFIG_CONSOLE
	/*
	 * set up the console device so that later boot sequences can 
	 * inform about problems etc..
	 */
	return con_init(kmem_start);
#else /* !CONFIG_CONSOLE */
	return kmem_start;
#endif /* !CONFIG_CONSOLE */
}

static struct tty_driver dev_tty_driver, dev_console_driver;

/*
 * Ok, now we can initialize the rest of the tty devices and can count
 * on memory allocations, interrupts etc..
 */
int tty_init(void)
{
	if (sizeof(struct tty_struct) > PAGE_SIZE)
		panic("size of tty structure > PAGE_SIZE!");

	/*
	 * dev_tty_driver and dev_console_driver are actually magic
	 * devices which get redirected at open time.  Nevertheless,
	 * we register them so that register_chrdev is called
	 * appropriately.
	 */
	memset(&dev_tty_driver, 0, sizeof(struct tty_driver));
	dev_tty_driver.magic = TTY_DRIVER_MAGIC;
	dev_tty_driver.name = "tty";
	dev_tty_driver.name_base = 0;
	dev_tty_driver.major = TTY_MAJOR;
	dev_tty_driver.minor_start = 0;
	dev_tty_driver.num = 1;
	
	if (tty_register_driver(&dev_tty_driver))
		panic("Couldn't register /dev/tty driver\n");

	dev_console_driver = dev_tty_driver;
	dev_console_driver.name = "console";
	dev_console_driver.major = TTYAUX_MAJOR;

	if (tty_register_driver(&dev_console_driver))
		panic("Couldn't register /dev/console driver\n");
	
#ifdef CONFIG_FRAMEBUFFER
	fbmem_init();
#endif

#if defined(CONFIG_KEYBOARD) || defined (CONFIG_68332_KEYBOARD)
	kbd_init();
#endif
#ifdef CONFIG_COLDFIRE_SERIAL
	mcfrs_init();
#endif
#ifdef CONFIG_SERIAL
	rs_init();
#endif
#ifdef CONFIG_LEON_SERIAL
	rsLEON_init();
#endif
#ifdef CONFIG_NIOS_SERIAL
	rsNIOS_init();
#endif
#ifdef CONFIG_GBATXT
	gbatxt_init();
#endif
#ifdef CONFIG_68328_SERIAL
	rs68328_init();
#endif
#ifdef CONFIG_68332_SERIAL
	rs68332_init();
#endif
#ifdef CONFIG_LC7981
	lc7981_init();
#endif
#ifdef CONFIG_68332_TPU
	tpu_68332_init();
#endif
#ifdef CONFIG_68332_PORTF
	portf_68332_init();
#endif
#ifdef CONFIG_68332_PORTE
	porte_68332_init();
#endif
#ifdef CONFIG_MAX311X_SERIAL
	rsmax311x_init();
#endif
#ifdef CONFIG_M68360
	rs_quicc_init();
	/* this will internally check for 68360 serial support. */
#endif
#ifdef CONFIG_72001_SERIAL
        /* 2002-05-23 gc: */
        mpsc_init();
#endif
#ifdef CONFIG_SC28L91
	rs_sc28l91_init();
#endif
#ifdef CONFIG_SERIAL_ATMEL
	rs_atmel_init();
#endif
#ifdef CONFIG_SERIAL_ATMEL_BT
	atmel_bt_raw_init();
#endif
#ifdef CONFIG_SCC
	scc_init();
#endif
#ifdef CONFIG_HITACHI_SCI
	sci_init();
#endif
#ifdef CONFIG_CYCLADES
	cy_init();
#endif
#ifdef CONFIG_STALLION
	stl_init();
#endif
#ifdef CONFIG_ISTALLION
	stli_init();
#endif
#ifdef CONFIG_DIGI
	pcxe_init();
#endif
#ifdef CONFIG_RISCOM8
	riscom8_init();
#endif
#ifdef CONFIG_SPECIALIX
	specialix_init();
#endif
	pty_init();
#ifdef CONFIG_CONSOLE
	vcs_init();
#endif
	return 0;
}
