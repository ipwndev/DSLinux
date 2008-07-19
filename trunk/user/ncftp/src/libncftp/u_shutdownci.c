/* u_shutdownci.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif



void
FTPShutdownHost(const FTPCIPtr cip)
{
#if defined(SIGPIPE) && !defined(NO_SIGNALS)
	FTPSigProc osigpipe;
#endif

	if (cip == NULL)
		return;
	if (strcmp(cip->magic, kLibraryMagic))
		return;

#if defined(SIGPIPE) && !defined(NO_SIGNALS)
	osigpipe = signal(SIGPIPE, (FTPSigProc) SIG_IGN);
#endif

	/* Linger could cause close to block, so unset it. */
	if (cip->dataSocket != kClosedFileDescriptor)
		(void) SetSocketLinger(cip->dataSocket, 0, 0);
	CloseDataConnection(cip);	/* Shouldn't be open normally. */

	/* Linger should already be turned off for this. */
	FTPCloseControlConnection(cip);

	FTPDeallocateHost(cip);

#if defined(SIGPIPE) && !defined(NO_SIGNALS)
	(void) signal(SIGPIPE, (FTPSigProc) osigpipe);
#endif
}	/* FTPShutdownHost */
