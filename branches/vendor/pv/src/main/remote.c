/*
 * Remote-control functions.
 *
 * Copyright 2008 Andrew Wood, distributed under the Artistic License 2.0.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "options.h"
#include "pv.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>


struct remote_msg {
	long mtype;
	unsigned char progress;		 /* progress bar flag */
	unsigned char timer;		 /* timer flag */
	unsigned char eta;		 /* ETA flag */
	unsigned char rate;		 /* rate counter flag */
	unsigned char bytes;		 /* bytes transferred flag */
	unsigned long long rate_limit;	 /* rate limit, in bytes per second */
	unsigned long long buffer_size;	 /* buffer size, in bytes (0=default) */
	unsigned long long size;	 /* total size of data */
	double interval;		 /* interval between updates */
	unsigned int width;		 /* screen width */
	unsigned int height;		 /* screen height */
	char name[256];			 /* RATS: ignore */
};


static opts_t remote__opts = NULL;


/*
 * Return a key for use with msgget() which will be unique to the current
 * user.
 *
 * We can't just use ftok() because the queue needs to be user-specific
 * so that a user cannot send messages to another user's process, and we
 * can't easily find out the terminal a given process is connected to in a
 * cross-platform way.
 */
static key_t remote__genkey(opts_t opts)
{
	int uid;
	key_t key;

	uid = geteuid();
	if (uid < 0)
		uid = 0;

	key = ftok("/tmp", 'P') | uid;

	return key;
}


/*
 * Return a message queue ID that is unique to the current user and the
 * given process ID, or -1 on error.
 */
static int remote__msgget(opts_t opts)
{
	return msgget(remote__genkey(opts), IPC_CREAT | 0600);
}


/*
 * Set the options of a remote process by setting up an IPC message queue,
 * sending a message containing the new options, and then sending a SIGUSR1
 * so the process knows it has a message to read.
 *
 * Returns nonzero on error.
 */
int remote_set(opts_t opts)
{
	struct remote_msg msgbuf;
	int msgid;

	memset(&msgbuf, 0, sizeof(msgbuf));
	msgbuf.mtype = opts->remote;
	msgbuf.progress = opts->progress;
	msgbuf.timer = opts->timer;
	msgbuf.eta = opts->eta;
	msgbuf.rate = opts->rate;
	msgbuf.rate_limit = opts->rate_limit;
	msgbuf.buffer_size = opts->buffer_size;
	msgbuf.size = opts->size;
	msgbuf.interval = opts->interval;
	msgbuf.width = opts->width;
	msgbuf.height = opts->height;
	if (opts->name != NULL) {
		strncpy(msgbuf.name, opts->name, sizeof(msgbuf.name) - 1);
	}

	msgid = remote__msgget(opts);
	if (msgid < 0) {
		fprintf(stderr, "%s: %s", opts->program_name,
			strerror(errno));
		return -1;
	}

	if (msgsnd(msgid, &msgbuf, sizeof(msgbuf) - sizeof(long), 0) != 0) {
		fprintf(stderr, "%s: %s", opts->program_name,
			strerror(errno));
		return -1;
	}

	if (kill(opts->remote, SIGUSR1) != 0) {
		fprintf(stderr, "%s: %s", opts->program_name,
			strerror(errno));
		return -1;
	}

	return 0;
}


/*
 * Handle SIGUSR1 by replacing the current process's options with those
 * being passed in via IPC message. The message queue is deleted afterwards.
 */
static void remote__sig_usr1(int s)
{
	struct remote_msg msgbuf;
	struct msqid_ds qbuf;
	ssize_t got;
	int msgid;

	memset(&msgbuf, 0, sizeof(msgbuf));

	msgid = remote__msgget(remote__opts);
	if (msgid < 0) {
		return;
	}

	got =
	    msgrcv(msgid, &msgbuf, sizeof(msgbuf) - sizeof(long), getpid(),
		   IPC_NOWAIT);
	if (got < 0) {
		msgctl(msgid, IPC_RMID, &qbuf);
		return;
	}

	if (msgctl(msgid, IPC_RMID, &qbuf) == 0) {
		if (qbuf.msg_qnum < 1) {
			msgctl(msgid, IPC_RMID, &qbuf);
		}
	}

	if ((got < 1) || (remote__opts == NULL)) {
		return;
	}


	remote__opts->progress = msgbuf.progress;
	remote__opts->timer = msgbuf.timer;
	remote__opts->eta = msgbuf.eta;
	remote__opts->rate = msgbuf.rate;

	if (msgbuf.rate_limit > 0)
		remote__opts->rate_limit = msgbuf.rate_limit;
	if (msgbuf.buffer_size > 0) {
		remote__opts->buffer_size = msgbuf.buffer_size;
		pv_set_buffer_size(msgbuf.buffer_size, 1);
	}
	if (msgbuf.size > 0)
		remote__opts->size = msgbuf.size;
	if (msgbuf.interval > 0)
		remote__opts->interval = msgbuf.interval;
	if (msgbuf.width > 0)
		remote__opts->width = msgbuf.width;
	if (msgbuf.height > 0)
		remote__opts->height = msgbuf.height;
	if (msgbuf.name[0] != 0)
		remote__opts->name = strdup(msgbuf.name);
}


/*
 * Initialise handling of SIGUSR1 so that remote control messages can be
 * processed correctly.
 */
void remote_sig_init(opts_t opts)
{
	struct sigaction sa;

	remote__opts = opts;

	sa.sa_handler = remote__sig_usr1;
	sigemptyset(&(sa.sa_mask));
	sa.sa_flags = 0;
	sigaction(SIGUSR1, &sa, NULL);
}

/* EOF */
