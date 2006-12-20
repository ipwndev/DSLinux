#include <features.h>
#ifdef __UCLIBC_HAS_LFS__
#if defined _FILE_OFFSET_BITS && _FILE_OFFSET_BITS != 64 
#undef _FILE_OFFSET_BITS
#define	_FILE_OFFSET_BITS   64
#endif
#ifndef __USE_LARGEFILE64
# define __USE_LARGEFILE64	1
#endif
/* We absolutely do _NOT_ want interfaces silently
 * renamed under us or very bad things will happen... */
#ifdef __USE_FILE_OFFSET64
# undef __USE_FILE_OFFSET64
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "dirstream.h"


int readdir64_r(DIR *dir, struct dirent64 *entry, struct dirent64 **result)
{
	int ret;
	ssize_t bytes;
	struct dirent64 *de;

	if (!dir) {
	    __set_errno(EBADF);
	    return(EBADF);
	}
	de = NULL;

#ifdef __UCLIBC_HAS_THREADS__
	__pthread_mutex_lock(&(dir->dd_lock));
#endif

	do {
	    if (dir->dd_size <= dir->dd_nextloc) {
		/* read dir->dd_max bytes of directory entries. */
		bytes = __getdents64(dir->dd_fd, dir->dd_buf, dir->dd_max);
		if (bytes <= 0) {
		    *result = NULL;
		    ret = errno;
		    goto all_done;
		}
		dir->dd_size = bytes;
		dir->dd_nextloc = 0;
	    }

	    de = (struct dirent64 *) (((char *) dir->dd_buf) + dir->dd_nextloc);

	    /* Am I right? H.J. */
	    dir->dd_nextloc += de->d_reclen;

	    /* We have to save the next offset here. */
	    dir->dd_nextoff = de->d_off;
	    /* Skip deleted files.  */
	} while (de->d_ino == 0);

	if (de == NULL) {
	    *result = NULL;
	} else {
	    *result = memcpy (entry, de, de->d_reclen);
	}
	ret = 0;

all_done:

#ifdef __UCLIBC_HAS_THREADS__
	__pthread_mutex_unlock(&(dir->dd_lock));
#endif
        return((de != NULL)? 0 : ret);
}
#endif /* __UCLIBC_HAS_LFS__ */

