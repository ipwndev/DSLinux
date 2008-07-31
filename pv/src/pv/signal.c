/*
 * Signal handling functions.
 *
 * Copyright 2008 Andrew Wood, distributed under the Artistic License 2.0.
 */

#include "pv.h"

#include <signal.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

static int pv__sig_old_stderr;		 /* see pv__sig_ttou() */
static struct timeval pv__sig_tstp_time; /* see pv__sig_tstp() / __cont() */

struct timeval pv_sig_toffset;		 /* total time spent stopped */
sig_atomic_t pv_sig_newsize = 0;	 /* whether we need to get term size again */
sig_atomic_t pv_sig_abort = 0;		 /* whether we need to abort right now */

#ifdef HAVE_IPC
void pv_crs_needreinit(void);
#endif


/*
 * Handle SIGTTOU (tty output for background process) by redirecting stderr
 * to /dev/null, so that we can be stopped and backgrounded without messing
 * up the terminal. We store the old stderr file descriptor so that on a
 * subsequent SIGCONT we can try writing to the terminal again, in case we
 * get backgrounded and later get foregrounded again.
 */
static void pv__sig_ttou(int s)
{
	int fd;

	fd = open("/dev/null", O_RDWR);	    /* RATS: ignore (no race) */
	if (fd < 0)
		return;

	if (pv__sig_old_stderr == -1)
		pv__sig_old_stderr = dup(STDERR_FILENO);

	dup2(fd, STDERR_FILENO);
	close(fd);
}


/*
 * Handle SIGTSTP (stop typed at tty) by storing the time the signal
 * happened for later use by pv__sig_cont(), and then stopping the process.
 */
static void pv__sig_tstp(int s)
{
	gettimeofday(&pv__sig_tstp_time, NULL);
	raise(SIGSTOP);
}


/*
 * Handle SIGCONT (continue if stopped) by adding the elapsed time since the
 * last SIGTSTP to the elapsed time offset, and by trying to write to the
 * terminal again (by replacing the /dev/null stderr with the old stderr).
 */
static void pv__sig_cont(int s)
{
	struct timeval tv;
	struct termios t;

	pv_sig_newsize = 1;

	if (pv__sig_tstp_time.tv_sec == 0) {
		tcgetattr(STDERR_FILENO, &t);
		t.c_lflag |= TOSTOP;
		tcsetattr(STDERR_FILENO, TCSANOW, &t);
#ifdef HAVE_IPC
		pv_crs_needreinit();
#endif
		return;
	}

	gettimeofday(&tv, NULL);

	pv_sig_toffset.tv_sec += (tv.tv_sec - pv__sig_tstp_time.tv_sec);
	pv_sig_toffset.tv_usec += (tv.tv_usec - pv__sig_tstp_time.tv_usec);
	if (pv_sig_toffset.tv_usec >= 1000000) {
		pv_sig_toffset.tv_sec++;
		pv_sig_toffset.tv_usec -= 1000000;
	}
	if (pv_sig_toffset.tv_usec < 0) {
		pv_sig_toffset.tv_sec--;
		pv_sig_toffset.tv_usec += 1000000;
	}

	pv__sig_tstp_time.tv_sec = 0;
	pv__sig_tstp_time.tv_usec = 0;

	if (pv__sig_old_stderr != -1) {
		dup2(pv__sig_old_stderr, STDERR_FILENO);
		close(pv__sig_old_stderr);
		pv__sig_old_stderr = -1;
	}

	tcgetattr(STDERR_FILENO, &t);
	t.c_lflag |= TOSTOP;
	tcsetattr(STDERR_FILENO, TCSANOW, &t);

#ifdef HAVE_IPC
	pv_crs_needreinit();
#endif
}


/*
 * Handle SIGWINCH (window size changed) by setting a flag.
 */
static void pv__sig_winch(int s)
{
	pv_sig_newsize = 1;
}


/*
 * Handle termination signals by setting the abort flag.
 */
static void pv__sig_term(int s)
{
	pv_sig_abort = 1;
}


/*
 * Initialise signal handling.
 */
void pv_sig_init(void)
{
	struct sigaction sa;

	pv__sig_old_stderr = -1;
	pv__sig_tstp_time.tv_sec = 0;
	pv__sig_tstp_time.tv_usec = 0;
	pv_sig_toffset.tv_sec = 0;
	pv_sig_toffset.tv_usec = 0;

	/*
	 * Ignore SIGPIPE, so we don't die if stdout is a pipe and the other
	 * end closes unexpectedly.
	 */
	sa.sa_handler = SIG_IGN;
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = 0;
	sigaction(SIGPIPE, &sa, NULL);

	/*
	 * Handle SIGTTOU by continuing with output switched off, so that we
	 * can be stopped and backgrounded without messing up the terminal.
	 */
	sa.sa_handler = pv__sig_ttou;
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = 0;
	sigaction(SIGTTOU, &sa, NULL);

	/*
	 * Handle SIGTSTP by storing the time the signal happened for later
	 * use by pv__sig_cont(), and then stopping the process.
	 */
	sa.sa_handler = pv__sig_tstp;
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = 0;
	sigaction(SIGTSTP, &sa, NULL);

	/*
	 * Handle SIGCONT by adding the elapsed time since the last SIGTSTP
	 * to the elapsed time offset, and by trying to write to the
	 * terminal again.
	 */
	sa.sa_handler = pv__sig_cont;
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = 0;
	sigaction(SIGCONT, &sa, NULL);

	/*
	 * Handle SIGWINCH by setting a flag to let the main loop know it
	 * has to reread the terminal size.
	 */
	sa.sa_handler = pv__sig_winch;
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = 0;
	sigaction(SIGWINCH, &sa, NULL);

	/*
	 * Handle SIGINT, SIGHUP, SIGTERM by setting a flag to let the
	 * main loop know it should quit now.
	 */
	sa.sa_handler = pv__sig_term;
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);

	sa.sa_handler = pv__sig_term;
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = 0;
	sigaction(SIGHUP, &sa, NULL);

	sa.sa_handler = pv__sig_term;
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = 0;
	sigaction(SIGTERM, &sa, NULL);
}


/*
 * Stop reacting to SIGTSTP and SIGCONT.
 */
void pv_sig_nopause(void)
{
	struct sigaction sa;

	sa.sa_handler = SIG_IGN;
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = 0;
	sigaction(SIGTSTP, &sa, NULL);

	sa.sa_handler = SIG_DFL;
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = 0;
	sigaction(SIGCONT, &sa, NULL);
}


/*
 * Start catching SIGTSTP and SIGCONT again.
 */
void pv_sig_allowpause(void)
{
	struct sigaction sa;

	sa.sa_handler = pv__sig_tstp;
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = 0;
	sigaction(SIGTSTP, &sa, NULL);

	sa.sa_handler = pv__sig_cont;
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = 0;
	sigaction(SIGCONT, &sa, NULL);
}


/*
 * If we have redirected stderr to /dev/null, check every second or so to
 * see whether we can write to the terminal again - this is so that if we
 * get backgrounded, then foregrounded again, we start writing to the
 * terminal again.
 */
void pv_sig_checkbg(void)
{
	static time_t next_check = 0;
	struct termios t;

	if (time(NULL) < next_check)
		return;

	next_check = time(NULL) + 1;

	if (pv__sig_old_stderr == -1)
		return;

	dup2(pv__sig_old_stderr, STDERR_FILENO);
	close(pv__sig_old_stderr);
	pv__sig_old_stderr = -1;

	tcgetattr(STDERR_FILENO, &t);
	t.c_lflag |= TOSTOP;
	tcsetattr(STDERR_FILENO, TCSANOW, &t);

#ifdef HAVE_IPC
	pv_crs_needreinit();
#endif
}

/* EOF */
