/* rcmd.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#if !defined(NO_SIGNALS) && (USE_SIO || !defined(SIGALRM) || !defined(SIGPIPE) || !defined(SIGINT))
#	define NO_SIGNALS 1
#endif



/* A 'Response' parameter block is simply zeroed to be considered init'ed. */
ResponsePtr
InitResponse(void)
{
	ResponsePtr rp;
	
	rp = (ResponsePtr) calloc(SZ(1), sizeof(Response));
	if (rp != NULL)
		InitLineList(&rp->msg);
	return (rp);
}	/* InitResponse */




/* If we don't print it to the screen, we may want to save it to our
 * trace log.
 */
void
TraceResponse(const FTPCIPtr cip, ResponsePtr rp)
{
	FTPLinePtr lp;
	
	if (rp != NULL)	{
		lp = rp->msg.first;
		if (lp != NULL) {
			PrintF(cip, "%3d: %s\n", rp->code, lp->line);
			for (lp = lp->next; lp != NULL; lp = lp->next)
				PrintF(cip, "     %s\n", lp->line);
		}
	}
}	/* TraceResponse */





void
PrintResponse(const FTPCIPtr cip, FTPLineListPtr llp)
{
	FTPLinePtr lp;
	
	if (llp != NULL) {
		for (lp = llp->first; lp != NULL; lp = lp->next)
			PrintF(cip, "%s\n", lp->line);
	}
}	/* PrintResponse */





static void
SaveLastResponse(const FTPCIPtr cip, ResponsePtr rp)
{
	if (rp == NULL) {
		cip->lastFTPCmdResultStr[0] = '\0';
		cip->lastFTPCmdResultNum = -1;
		DisposeLineListContents(&cip->lastFTPCmdResultLL);
	} else if ((rp->msg.first == NULL) || (rp->msg.first->line == NULL)) {
		cip->lastFTPCmdResultStr[0] = '\0';
		cip->lastFTPCmdResultNum = rp->code;
		DisposeLineListContents(&cip->lastFTPCmdResultLL);
	} else {
		(void) STRNCPY(cip->lastFTPCmdResultStr, rp->msg.first->line);
		cip->lastFTPCmdResultNum = rp->code;

		/* Dispose previous command's line list. */
		DisposeLineListContents(&cip->lastFTPCmdResultLL);

		/* Save this command's line list. */
		cip->lastFTPCmdResultLL = rp->msg;
	}
}	/* SaveLastResponse */



void
DoneWithResponse(const FTPCIPtr cip, ResponsePtr rp)
{
	/* Dispose space taken up by the Response, and clear it out
	 * again.  For some reason, I like to return memory to zeroed
	 * when not in use.
	 */
	if (rp != NULL) {
		TraceResponse(cip, rp);
		if (cip->printResponseProc != 0) {
			if ((rp->printMode & kResponseNoProc) == 0)
				(*cip->printResponseProc)(cip, rp);
		}
		if ((rp->printMode & kResponseNoSave) == 0)
			SaveLastResponse(cip, rp);
		else
			DisposeLineListContents(&rp->msg);
		(void) memset(rp, 0, sizeof(Response));
		free(rp);
	}
}	/* DoneWithResponse */




/* This takes an existing Response and recycles it, by clearing out
 * the current contents.
 */
void
ReInitResponse(const FTPCIPtr cip, ResponsePtr rp)
{
	if (rp != NULL) {
		TraceResponse(cip, rp);
		if (cip->printResponseProc != 0) {
			if ((rp->printMode & kResponseNoProc) == 0)
				(*cip->printResponseProc)(cip, rp);
		}
		if ((rp->printMode & kResponseNoSave) == 0)
			SaveLastResponse(cip, rp);
		else
			DisposeLineListContents(&rp->msg);
		(void) memset(rp, 0, sizeof(Response));
	}
}	/* ReInitResponse */



/* Returns 0 if a response was read, or (-1) if an error occurs.
 * This reads the entire response text into a FTPLineList, which is kept
 * in the 'Response' structure.
 */
int
GetResponse(const FTPCIPtr cip, ResponsePtr rp)
{
	longstring str;
	int eofError;
	str16 code;
	char *cp;
	int continuation;
	volatile FTPCIPtr vcip;
	int result;

	/* RFC 959 states that a reply may span multiple lines.  A single
	 * line message would have the 3-digit code <space> then the msg.
	 * A multi-line message would have the code <dash> and the first
	 * line of the msg, then additional lines, until the last line,
	 * which has the code <space> and last line of the msg.
	 *
	 * For example:
	 *	123-First line
	 *	Second line
	 *	234 A line beginning with numbers
	 *	123 The last line
	 */

	vcip = cip;

	cp = str;
	eofError = 0;
	for (;;) {
		if (cip->dataTimedOut > 0) {
			/* Give up immediately unless the server had already
			 * sent a message. Odds are since the data is timed
			 * out, so is the control.
			 */
			if (SWaitUntilReadyForReading(cip->ctrlSocketR, 0) == 0) {
				/* timeout */
				FTPLogError(cip, kDontPerror, "Could not read reply from control connection -- timed out.\n");
				FTPShutdownHost(vcip);
				cip->errNo = kErrControlTimedOut;
				return (cip->errNo);
			}
		}
		result = SReadline(&cip->ctrlSrl, str, sizeof(str) - 1);
		if (result == kTimeoutErr) {
			/* timeout */
			FTPLogError(cip, kDontPerror, "Could not read reply from control connection -- timed out.\n");
			FTPShutdownHost(vcip);
			cip->errNo = kErrControlTimedOut;
			return (cip->errNo);
		} else if (result == 0) {
			/* eof */
			eofError = 1;
			rp->hadEof = 1;
			if (rp->eofOkay == 0)
				FTPLogError(cip, kDontPerror, "Remote host has closed the connection.\n");
			FTPShutdownHost(vcip);
			cip->errNo = kErrRemoteHostClosedConnection;
			return (cip->errNo);
		} else if (result < 0) {
			/* error */
			FTPLogError(cip, kDoPerror, "Could not read reply from control connection");
			FTPShutdownHost(vcip);
			cip->errNo = kErrInvalidReplyFromServer;
			return (cip->errNo);
		}

		if ((str[0] == '\n') || (str[0] == '\0')) {
			/* Blank lines are violation of protocol, but try to be
			 * lenient with broken servers.
			 */
			FTPLogError(cip, kDontPerror, "Protocol violation by server: blank line on control.\n");
			continue;
		}
		if (str[result - 1] == '\n')
			str[result - 1] = '\0';
		else
			PrintF(cip, "Warning: Remote line was too long: [%s]\n", str);
		break;
	}

	if (!isdigit((int) *cp)) {
		FTPLogError(cip, kDontPerror, "Invalid reply: \"%s\"\n", cp);
		cip->errNo = kErrInvalidReplyFromServer;
		return (cip->errNo);
	}

	rp->codeType = *cp - '0';
	cp += 3;
	continuation = (*cp == '-');
	*cp++ = '\0';
	(void) STRNCPY(code, str);
	rp->code = atoi(code);
	(void) AddLine(&rp->msg, cp);
	if (eofError < 0) {
		/* Read reply, but EOF was there also. */
		rp->hadEof = 1;
	}
	
	while (continuation) {
		result = SReadline(&cip->ctrlSrl, str, sizeof(str) - 1);
		if (result == kTimeoutErr) {
			/* timeout */
			FTPLogError(cip, kDontPerror, "Could not read reply from control connection -- timed out.\n");
			FTPShutdownHost(vcip);
			cip->errNo = kErrControlTimedOut;
			return (cip->errNo);
		} else if (result == 0) {
			/* eof */
			eofError = 1;
			rp->hadEof = 1;
			if (rp->eofOkay == 0)
				FTPLogError(cip, kDontPerror, "Remote host has closed the connection.\n");
			FTPShutdownHost(vcip);
			cip->errNo = kErrRemoteHostClosedConnection;
			return (cip->errNo);
		} else if (result < 0) {
			/* error */
			FTPLogError(cip, kDoPerror, "Could not read reply from control connection");
			FTPShutdownHost(vcip);
			cip->errNo = kErrInvalidReplyFromServer;
			return (cip->errNo);
		}

		if (str[result - 1] == '\n')
			str[result - 1] = '\0';
		cp = str;
		if (strncmp(code, cp, SZ(3)) == 0) {
			cp += 3;
			if (*cp != '-')
				continuation = 0;
			++cp;
		}
		(void) AddLine(&rp->msg, cp);
	}

	if (rp->code == 421) {
		/*
		 *   421 Service not available, closing control connection.
		 *       This may be a reply to any command if the service knows it
		 *       must shut down.
		 */
		if (rp->eofOkay == 0)
			FTPLogError(cip, kDontPerror, "Remote host has closed the connection.\n");
		FTPShutdownHost(vcip);
		cip->errNo = kErrRemoteHostClosedConnection;
		return(cip->errNo);
	}

	(void) gettimeofday(&cip->lastCmdFinish, NULL);
	return (kNoErr);
}	/* GetResponse */




/* This creates the complete command text to send, and writes it
 * on the stream.
 */

int
FTPSendCommandStr(const FTPCIPtr cip, char *const command, const size_t siz)
{
	int result;
	size_t clen;
	char *cp;

	if (cip->ctrlSocketW == kClosedFileDescriptor) {
		cip->errNo = kErrNotConnected;
		return (kErrNotConnected);
	}

	clen = strlen(command);
	if (clen == 0)
		return (kErrBadParameter);

	cp = command + clen - 1;
	if (*cp == '\n') {
		*cp = '\0';
		if (clen <= 2)
			return (kErrBadParameter);
		if (*--cp == '\r') {
			*cp = '\0';
		} else {
			/* Don't send "Command\n", send
			 * FTP-compliant "Command\r\n" instead.
			 */
			++cp;
		}
	} else {
		cp++;
	}

	if ((strncmp(command, "PASS", SZ(4)) != 0) || ((strcmp(cip->user, "anonymous") == 0) && (cip->firewallType == kFirewallNotInUse)))
		PrintF(cip, "Cmd: %s\n", command);
	else
		PrintF(cip, "Cmd: %s\n", "PASS xxxxxxxx");

	if ((cp + 2) >= (command + siz - 1)) {
		/* Not enough room to add \r\n */
		return (kErrBadParameter);
	}
	/* Use TELNET end-of-line as mandated by FTP protocol. */
	*cp++ = '\r';
	*cp++ = '\n';
	*cp = '\0';

	cip->lastFTPCmdResultStr[0] = '\0';
	cip->lastFTPCmdResultNum = -1;

	result = SWrite(cip->ctrlSocketW, command, strlen(command), (int) cip->ctrlTimeout, 0);

	if (result < 0) {
		cip->errNo = kErrSocketWriteFailed;
		FTPLogError(cip, kDoPerror, "Could not write to control stream.\n");
		return (cip->errNo);
	}
	return (kNoErr);
}	/* FTPSendCommandStr */




int
FTPSendCommand(const FTPCIPtr cip, const char *const cmdspec, va_list ap)
{
	longstring command;

#ifdef HAVE_VSNPRINTF
	(void) vsnprintf(command, sizeof(command) - 1, cmdspec, ap);
	command[sizeof(command) - 1] = '\0';
#else
	(void) vsprintf(command, cmdspec, ap);
#endif
	return (FTPSendCommandStr(cip, command, sizeof(command)));
}	/* FTPSendCommand */




/* For "simple" (i.e. not data transfer) commands, this routine is used
 * to send the command and receive one response.  It returns the codeType
 * field of the 'Response' as the result, or a negative number upon error.
 */
/*VARARGS*/
int
FTPCmd(const FTPCIPtr cip, const char *const cmdspec, ...)
{
	va_list ap;
	int result;
	ResponsePtr rp;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	rp = InitResponse();
	if (rp == NULL) {
		result = kErrMallocFailed;
		cip->errNo = kErrMallocFailed;
		FTPLogError(cip, kDontPerror, "Malloc failed.\n");
		return (cip->errNo);
	}

	va_start(ap, cmdspec);
	result = FTPSendCommand(cip, cmdspec, ap);
	va_end(ap);
	if (result < 0) {
		return (result);
	}

	/* Get the response to the command we sent. */
	result = GetResponse(cip, rp);

	if (result == kNoErr) {
		result = rp->codeType;
	}
	DoneWithResponse(cip, rp);
	return (result);
}	/* FTPCmd */




/* This is for debugging the library -- don't use. */
/*VARARGS*/
int
FTPCmdNoResponse(const FTPCIPtr cip, const char *const cmdspec, ...)
{
	va_list ap;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	va_start(ap, cmdspec);
	(void) FTPSendCommand(cip, cmdspec, ap);
	va_end(ap);

	return (kNoErr);
}	/* FTPCmdNoResponse */




int
WaitResponse(const FTPCIPtr cip, unsigned int sec)
{
	int result;
	fd_set ss;
	struct timeval tv;
	int fd;

	fd = cip->ctrlSocketR;
	if (fd < 0)
		return (-1);
	MY_FD_ZERO(&ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message save
#pragma message disable trunclongint
#endif
	MY_FD_SET(fd, &ss);
#if defined(__DECC) || defined(__DECCXX)
#pragma message restore
#endif
	tv.tv_sec = (tv_sec_t) sec;
	tv.tv_usec = 0;
	do {
		errno = 0;
		result = select(fd + 1, SELECT_TYPE_ARG234 &ss, NULL, NULL, &tv);
	} while ((result < 0) && (errno == EINTR));
	return (result);
}	/* WaitResponse */




/* For "simple" (i.e. not data transfer) commands, this routine is used
 * to send the command and receive one response.  It returns the codeType
 * field of the 'Response' as the result, or a negative number upon error.
 */

/*VARARGS*/
int
RCmd(const FTPCIPtr cip, ResponsePtr rp, const char *cmdspec, ...)
{
	va_list ap;
	int result;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	va_start(ap, cmdspec);
	result = FTPSendCommand(cip, cmdspec, ap);
	va_end(ap);
	if (result < 0) {
		return (result);
	}

	/* Get the response to the command we sent. */
	result = GetResponse(cip, rp);

	if (result == kNoErr) {
		result = rp->codeType;
	}
	return (result);
}	/* RCmd */



/* Returns -1 if an error occurred, or 0 if not.
 * This differs from RCmd, which returns the code class of a response.
 */




int
FTPStartDataCmd3(const FTPCIPtr cip, const int netMode, const int type, const longest_int startPoint0, char *const cmdstr, const size_t cmdstrSize, const char *const variableCommandSpec, va_list ap)
{
	int result;
	int respCode;
	ResponsePtr rp;
	longest_int startPoint = startPoint0;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	result = FTPSetTransferType(cip, type);
	if (result < 0)
		return (result);

	/* Re-set the cancellation flag. */
	cip->cancelXfer = 0;
	cip->canceled = 0;
	cip->dataSocketConnected = 0;

	/* To transfer data, we do these things in order as specifed by
	 * the RFC.
	 * 
	 * First, we tell the other side to set up a data line.  This
	 * is done below by calling OpenDataConnection(), which sets up
	 * the socket.  When we do that, the other side detects a connection
	 * attempt, so it knows we're there.  Then tell the other side
	 * (by using listen()) that we're willing to receive a connection
	 * going to our side.
	 */

	if ((result = OpenDataConnection(cip, cip->dataPortMode)) < 0)
		goto done;

	/* If asked, attempt to start at a later position in the remote file. */
	if (startPoint != (longest_int) 0) {
		if (startPoint == kSizeUnknown) {
			startPoint = (longest_int) 0;
		} else if (FTPSetStartOffset(cip, startPoint) != 0) {
			/* REST failed */
			if ((cmdstr != NULL) && (ISTRNEQ(cmdstr, "STOR ", 5))) {
				memcpy(cmdstr, "APPE ", 5);
			} else {
				startPoint = (longest_int) 0;
			}
		}
	}
	cip->startPoint = startPoint;

	/* Now we tell the server what we want to do.  This sends the
	 * the type of transfer we want (RETR, STOR, LIST, etc) and the
	 * parameters for that (files to send, directories to list, etc).
	 */
	if ((cmdstr != NULL) && (cmdstr[0] != '\0')) {
		result = FTPSendCommandStr(cip, cmdstr, cmdstrSize);
	} else if ((variableCommandSpec != NULL) && (variableCommandSpec[0] != '\0')) {
		result = FTPSendCommand(cip, variableCommandSpec, ap);
	} else {
		result = kErrBadParameter;
	}
	if (result < 0) { 
		goto done;
	}

	/* Get the response to the transfer command we sent, to see if
	 * they can accomodate the request.  If everything went okay,
	 * we will get a preliminary response saying that the transfer
	 * initiation was successful and that the data is there for
	 * reading (for retrieves;  for sends, they will be waiting for
	 * us to send them something).
	 */
	rp = InitResponse();
	if (rp == NULL) {
		FTPLogError(cip, kDontPerror, "Malloc failed.\n");
		cip->errNo = kErrMallocFailed;
		result = cip->errNo;
		goto done;
	}

	result = GetResponse(cip, rp);
	if (result < 0)
		goto done;

	respCode = rp->codeType;
	DoneWithResponse(cip, rp);

	if ((respCode > 2) && (cmdstr != NULL) && (ISTRNEQ(cmdstr, "STOR ", 5))  && (startPoint != (longest_int) 0)) {
		/* Special case: REST + STOR is the same as APPE */
		(void) FTPCmd(cip, "REST 0");
		memcpy(cmdstr, "APPE ", 4 + 1);
		result = FTPSendCommandStr(cip, cmdstr, cmdstrSize);

		rp = InitResponse();
		if (rp == NULL) {
			FTPLogError(cip, kDontPerror, "Malloc failed.\n");
			cip->errNo = kErrMallocFailed;
			result = cip->errNo;
			goto done;
		}

		result = GetResponse(cip, rp);
		if (result < 0)
			goto done;

		respCode = rp->codeType;
		DoneWithResponse(cip, rp);
	}

	if (respCode > 2) {
		cip->errNo = kErrCouldNotStartDataTransfer;
		result = cip->errNo;
		goto done;
	}

	/* Now we accept the data connection that the other side is offering
	 * to us.  Then we can do the actual I/O on the data we want.
	 */
	cip->netMode = netMode;
	if ((result = AcceptDataConnection(cip)) < 0)
		goto done;

	cip->dataSocketConnected = 1;

	/* Close the half of the bidirectional pipe that we won't be using. */
	if (cip->shutdownUnusedSideOfSockets != 0)
		(void) shutdown(cip->dataSocket, ((netMode == kNetReading) ? 1 : 0));

	return (kNoErr);

done:
	(void) FTPEndDataCmd(cip, 0);
	return (result);
}	/* FTPStartDataCmd3 */




int
FTPStartDataCmd2(const FTPCIPtr cip, const int netMode, const int type, const longest_int startPoint, char *const cmdstr, const size_t cmdstrSize, const char *variableCommandSpec, ...)
{
	int result;
	va_list ap;

	if ((cmdstr != NULL) && (cmdstr[0] != '\0')) {
		memset(&ap, 0, sizeof(ap));
		result = FTPStartDataCmd3(cip, netMode, type, startPoint, cmdstr, cmdstrSize, "(not used)", ap);
	} else if ((variableCommandSpec != NULL) && (variableCommandSpec[0] != '\0')) {
		va_start(ap, variableCommandSpec);
		result = FTPStartDataCmd3(cip, netMode, type, startPoint, NULL, 0, variableCommandSpec, ap);
		va_end(ap);
	} else {
		result = kErrBadParameter;
	}

	return (result);
}	/* FTPStartDataCmd2 */




/*VARARGS*/
int
FTPStartDataCmd(const FTPCIPtr cip, const int netMode, const int type, const longest_int startPoint, const char *const cmdspec, ...)
{
	va_list ap;
	int rc;

	va_start(ap, cmdspec);
	rc = FTPStartDataCmd3(cip, netMode, type, startPoint, NULL, 0, cmdspec, ap);
	va_end(ap);
	return (rc);
}	/* FTPStartDataCmd */




void 
FTPAbortDataTransfer(const FTPCIPtr cip)
{
	ResponsePtr rp;
	int result;
	unsigned int abto1, abto2;
	int closed = 0;

	if (cip->dataSocket != kClosedFileDescriptor) {
		PrintF(cip, "Starting abort sequence.\n");
		cip->canceling = 1;
		FTPUpdateIOTimer(cip);
		SendTelnetInterrupt(cip);		/* Probably could get by w/o doing this. */

		result = FTPCmdNoResponse(cip, "ABOR");
		if (result != kNoErr) {
			/* Linger could cause close to block, so unset it. */
			(void) SetSocketLinger(cip->dataSocket, 0, 0);
			CloseDataConnection(cip);
			PrintF(cip, "Could not send abort command.\n");
			cip->canceling = 0;
			return;
		}

		if (cip->abortTimeout != 0) {
			if (cip->abortTimeout < 4) {
				abto1 = 1;
			} else if (cip->abortTimeout < 6) {
				abto1 = 3;
			} else {
				abto1 = 5;
			}
			abto2 = (unsigned int) cip->abortTimeout - abto1;
			if (abto2 == 0)
				abto2 = 1;
			result = WaitResponse(cip, abto1);
			if (result < 0) {
				/* Error received to ABOR */
				(void) SetSocketLinger(cip->dataSocket, 0, 0);
				CloseDataConnection(cip);
				PrintF(cip, "Error occurred while waiting for abort reply.\n");
				cip->canceling = 0;
				return;
			}
			if (result == 0) {
				(void) SetSocketLinger(cip->dataSocket, 0, 0);
				PrintF(cip, "No response received to abort request yet; closing data connection.\n");

				/* Maybe this will get their attention... */
				CloseDataConnection(cip);
				closed = 1;

				result = WaitResponse(cip, abto2);
				if (result <= 0) {
					/* Error or finished timeout to ABOR */
					PrintF(cip, "No response received to abort request yet; giving up.\n");
					cip->canceling = 0;
					return;
				}
			}
		}

		rp = InitResponse();
		if (rp == NULL) {
			FTPLogError(cip, kDontPerror, "Malloc failed.\n");
			cip->errNo = kErrMallocFailed;
			cip->canceling = 0;
			return;
		}

		/*
		 * In the first case, the server closes the data connection
		 * (if it is open) and responds with a 226 reply, indicating
		 * that the abort command was successfully processed.
		 */
		result = GetResponse(cip, rp);
		if (result < 0) {
			/* Shouldn't happen, and doesn't matter if it does. */
			if (closed == 0) {
				(void) SetSocketLinger(cip->dataSocket, 0, 0);
				CloseDataConnection(cip);
			}
			PrintF(cip, "Invalid response to abort request.\n");
			DoneWithResponse(cip, rp);
			cip->canceling = 0;
			return;
		}

		/*
		 * In the second case, the server aborts the FTP service in
		 * progress and closes the data connection, returning a 426
		 * reply to indicate that the service request terminated
		 * abnormally.  The server then sends a 226 reply,
		 * indicating that the abort command was successfully
		 * processed.
		 */
		if (rp->codeType == 4) {
			ReInitResponse(cip, rp);
			result = GetResponse(cip, rp);
			if (result < 0) {
				if (closed == 0) {
					(void) SetSocketLinger(cip->dataSocket, 0, 0);
					CloseDataConnection(cip);
				}
				PrintF(cip, "Invalid second abort reply.\n");
				DoneWithResponse(cip, rp);
				cip->canceling = 0;
				return;
			}
		}
		DoneWithResponse(cip, rp);
		cip->canceled = 1;

		/* A response to the abort request has been received.
		 * Now the only thing left to do is close the data
		 * connection, making sure to turn off linger mode
		 * since we don't care about straggling data bits.
		 */
		if (closed == 0) {
			(void) SetSocketLinger(cip->dataSocket, 0, 0);
			CloseDataConnection(cip);		/* Must close (by protocol). */
		}
		PrintF(cip, "Aborted successfully.\n");
	}
	cip->canceling = 0;
}	/* FTPAbortDataTransfer */




int
FTPEndDataCmd(const FTPCIPtr cip, int didXfer)
{
	int result;
	int respCode;
	ResponsePtr rp;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if (cip->canceled == 1) {
		/* Already read the post-transfer response. */
		return (kNoErr);
	}

	CloseDataConnection(cip);
	result = kNoErr;
	if (didXfer) {
		/* Get the response to the data transferred.  Most likely a message
		 * saying that the transfer completed succesfully.  However, if
		 * we tried to abort the transfer using ABOR, we will have a response
		 * to that command instead.
		 */
		rp = InitResponse();
		if (rp == NULL) {
			FTPLogError(cip, kDontPerror, "Malloc failed.\n");
			cip->errNo = kErrMallocFailed;
			result = cip->errNo;
			return (result);
		}
		result = GetResponse(cip, rp);
		if (result < 0)
			return (result);
		respCode = rp->codeType;
		DoneWithResponse(cip, rp);
		if (respCode != 2) {
			cip->errNo = kErrDataTransferFailed;
			result = cip->errNo;
		} else {
			result = kNoErr;
		}
	}
	return (result);
}	/* FTPEndDataCmd */




int
BufferGets(char *buf, size_t bufsize, int inStream, char *secondaryBuf, char **secBufPtr, char **secBufLimit, size_t secBufSize)
{
	int err;
	char *src;
	char *dst;
	char *dstlim;
	int len;
	int nr;
	int haveEof = 0;

	err = 0;
	dst = buf;
	dstlim = dst + bufsize - 1;		/* Leave room for NUL. */
	src = (*secBufPtr);
	for ( ; dst < dstlim; ) {
		if (src >= (*secBufLimit)) {
			/* Fill the buffer. */

/* Don't need to poll it here.  The routines that use BufferGets don't
 * need any special processing during timeouts (i.e. progress reports),
 * so go ahead and just let it block until there is data to read.
 */
			nr = (int) read(inStream, secondaryBuf, secBufSize);
			if (nr == 0) {
				/* EOF. */
				haveEof = 1;
				goto done;
			} else if (nr < 0) {
				/* Error. */
				err = -1;
				goto done;
			}
			(*secBufPtr) = secondaryBuf;
			(*secBufLimit) = secondaryBuf + nr;
			src = (*secBufPtr);
			if (nr < (int) secBufSize)
				src[nr] = '\0';
		}
		if (*src == '\r') {
			++src;
		} else {
			if (*src == '\n') {
				/* *dst++ = *src++; */	++src;
				goto done;
			}
			*dst++ = *src++;
		}
	}

done:
	(*secBufPtr) = src;
	*dst = '\0';
	len = (int) (dst - buf);
	if (err < 0)
		return (err);
	if ((len == 0) && (haveEof == 1))
		return (-1);
	return (len);	/* May be zero, if a blank line. */
}	/* BufferGets */

/* eof */
