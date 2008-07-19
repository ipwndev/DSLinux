/* c_size.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/* If the remote host supports the SIZE command, we can find out the exact
 * size of a remote file, depending on the transfer type in use.  SIZE
 * returns different values for ascii and binary modes!
 */
int
FTPFileSize(const FTPCIPtr cip, const char *const file, longest_int *const size, const int type)
{
	int result;
	ResponsePtr rp;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if ((size == NULL) || (file == NULL))
		return (kErrBadParameter);
	*size = kSizeUnknown;

	result = FTPSetTransferType(cip, type);
	if (result < 0)
		return (result);

	if (cip->hasSIZE == kCommandNotAvailable) {
		cip->errNo = kErrSIZENotAvailable;
		result = kErrSIZENotAvailable;
	} else {
		rp = InitResponse();
		if (rp == NULL) {
			result = kErrMallocFailed;
			cip->errNo = kErrMallocFailed;
			FTPLogError(cip, kDontPerror, "Malloc failed.\n");
		} else {
			result = RCmd(cip, rp, "SIZE %s", file);
			if (result < 0) {
				DoneWithResponse(cip, rp);
				return (result);
			} else if (result == 2) {
#if defined(HAVE_LONG_LONG) && defined(SCANF_LONG_LONG)
				(void) sscanf(rp->msg.first->line, SCANF_LONG_LONG, size);
#elif defined(HAVE_LONG_LONG) && defined(HAVE_STRTOQ)
				*size = (longest_int) strtoq(rp->msg.first->line, NULL, 0);
#else
				(void) sscanf(rp->msg.first->line, "%ld", size);
#endif
				cip->hasSIZE = kCommandAvailable;
				result = kNoErr;
			} else if (FTP_UNIMPLEMENTED_CMD(rp->code)) {
				cip->hasSIZE = kCommandNotAvailable;
				cip->errNo = kErrSIZENotAvailable;
				result = kErrSIZENotAvailable;
			} else {
				cip->errNo = kErrSIZEFailed;
				result = kErrSIZEFailed;
			}
			DoneWithResponse(cip, rp);
		}
	}
	return (result);
}	/* FTPFileSize */




longest_int
FTPLocalASCIIFileSize(const char *const fn, char *buf, const size_t bufsize)
{
	char *tbuf = NULL;
	int c, prevc;
	longest_int asize;
	const char *scp, *slim;
	int fd;
	read_return_t nread;
	int oerrno;

	if (buf == NULL) {
		tbuf = (char *) malloc(bufsize);
		if (tbuf == NULL)
			return ((longest_int) -1);
		buf = tbuf;
	}

	fd = Open(fn, O_RDONLY, 00666);
	if (fd < 0) {
		if (tbuf != NULL)
			free(tbuf);
		return ((longest_int) -1);
	}

	prevc = 0;
	for (asize = (longest_int) 0; ; asize += (longest_int) nread) {
		nread = read(fd, buf, bufsize);
		if (nread < 0) {
			oerrno = errno;
			(void) close(fd);
			if (tbuf != NULL)
				free(tbuf);
			errno = oerrno;
			return ((longest_int) -1);
		} else if (nread == 0) {
			break;
		}

		for (scp = buf, slim = buf + nread; scp < slim; ) {
			c = *scp++;
			if ((c == '\n') && (prevc != '\r')) {
				/* When we actually use
				 * the file later, we translate
				 * newlines into newline+carriage return,
				 * so add one to the count for the
				 * CR byte we'll add then.
				 */
				nread++;
			}
			prevc = c;
		}
	}

	if (tbuf != NULL)
		free(tbuf);
	(void) close(fd);
	return (asize);
}	/* FTPLocalASCIIFileSize */



