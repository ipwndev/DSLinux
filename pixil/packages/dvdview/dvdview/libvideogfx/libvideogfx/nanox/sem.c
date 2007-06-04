/* System V IPC semaphores */
/* Copyright (c) 2001 by Greg Haerr <greg@censoft.com>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include "sem.h"

/* Not defined by many operating systems, use configure to detect */
#if !defined(HAVE_SEMUN)
union semun {
	int val;
	struct semid_ds *buf;
	ushort *array;
};
#endif

static struct sembuf op_trywait[2] = {
	{ 0, -1, (IPC_NOWAIT|SEM_UNDO) } /* Decrement semaphore, no block */
};
static struct sembuf op_wait[2] = {
	{ 0, -1, SEM_UNDO }		/* Decrement semaphore */
};
static struct sembuf op_post[1] = {
	{ 0, 1, (IPC_NOWAIT|SEM_UNDO) }	/* Increment semaphore */
};

MWSEMAPHORE
GdCreateSemaphore(int public, int create, int initial_value)
{
	int		id;
	int		flags = 0666;
	key_t		key;
	union semun	init;

#define GR_NAMED_SOCKET "/tmp/.nano-X"
	key = public? ftok(GR_NAMED_SOCKET, public): IPC_PRIVATE;
	if (create)
		flags |= IPC_CREAT;

	/* Keep trying to create sem while we don't own the requested key */
	//do {
		//if (key != IPC_PRIVATE)
			//++key;
		id = semget(key, 1, flags);
	//} while (id < 0 && key != IPC_PRIVATE && errno == EACCES);

	/* Report the error if we eventually failed */
	if (id < 0) {
		printf("Couldn't create semaphore");
		return -1;
	}

	if (create) {
		/* Initialize semaphore */
		init.val = initial_value;
		semctl(id, 0, SETVAL, init);
	}
	return id;
}

void
GdDestroySemaphore(MWSEMAPHORE id)
{
	union semun dummy;

	dummy.val = 0;
	semctl(id, 0, IPC_RMID, dummy);
}

int
GdSemWait(MWSEMAPHORE id)
{
	int retval = MWSEM_OK;

tryagain:
	if (semop(id, op_wait, 1) < 0) {
		if (errno == EINTR)
			goto tryagain;
		printf("Semaphore operation error");
		retval = MWSEM_ERR;
	}
	return retval;
}

#if 0
int
GdSemTryWait(MWSEMAPHORE id)
{
	int retval = MWSEM_OK;

tryagain:
	if (semop(id, op_trywait, 1) < 0) {
		if (errno == EINTR)
			goto tryagain;
		retval = MWSEM_TIMEDOUT;
	}
	return retval;
}

int
GdSemWaitTimeout(MWSEMAPHORE id, unsigned int timeout)
{
	int retval;

	/* Try the easy cases first */
	if (timeout == 0)
		return GdSemTryWait(id);
	if (timeout == MWSEM_MAXWAIT)
		return GdSemWait(id);

	/* We have to busy wait... */
	timeout += GsGetTickCount();
	do {
		retval = GdSemTryWait(id);
		if (retval == MWSEM_OK)
			break;
		GsDelay(1);
	} while (GsGetTickCount() < timeout);

	return retval;
}

int
GdSemValue(MWSEMAPHORE id)
{
	int semval;
	int value;
	
	value = 0;
  tryagain:
	semval = semctl(id, 0, GETVAL);
	if (semval < 0) {
		if (errno == EINTR)
			goto tryagain;
	} else {
		value = semval;
	}
	return value;
}
#endif

int
GdSemPost(MWSEMAPHORE id)
{
	int retval = MWSEM_OK;

tryagain:
	if (semop(id, op_post, 1) < 0) {
		if (errno == EINTR)
			goto tryagain;
		printf("Semaphore operation error");
		retval = MWSEM_ERR;
	}
	return retval;
}
