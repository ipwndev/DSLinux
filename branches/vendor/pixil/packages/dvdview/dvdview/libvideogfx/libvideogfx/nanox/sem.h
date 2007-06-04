/* sem.h*/

#define MWSEM_OK	0
#define MWSEM_ERR	(-1)
#define MWSEM_TIMEDOUT	1	/* time out while waiting on semaphore*/
#define MWSEM_MAXWAIT	(~0)	/* timeout value to never time out*/

typedef int	MWSEMAPHORE;		/* Sys V IPC semaphore ID*/

#define GetServerSemaphore()	GdCreateSemaphore(1,1,1)
#define GetClientSemaphore()	GdCreateSemaphore(1,0,0)
#define LockRegion(id)		GdSemWait(id)
#define UnlockRegion(id)	GdSemPost(id)

MWSEMAPHORE	GdCreateSemaphore(int public, int create, int initial_value);
void		GdDestroySemaphore(MWSEMAPHORE id);
int		GdSemTryWait(MWSEMAPHORE id);
int		GdSemWait(MWSEMAPHORE id);
int		GdSemWaitTimeout(MWSEMAPHORE id, unsigned int timeout);
int		GdSemValue(MWSEMAPHORE id);
int		GdSemPost(MWSEMAPHORE id);
