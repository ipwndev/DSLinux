#ifdef __BEOS__
/*
 * select.c
 * Copyright (c) 1995,1996,1997 Be, Inc.	All Rights Reserved 
 *
 * Handling select()ing on both the net and the tty simultaneously.  
 * Warning: This code is very tricky.
 *
 * One day, select() will just work on all types of descriptors and this
 * code won't be necessary.
 *
 * The original version is in the GeekGadgets TCL port.  This version
 * has been further streamlined to not handle sockets at all, but only tty's.
 */
#include <stdio.h>
#include <OS.h>
#include <socket.h>
#include <fcntl.h>
#include <stdarg.h>
#include <signal.h>
#include <errno.h>

/* Change this section if necessary */
#define dprintf (void)
#define SOCKBASE OPEN_MAX

#undef select
#undef ioctl

#define NTTY 4
#define STEP 50000

extern void _socket_interrupt(int fd);

typedef struct select_parms {
	int fd;
	int *fds;
	int extra;
	int mask;
	int *masks;
	int ready; 
	int result;
	thread_id id;
	struct timeval *timeout;
	sem_id eventsem;
	sem_id running;
	sem_id donesem;
} select_parms;

#define fork_thread(f, a) _fork_thread(f, #f, a)

static struct timeval zero = { 0, 0 };

static long
_fork_thread(
			 thread_entry func,
			 char *name,
			 void *args
			 )
{
	thread_id id;

	dprintf("forking thread %s\n", name);
	resume_thread(id = spawn_thread(func, name, B_NORMAL_PRIORITY, args));
	return (id);
}


void
sigint(int ignored)
{
	signal(SIGINT, sigint);
}

static long
tty_select_thread(
				  void *arg
				  )
{
	select_parms *parms = (select_parms *)arg;
	int n;
	int ret;

	signal(SIGINT, sigint);
	release_sem(parms->running);
	ret = tty_select(1, &parms->fd, &parms->mask, parms->timeout);
	dprintf("tty select returns %d, %d\n", ret, parms->mask);
	parms->result = ret;
	parms->ready = 1;
	release_sem(parms->eventsem);
	release_sem(parms->donesem);
	return (0);
}

static long
tty_mult_select_thread(
					   void *arg
					   )
{
	select_parms *parms = (select_parms *)arg;
	int n;
	int ret;

	signal(SIGINT, sigint);
	release_sem(parms->running);
	ret = tty_select(parms->fd, parms->fds, parms->masks, parms->timeout);
	dprintf("tty select (mult) returns %d\n", ret);
	parms->result = ret;
	parms->ready = 1;
	release_sem(parms->eventsem);
	release_sem(parms->donesem);
	return (0);
}

int
tty_select_multiple(int ntty, int *ttyfd, int *ttyflags, 
					struct timeval *timeout)
{
	sem_id eventsem;
	select_parms parms[10];
	int nparms = 0;
	long status;
	int i;

	eventsem = create_sem(0, "select event");

	for (i = 0; i < ntty; i++) {
		if (ttyflags[i] & 1) {
			parms[nparms].fd = ttyfd[i];
			parms[nparms].extra = i;
			parms[nparms].mask = 1;
			parms[nparms].ready = 0;
			parms[nparms].timeout = timeout;
			parms[nparms].eventsem = eventsem;
			parms[nparms].donesem = create_sem(0, "ttyrw done");
			parms[nparms].running = create_sem(0, "ttyrw running");
			parms[nparms].id = fork_thread(tty_select_thread, 
										   &parms[nparms]);
			acquire_sem(parms[nparms].running);
			delete_sem(parms[nparms].running);
			nparms++;
		}
		if (ttyflags[i] & 2) {
			parms[nparms].fd = ttyfd[i];
			parms[nparms].extra = i;
			parms[nparms].mask = 2;
			parms[nparms].ready = 0;
			parms[nparms].timeout = timeout;
			parms[nparms].eventsem = eventsem;
			parms[nparms].donesem = create_sem(0, "ttyrw done");
			parms[nparms].running = create_sem(0, "ttyrw running");
			parms[nparms].id = fork_thread(tty_select_thread, 
										   &parms[nparms]);
			acquire_sem(parms[nparms].running);
			delete_sem(parms[nparms].running);
			nparms++;
		}
	}
	status = acquire_sem(eventsem);
	if (status < B_NO_ERROR) {
		dprintf("select net tty event: %s\n", strerror(status));
	}
	
	for (i = 0; i < nparms; i++) {
		if (!parms[i].ready) {
			dprintf("tty1 not ready: interrupt it\n");
			for (;;) {
				kill(parms[i].id, SIGINT);
				status = acquire_sem_etc(parms[i].donesem, 1, B_TIMEOUT, 
									 1000000);
				if (status != B_TIMED_OUT) {
					break;
				}
				dprintf("tty thread didn't die, killing again\n");	
			}
		}
		delete_sem(parms[i].donesem);
		wait_for_thread(parms[i].id, &status);
	}
	delete_sem(eventsem);
	for (i = 0; i < nparms; i++) {
		if (parms[i].ready && parms[i].result > 0) {
			memset(ttyflags, 0, sizeof(ttyflags[0]) * ntty);
			ttyflags[parms[i].extra] = parms[i].mask;
			return (1);
		}
	}
	return (-1);
}

int
tty_select(
		   int ntty,
		   int *ttyfdp,
		   int *ttyflagsp,
		   struct timeval *timeout
		   )
{
	int n = 1;
	int howmany;
	int cmd;
	int tryflags;
	int ttyflags = *ttyflagsp;
	int ttyfd = *ttyfdp;
	int i;
	bigtime_t d;
	bigtime_t useconds = 0;

	if (timeout != NULL) {
		if (timeout->tv_sec == 0 && timeout->tv_usec == 0) {
			n = 0;
		} else {
			useconds = (timeout->tv_sec * 1000000LL + timeout->tv_usec);
		}
	}
	if (ntty > 1 || (ttyflagsp[0] == 3)) {
		if (n > 0) {
			return (tty_select_multiple(ntty, ttyfdp, ttyflagsp, timeout));
		} else {
			for (i = 0; i < ntty; i++) {
				if (ttyflagsp[i] & 1) {
					tryflags = 1;
					n = 0;
					n = tty_select(1, &ttyfdp[i], &tryflags, &zero);
					if (n > 0) {
						memset(ttyflagsp, 0, sizeof(ttyflagsp[0]) * ntty);
						ttyflagsp[i] = tryflags;
						return (n);
					}
				}
				if (ttyflagsp[i] & 2) {
					tryflags = 2;
					n = tty_select(1, &ttyfdp[i], &tryflags, &zero);
					if (n > 0) {
						memset(ttyflagsp, 0, sizeof(ttyflagsp[0]) * ntty);
						ttyflagsp[i] = tryflags;
						return (n);
					}
				}
			}
			memset(ttyflagsp, 0, sizeof(ttyflagsp[0]) * ntty);
		}
		return (0);
	}
	if (ttyflags == 1) {
		cmd = 'ichr';
	} else {
		cmd = 'ochr';
	}
	if (useconds) {
		for (d = 0; d < useconds; d += STEP) {
			n = 0;
			dprintf("ioctl(%d, %08x, %d)...\n", ttyfd, cmd, n);
			howmany = ioctl(ttyfd, cmd, &n);
			dprintf("ioctl(%d, %08x) = %d, %d\n", ttyfd, cmd, howmany, n);
			if (howmany >= 0 && n > 0) {
				break;
			}
			snooze(STEP);
		}
	} else {
		dprintf("ioctl(%d, %08x, %d)...\n", ttyfd, cmd, n);
		howmany = ioctl(ttyfd, cmd, &n);
		dprintf("ioctl(%d, %08x) = %d, %d\n", ttyfd, cmd, howmany, n);
	}
	if (howmany < 0) {
		if (useconds == 0 && cmd == 'ichr') {
			/*
			 * Check for EOF
			 */
			n = 0;
			howmany = ioctl(ttyfd, cmd, &n);
			if (howmany < 0 || n > 0) {
				dprintf("EOF or data\n");
				return (1);
			}
		}
		/*
		 * Else, assume EINTR.
		 */
		*ttyflagsp = 0;
		return (0);
	}
	if (n == 0) {
		dprintf("clearing flags\n");
		*ttyflagsp = 0;
	}
	dprintf("tty_select: %d\n", !!n);
	return (!!n);
}

static void 
show(char *name,
	 int nbits,
	 struct fd_set *rbits,
	 struct fd_set *wbits,
	 struct fd_set *ebits,
	 struct timeval *timeout
	 )
{
	dprintf("%s select(%d): timeout = %08x %d %d, masks = %08x %08x %08x\n",
			name,
			nbits,
			timeout, (timeout ? timeout->tv_sec: -1),
			(timeout ? timeout->tv_usec: -1),
			(rbits ? rbits->mask[0] : 0xffffffff),
			(wbits ? wbits->mask[0] : 0xffffffff),
			(ebits ? ebits->mask[0] : 0xffffffff));

}

int
check_select(
			 int nbits,
			 struct fd_set *rbits,
			 struct fd_set *wbits,
			 struct fd_set *ebits,
			 struct timeval *timeout
			 )
{
	int ret;
	int ttyfd[NTTY];
	int ttyflags[NTTY];
	int ntty = 0;
	int i;


	show("Enter", nbits, rbits, wbits, ebits, timeout);
	if (ebits) {
		ebits->mask[0] = 0;
	}
	for (i = 0; i < nbits; i++) {
		if (rbits && (rbits->mask[i/32] & (1 << (i%32)))) {
				ttyfd[ntty] = i;
				ttyflags[ntty++] = 1;
		}
		if (wbits && (wbits->mask[i/32] & (1 << (i%32)))) {
				if (ntty > 0 && ttyfd[ntty] == i) {
					ttyflags[ntty++] |= 2;
				} else {
					ttyfd[ntty] = i;
					ttyflags[ntty++] = 2;
				}
		}
	}
	if (ntty > 0) {
		ret = tty_select(ntty, ttyfd, ttyflags, timeout);
		if (ret >= 0) {
			if (rbits) {
				rbits->mask[0] = 0;
			}
			if (wbits) {
				wbits->mask[0] = 0;
			}
			for (i = 0; i < ntty; i++) {
				if (ttyflags[i] & 1) {
					rbits->mask[0] |= (1 << ttyfd[i]);
				}
				if (ttyflags[i] & 2) {
					wbits->mask[0] |= (1 << ttyfd[i]);
				}
			}
		}
		show("Exit-tty", ret, rbits, wbits, ebits, timeout);
		return (ret);
	}
	show("Exit-nothing", 0, rbits, wbits, ebits, timeout);
	return (0);
}
#endif	/* __BEOS__ */
