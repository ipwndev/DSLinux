/*
 * Cursor positioning functions.
 *
 * If IPC is available, then a shared memory segment is used to co-ordinate
 * cursor positioning across multiple instances of `pv'. The shared memory
 * segment contains an integer which is the original "y" co-ordinate of the
 * first `pv' process.
 *
 * However, some OSes (FreeBSD and MacOS X so far) don't allow locking of a
 * terminal, so we try to use a lockfile if terminal locking doesn't work,
 * and finally abort if even that is unavailable.
 *
 * Copyright 2008 Andrew Wood, distributed under the Artistic License 2.0.
 */

#include "options.h"
#include "pv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_IPC
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
# ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
# endif
# ifdef HAVE_LIBGEN_H
# include <libgen.h>
# endif
#endif				/* HAVE_IPC */


#ifdef HAVE_IPC
static int pv_crs__shmid = -1;		 /* ID of our shared memory segment */
static int pv_crs__pvcount = 1;		 /* number of `pv' processes in total */
static int pv_crs__pvmax = 0;		 /* highest number of `pv's seen */
static int *pv_crs__y_top = 0;		 /* pointer to Y coord of topmost `pv' */
static int pv_crs__y_lastread = 0;	 /* last value of __y_top seen */
static int pv_crs__y_offset = 0;	 /* our Y offset from this top position */
static int pv_crs__needreinit = 0;	 /* set if we need to reinit cursor pos */
static int pv_crs__noipc = 0;		 /* set if we can't use IPC */
#endif				/* HAVE_IPC */
static int pv_crs__uselockfile = 0;	 /* set if we used a lockfile */
static int pv_crs__lock_fd = -1;	 /* fd of lockfile, -1 if none open */
static int pv_crs__y_start = 0;		 /* our initial Y coordinate */


/*
 * Lock the terminal on the given file descriptor by creating and locking a
 * per-euid, per-tty, lockfile in ${TMPDIR:-${TMP:-/tmp}}.
 */
static void pv_crs__lock_lockfile(int fd)
{
#ifdef O_EXLOCK
	char *ttydev;
	char *tmpdir;
#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif
	char lockfile[MAXPATHLEN + 1];	 /* RATS: ignore */

	pv_crs__uselockfile = 1;

	ttydev = ttyname(fd);		    /* RATS: ignore */
	if (!ttydev) {
#ifdef HAVE_IPC
		pv_crs__noipc = 1;
#endif
		return;
	}

	tmpdir = (char *) getenv("TMPDIR"); /* RATS: ignore */
	if (!tmpdir)
		tmpdir = (char *) getenv("TMP");	/* RATS: ignore */
	if (!tmpdir)
		tmpdir = "/tmp";

#ifdef HAVE_SNPRINTF
	snprintf(lockfile, MAXPATHLEN, "%s/pv-%s-%i.lock",
		 tmpdir, basename(ttydev), geteuid());
#else
	sprintf(lockfile,		    /* RATS: ignore */
		"%.*s/pv-%8s-%i.lock",
		MAXPATHLEN - 64, tmpdir, basename(ttydev), geteuid());
#endif

	pv_crs__lock_fd =
	    open(lockfile, O_RDWR | O_EXLOCK | O_CREAT | O_NOFOLLOW, 0600);
#ifdef HAVE_IPC
	if (pv_crs__lock_fd < 0)
		pv_crs__noipc = 1;
#endif

#else				/* !O_EXLOCK */

	pv_crs__uselockfile = 1;
#ifdef HAVE_IPC
	pv_crs__noipc = 1;
#endif

#endif				/* O_EXLOCK */
}


/*
 * Lock the terminal on the given file descriptor, falling back to using a
 * lockfile if the terminal itself cannot be locked.
 */
static void pv_crs__lock(int fd)
{
	struct flock lock;

	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 1;
	while (fcntl(fd, F_SETLKW, &lock) < 0) {
		if (errno != EINTR) {
			pv_crs__lock_lockfile(fd);
			return;
		}
	}
}


/*
 * Unlock the terminal on the given file descriptor.  If pv_crs__lock used
 * lockfile locking, unlock the lockfile.
 */
static void pv_crs__unlock(int fd)
{
	struct flock lock;

	if (pv_crs__uselockfile) {
		if (pv_crs__lock_fd >= 0)
			close(pv_crs__lock_fd);
		pv_crs__lock_fd = -1;
	} else {
		lock.l_type = F_UNLCK;
		lock.l_whence = SEEK_SET;
		lock.l_start = 0;
		lock.l_len = 1;
		fcntl(fd, F_SETLK, &lock);
	}
}


#ifdef HAVE_IPC
/*
 * Get the current number of processes attached to our shared memory
 * segment, i.e. find out how many `pv' processes in total are running in
 * cursor mode (including us), and store it in pv_crs__pvcount. If this is
 * larger than pv_crs__pvmax, update pv_crs__pvmax.
 */
static void pv_crs__ipccount(void)
{
	struct shmid_ds buf;

	buf.shm_nattch = 0;

	shmctl(pv_crs__shmid, IPC_STAT, &buf);
	pv_crs__pvcount = buf.shm_nattch;

	if (pv_crs__pvcount > pv_crs__pvmax)
		pv_crs__pvmax = pv_crs__pvcount;
}
#endif				/* HAVE_IPC */


/*
 * Get the current cursor Y co-ordinate by sending the ECMA-48 CPR code to
 * the terminal connected to the given file descriptor.
 */
static int pv_crs__get_ypos(int terminalfd)
{
	struct termios tty;
	struct termios old_tty;
	char cpr[32];			 /* RATS: ignore (checked) */
	int ypos;

	tcgetattr(terminalfd, &tty);
	tcgetattr(terminalfd, &old_tty);
	tty.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(terminalfd, TCSANOW | TCSAFLUSH, &tty);
	write(terminalfd, "\033[6n", 4);
	memset(cpr, 0, sizeof(cpr));
	read(terminalfd, cpr, 6);	    /* RATS: ignore (OK) */
	ypos = pv_getnum_i(cpr + 2);
	tcsetattr(terminalfd, TCSANOW | TCSAFLUSH, &old_tty);

	return ypos;
}


#ifdef HAVE_IPC
/*
 * Initialise the IPC data, returning nonzero on error.
 *
 * To do this, we attach to the shared memory segment (creating it if it
 * does not exist). If we are the only process attached to it, then we
 * initialise it with the current cursor position.
 *
 * There is a race condition here: another process could attach before we've
 * had a chance to check, such that no process ends up getting an "attach
 * count" of one, and so no initialisation occurs. So, we lock the terminal
 * with pv_crs__lock() while we are attaching and checking.
 */
static int pv_crs__ipcinit(opts_t opts, char *ttyfile, int terminalfd)
{
	key_t key;

	/*
	 * Base the key for the shared memory segment on our current tty, so
	 * we don't end up interfering in any way with instances of `pv'
	 * running on another terminal.
	 */
	key = ftok(ttyfile, 'p');
	if (key == -1) {
		fprintf(stderr, "%s: %s: %s\n",
			opts->program_name,
			_("failed to open terminal"), strerror(errno));
		return 1;
	}

	pv_crs__lock(terminalfd);
	if (pv_crs__noipc) {
		fprintf(stderr, "%s: %s: %s\n",
			opts->program_name,
			_("failed to lock terminal"), strerror(errno));
		return 1;
	}

	pv_crs__shmid = shmget(key, sizeof(int), 0600 | IPC_CREAT);
	if (pv_crs__shmid < 0) {
		fprintf(stderr, "%s: %s: %s\n",
			opts->program_name,
			_("failed to open terminal"), strerror(errno));
		pv_crs__unlock(terminalfd);
		return 1;
	}

	pv_crs__y_top = shmat(pv_crs__shmid, 0, 0);

	pv_crs__ipccount();

	/*
	 * If nobody else is attached to the shared memory segment, we're
	 * the first, so we need to initialise the shared memory with our
	 * current Y cursor co-ordinate.
	 */
	if (pv_crs__pvcount < 2) {
		pv_crs__y_start = pv_crs__get_ypos(terminalfd);
		*pv_crs__y_top = pv_crs__y_start;
		pv_crs__y_lastread = pv_crs__y_start;
	}

	pv_crs__y_offset = pv_crs__pvcount - 1;
	if (pv_crs__y_offset < 0)
		pv_crs__y_offset = 0;

	/*
	 * If anyone else had attached to the shared memory segment, we need
	 * to read the top Y co-ordinate from it.
	 */
	if (pv_crs__pvcount > 1) {
		pv_crs__y_start = *pv_crs__y_top;
		pv_crs__y_lastread = pv_crs__y_start;
	}

	pv_crs__unlock(terminalfd);

	return 0;
}
#endif				/* HAVE_IPC */


/*
 * Initialise the terminal for cursor positioning.
 */
void pv_crs_init(opts_t opts)
{
	char *ttyfile;
	int fd;

	if (!opts->cursor)
		return;

	ttyfile = ttyname(STDERR_FILENO);   /* RATS: ignore (unimportant) */
	if (!ttyfile) {
		opts->cursor = 0;
		return;
	}

	fd = open(ttyfile, O_RDWR);	    /* RATS: ignore (no race) */
	if (fd < 0) {
		fprintf(stderr, "%s: %s: %s\n",
			opts->program_name,
			_("failed to open terminal"), strerror(errno));
		opts->cursor = 0;
		return;
	}
#ifdef HAVE_IPC
	if (pv_crs__ipcinit(opts, ttyfile, fd)) {
		opts->cursor = 0;
		close(fd);
		return;
	}

	/*
	 * If we are not using IPC, then we need to get the current Y
	 * co-ordinate. If we are using IPC, then the pv_crs__ipcinit()
	 * function takes care of this in a more multi-process-friendly way.
	 */
	if (pv_crs__noipc) {
#else				/* ! HAVE_IPC */
	if (1) {
#endif				/* HAVE_IPC */
		/*
		 * Get current cursor position + 1.
		 */
		pv_crs__lock(fd);
		pv_crs__y_start = pv_crs__get_ypos(fd);
		pv_crs__unlock(fd);

		if (pv_crs__y_start < 1)
			opts->cursor = 0;
	}

	close(fd);
}


#ifdef HAVE_IPC
/*
 * Set the "we need to reinitialise cursor positioning" flag.
 */
void pv_crs_needreinit(void)
{
	pv_crs__needreinit += 2;
	if (pv_crs__needreinit > 3)
		pv_crs__needreinit = 3;
}
#endif


#ifdef HAVE_IPC
/*
 * Reinitialise the cursor positioning code (called if we are backgrounded
 * then foregrounded again).
 */
void pv_crs_reinit(void)
{
	pv_crs__lock(STDERR_FILENO);

	pv_crs__needreinit--;
	if (pv_crs__y_offset < 1)
		pv_crs__needreinit = 0;

	if (pv_crs__needreinit > 0) {
		pv_crs__unlock(STDERR_FILENO);
		return;
	}

	pv_crs__y_start = pv_crs__get_ypos(STDERR_FILENO);

	if (pv_crs__y_offset < 1)
		*pv_crs__y_top = pv_crs__y_start;
	pv_crs__y_lastread = pv_crs__y_start;

	pv_crs__unlock(STDERR_FILENO);
}
#endif


/*
 * Output a single-line update, moving the cursor to the correct position to
 * do so.
 */
void pv_crs_update(opts_t opts, char *str)
{
	char pos[32];			 /* RATS: ignore (checked OK) */
	int y;

#ifdef HAVE_IPC
	if (!pv_crs__noipc) {
		if (pv_crs__needreinit)
			pv_crs_reinit();

		pv_crs__ipccount();
		if (pv_crs__y_lastread != *pv_crs__y_top) {
			pv_crs__y_start = *pv_crs__y_top;
			pv_crs__y_lastread = pv_crs__y_start;
		}

		if (pv_crs__needreinit > 0)
			return;
	}
#endif				/* HAVE_IPC */

	y = pv_crs__y_start;

#ifdef HAVE_IPC
	/*
	 * If the screen has scrolled, or is about to scroll, due to
	 * multiple `pv' instances taking us near the bottom of the screen,
	 * scroll the screen (only if we're the first `pv'), and then move
	 * our initial Y co-ordinate up.
	 */
	if (((pv_crs__y_start + pv_crs__pvmax) > opts->height)
	    && (!pv_crs__noipc)
	    ) {
		int offs;

		offs = ((pv_crs__y_start + pv_crs__pvmax) - opts->height);

		pv_crs__y_start -= offs;
		if (pv_crs__y_start < 1)
			pv_crs__y_start = 1;

		/*
		 * Scroll the screen if we're the first `pv'.
		 */
		if (pv_crs__y_offset == 0) {
			pv_crs__lock(STDERR_FILENO);

			sprintf(pos, "\033[%d;1H", opts->height);
			write(STDERR_FILENO, pos, strlen(pos));
			for (; offs > 0; offs--) {
				write(STDERR_FILENO, "\n", 1);
			}

			pv_crs__unlock(STDERR_FILENO);
		}
	}

	if (!pv_crs__noipc)
		y = pv_crs__y_start + pv_crs__y_offset;
#endif				/* HAVE_IPC */

	/*
	 * Keep the Y co-ordinate within sensible bounds, so we can never
	 * overflow the "pos" buffer.
	 */
	if ((y < 1) || (y > 999999))
		y = 1;
	sprintf(pos, "\033[%d;1H", y);

	pv_crs__lock(STDERR_FILENO);

	write(STDERR_FILENO, pos, strlen(pos));	/* RATS: ignore */
	write(STDERR_FILENO, str, strlen(str));	/* RATS: ignore */

	pv_crs__unlock(STDERR_FILENO);
}


/*
 * Reposition the cursor to a final position.
 */
void pv_crs_fini(opts_t opts)
{
	char pos[32];			 /* RATS: ignore (checked OK) */
	int y;

	y = pv_crs__y_start;

#ifdef HAVE_IPC
	if ((pv_crs__pvmax > 0) && (!pv_crs__noipc))
		y += pv_crs__pvmax - 1;
#endif				/* HAVE_IPC */

	if (y > opts->height)
		y = opts->height;

	/*
	 * Absolute bounds check.
	 */
	if ((y < 1) || (y > 999999))
		y = 1;

	sprintf(pos, "\033[%d;1H\n", y);    /* RATS: ignore */

	pv_crs__lock(STDERR_FILENO);

	write(STDERR_FILENO, pos, strlen(pos));	/* RATS: ignore */

#ifdef HAVE_IPC
	pv_crs__ipccount();
	shmdt((void *) pv_crs__y_top);

	/*
	 * If we are the last instance detaching from the shared memory,
	 * delete it so it's not left lying around.
	 */
	if (pv_crs__pvcount < 2)
		shmctl(pv_crs__shmid, IPC_RMID, 0);

#endif				/* HAVE_IPC */

	pv_crs__unlock(STDERR_FILENO);
}

/* EOF */
