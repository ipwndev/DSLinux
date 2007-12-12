/* io_get.c
 *
 * Copyright (c) 1996-2006 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#if 0	/* For now, don't do this, which takes a shortcut. */
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	define ASCII_TRANSLATION 0
#endif
#endif

#ifndef ASCII_TRANSLATION
#	define ASCII_TRANSLATION 1
#endif

#ifndef NO_SIGNALS
#	define NO_SIGNALS 1
#endif

#ifndef O_BINARY
	/* Needed for platforms using different EOLN sequence (i.e. DOS) */
#	ifdef _O_BINARY
#		define O_BINARY _O_BINARY
#	else
#		define O_BINARY 0
#	endif
#endif

int
FTPGetOneF(
	const FTPCIPtr cip,
	const char *const file,
	const char *dstfile,
	int xtype,
	const int fdtouse,
	longest_int expectedSize,
	time_t mdtm,
	const int resumeflag,
	const int appendflag,
	const int deleteflag,
	const FTPConfirmResumeDownloadProc resumeProc)
{
	char *buf;
	size_t bufSize;
	int tmpResult;
	volatile int result;
	read_return_t nread;
	write_return_t nwrote;
	volatile int fd;
#if ASCII_TRANSLATION
	char *src, *srclim;
	char *dst, *dstlim;
	char outbuf[512];
	int cr, crcr, add_another_eoln, crcr_offset;
#endif
	longest_int tstartPoint;
	volatile longest_int startPoint = 0;
	struct utimbuf ut;
	struct Stat st;
#if !defined(NO_SIGNALS)
	volatile FTPSigProc osigpipe;
	volatile FTPCIPtr vcip;
	volatile int vfd, vfdtouse;
	int sj;
#endif	/* NO_SIGNALS */
	volatile int created = 0;
	int zaction = kConfirmResumeProcSaidBestGuess;
	int statrc;
	int noMdtmCheck;
	int is_dev;
	time_t now;

	if (cip->buf == NULL) {
		FTPLogError(cip, kDoPerror, "Transfer buffer not allocated.\n");
		cip->errNo = kErrNoBuf;
		return (cip->errNo);
	}

	result = kNoErr;
	cip->usingTAR = 0;

	if (fdtouse < 0) {
		/* Only ask for extended information
		 * if we have the name of the file
		 * and we didn't already have the
		 * info.
		 *
		 * Always ask for the modification time,
		 * because even if it was passed in it
		 * may not be accurate.  This is often
		 * the case when it came from an ls
		 * listing, in which the local time
		 * zone could be a factor.
		 *
		 */

		if ((file == NULL) || (file[0] == '\0') || (dstfile == NULL) || (dstfile[0] == '\0')) {
			return (kErrBadParameter);
		}

		AutomaticallyUseASCIIModeDependingOnExtension(cip, file, &xtype);

		if ((cip->progress != (FTPProgressMeterProc) 0) || (resumeflag == kResumeYes) || (resumeProc != kNoFTPConfirmResumeDownloadProc)) {
			if (expectedSize == kSizeUnknown) {
				(void) FTPFileSizeAndModificationTime(cip, file, &expectedSize, xtype, &mdtm);
			} else {
				(void) FTPFileModificationTime(cip, file, &mdtm);
			}
		}

		/* For Get, we can't recover very well if it turns out restart
		 * didn't work, so check beforehand.
		 */
		if ((resumeflag == kResumeYes) || (resumeProc != kNoFTPConfirmResumeDownloadProc)) {
			FTPCheckForRestartModeAvailability(cip); 
		}

		if (appendflag == kAppendYes) {
			zaction = kConfirmResumeProcSaidAppend;
		} else if (cip->hasREST == kCommandNotAvailable) {
			zaction = kConfirmResumeProcSaidOverwrite;
		} else if (resumeflag == kResumeYes) {
			zaction = kConfirmResumeProcSaidBestGuess;
		} else {
			zaction = kConfirmResumeProcSaidOverwrite;
		}

		is_dev = 0;
		statrc = Stat(dstfile, &st);
		if (statrc == 0) {
			if (resumeProc != NULL) {
				tstartPoint = startPoint;
				zaction = (*resumeProc)(
						cip,
						&dstfile,
						(longest_int) st.st_size,	/* displays local size, not the FTP CR+LF size for ASCII text */
						st.st_mtime,
						file,
						expectedSize,
						mdtm,
						&tstartPoint
				);
				startPoint = tstartPoint;
			}
#ifdef S_ISBLK
			if (S_ISBLK(st.st_mode))
				is_dev = 1;
#endif
#ifdef S_ISCHR
			if (S_ISCHR(st.st_mode))
				is_dev = 1;
#endif

			if (zaction == kConfirmResumeProcSaidBestGuess) {
				if (expectedSize != kSizeUnknown) {
					/* We know the size of the remote file,
					 * and we have a local file too.
					 *
					 * Try and decide if we need to get
					 * the entire file, or just part of it.
					 */

					zaction = kConfirmResumeProcSaidResume;
					startPoint = (longest_int) st.st_size;
					if (xtype == kTypeAscii) {
						if ((dstfile == NULL) || (dstfile[0] == '\0') || ((startPoint = FTPLocalASCIIFileSize(dstfile, cip->buf, cip->bufSize)) < 0))
							zaction = kConfirmResumeProcSaidOverwrite;
					}

					/* If the local file exists and has a recent
					 * modification time (< 12 hours) and
					 * the remote file's modtime is not recent,
					 * then heuristically conclude that the
					 * local modtime should not be trusted
					 * (i.e. user killed the process before
					 * the local modtime could be preserved).
					 */
					noMdtmCheck = 0;
					if (mdtm != kModTimeUnknown) {
						time(&now);
						if ((st.st_mtime > now) || (((now - st.st_mtime) < 46200) && ((now - mdtm) >= 46200)))
							noMdtmCheck = 1;
					}

					if ((mdtm == kModTimeUnknown) || (noMdtmCheck != 0)) {
						/* Can't use the timestamps as an aid. */
						if (startPoint == expectedSize) {
							/* Don't go to all the trouble of downloading nothing. */
							cip->errNo = kErrLocalSameAsRemote;
							if (deleteflag == kDeleteYes)
								(void) FTPDelete(cip, file, kRecursiveNo, kGlobNo);
							return (cip->errNo);
						} else if (startPoint > expectedSize) {
							/* Panic;  odds are the file we have
							 * was a different file altogether,
							 * since it is larger than the
							 * remote copy.  Re-do it all.
							 */
							zaction = kConfirmResumeProcSaidOverwrite;
						} /* else resume at startPoint */
					} else if ((mdtm == st.st_mtime) || (mdtm == (st.st_mtime - 1)) || (mdtm == (st.st_mtime + 1))) {
						/* File has the same time.
						 * Note: Windows' file timestamps can be off by one second!
						 */
						if (startPoint == expectedSize) {
							/* Don't go to all the trouble of downloading nothing. */
							cip->errNo = kErrLocalSameAsRemote;
							if (deleteflag == kDeleteYes)
								(void) FTPDelete(cip, file, kRecursiveNo, kGlobNo);
							return (cip->errNo);
						} else if (startPoint > expectedSize) {
							/* Panic;  odds are the file we have
							 * was a different file altogether,
							 * since it is larger than the
							 * remote copy.  Re-do it all.
							 */
							zaction = kConfirmResumeProcSaidOverwrite;
						} else {
							/* We have a file by the same time,
							 * but smaller start point.  Leave
							 * the startpoint as is since it
							 * is most likely valid.
							 */
						}
					} else if (mdtm < st.st_mtime) {
						/* Remote file is older than
						 * local file.  Don't overwrite
						 * our file.
						 */
						cip->errNo = kErrLocalFileNewer;
						return (cip->errNo);
					} else /* if (mdtm > st.st_mtime) */ {
						/* File has a newer timestamp
						 * altogether, assume the remote
						 * file is an entirely new file
						 * and replace ours with it.
						 */
						zaction = kConfirmResumeProcSaidOverwrite;
					}
				} else {
						zaction = kConfirmResumeProcSaidOverwrite;
				}
			}
		} else {
			zaction = kConfirmResumeProcSaidOverwrite;
		}

		if (zaction == kConfirmResumeProcSaidCancel) {
			/* User wants to cancel this file and any
			 * remaining in batch.
			 */
			cip->errNo = kErrUserCanceled;
			return (cip->errNo);
		} else if (zaction == kConfirmResumeProcSaidSkip) {
			/* Nothing done, but not an error. */
			if (deleteflag == kDeleteYes)
				(void) FTPDelete(cip, file, kRecursiveNo, kGlobNo);
			return (kNoErr);
		} else if (zaction == kConfirmResumeProcSaidResume) {
			/* Resume; proc set the startPoint. */
			if (startPoint == expectedSize) {
				/* Don't go to all the trouble of downloading nothing. */
				/* Nothing done, but not an error. */
				if (deleteflag == kDeleteYes)
					(void) FTPDelete(cip, file, kRecursiveNo, kGlobNo);
				return (kNoErr);
			} else if (startPoint > expectedSize) {
				/* Cannot set start point past end of remote file */
				cip->errNo = result = kErrSetStartPoint;
				return (result);
			}
			fd = Open(dstfile, is_dev ? O_WRONLY|O_BINARY : O_WRONLY|O_APPEND|O_BINARY, 00666);
		} else if (zaction == kConfirmResumeProcSaidAppend) {
			/* leave startPoint at zero, we will append everything. */
			startPoint = (longest_int) 0;
			fd = Open(dstfile, is_dev ? O_WRONLY|O_BINARY|O_APPEND : O_WRONLY|O_CREAT|O_APPEND|O_BINARY, 00666);
		} else /* if (zaction == kConfirmResumeProcSaidOverwrite) */ {
			created = 1;
			startPoint = (longest_int) 0;
			fd = Open(dstfile, is_dev ? O_WRONLY|O_BINARY : O_WRONLY|O_CREAT|O_TRUNC|O_BINARY, 00666);
		}

		if (fd < 0) {
			FTPLogError(cip, kDoPerror, "Cannot open local file %s for writing.\n", dstfile);
			result = kErrOpenFailed;
			cip->errNo = kErrOpenFailed;
			return (result);
		}

		if ((expectedSize == (longest_int) 0) && (startPoint <= (longest_int) 0) && (zaction != kConfirmResumeProcSaidOverwrite)) {
			/* Don't go to all the trouble of downloading nothing. */
#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
			/* Note: Windows doesn't allow zero-size files. */
			(void) write(fd, "\r\n", (write_size_t) 2);
#endif
			(void) close(fd);
			if (mdtm != kModTimeUnknown) {
				cip->mdtm = mdtm;
				(void) time(&ut.actime);
				ut.modtime = mdtm;
				(void) utime(dstfile, &ut);
			}
			if (deleteflag == kDeleteYes)
				(void) FTPDelete(cip, file, kRecursiveNo, kGlobNo);
			return (kNoErr);
		}
	} else {
		fd = fdtouse;
	}

	if ((cip->numDownloads == 0) && (cip->dataSocketRBufSize != 0)) {
		/* If dataSocketSBufSize is non-zero, it means you
		 * want to explicitly try to set the size of the
		 * socket's I/O buffer.
		 *
		 * If it is zero, it means you want to just use the
		 * TCP stack's default value, which is typically
		 * between 8 and 64 kB.
		 *
		 * If you try to set the buffer larger than 64 kB,
		 * the TCP stack should try to use RFC 1323 to
		 * negotiate "TCP Large Windows" which may yield
		 * significant performance gains.
		 */
		if (cip->hasSITE_RETRBUFSIZE == kCommandAvailable)
			(void) FTPCmd(cip, "SITE RETRBUFSIZE %lu", (unsigned long) cip->dataSocketRBufSize);
		else if (cip->hasSITE_RBUFSIZ == kCommandAvailable)
			(void) FTPCmd(cip, "SITE RBUFSIZ %lu", (unsigned long) cip->dataSocketRBufSize);
		else if (cip->hasSITE_RBUFSZ == kCommandAvailable)
			(void) FTPCmd(cip, "SITE RBUFSZ %lu", (unsigned long) cip->dataSocketRBufSize);
		else if (cip->hasSITE_BUFSIZE == kCommandAvailable)
			(void) FTPCmd(cip, "SITE BUFSIZE %lu", (unsigned long) cip->dataSocketSBufSize);
	}

#ifdef NO_SIGNALS
#else	/* NO_SIGNALS */
	vcip = cip;
	vfdtouse = fdtouse;
	vfd = fd;
	osigpipe = (volatile FTPSigProc) signal(SIGPIPE, BrokenData);

	gGotBrokenData = 0;
	gCanBrokenDataJmp = 0;

#ifdef HAVE_SIGSETJMP
	sj = sigsetjmp(gBrokenDataJmp, 1);
#else
	sj = setjmp(gBrokenDataJmp);
#endif	/* HAVE_SIGSETJMP */

	if (sj != 0) {
		(void) signal(SIGPIPE, (FTPSigProc) osigpipe);
		if (vfdtouse < 0) {
			(void) close(vfd);
		}
		FTPShutdownHost(vcip);
		vcip->errNo = kErrRemoteHostClosedConnection;
		return(vcip->errNo);
	}
	gCanBrokenDataJmp = 1;
#endif	/* NO_SIGNALS */

	tmpResult = FTPStartDataCmd(cip, kNetReading, xtype, startPoint, "RETR %s", file);

	if (tmpResult < 0) {
		result = tmpResult;
		if (result == kErrGeneric)
			result = kErrRETRFailed;
		cip->errNo = result;
		if (fdtouse < 0) {
			(void) close(fd);
			if ((created != 0) && (appendflag == kAppendNo) && (cip->startPoint == 0))
				(void) unlink(dstfile);
		}
#if !defined(NO_SIGNALS)
		(void) signal(SIGPIPE, (FTPSigProc) osigpipe);
#endif	/* NO_SIGNALS */
		return (result);
	}

	if ((startPoint != 0) && (cip->startPoint == 0)) {
		/* Remote could not or would not set the start offset
		 * to what we wanted.
		 *
		 * So now we have to undo our seek.
		 */
		if (Lseek(fd, 0, SEEK_SET) != 0) {
			cip->errNo = kErrLseekFailed;
			if (fdtouse < 0) {
				(void) close(fd);
			}
#if !defined(NO_SIGNALS)
			(void) signal(SIGPIPE, (FTPSigProc) osigpipe);
#endif	/* NO_SIGNALS */
			return (cip->errNo);
		}
		startPoint = 0;
	}

	buf = cip->buf;
	bufSize = cip->bufSize;

	FTPInitIOTimer(cip);
	cip->mdtm = mdtm;
	(void) time(&ut.actime);
	ut.modtime = mdtm;
	cip->expectedSize = expectedSize;
	cip->lname = dstfile;	/* could be NULL */
	cip->rname = file;
	if (fdtouse >= 0)
		cip->useProgressMeter = 0;
	FTPStartIOTimer(cip);

#if ASCII_TRANSLATION
	if (xtype == kTypeAscii) {
		cr = crcr = crcr_offset = add_another_eoln = 0;
		/* Ascii */
		for (;;) {
			if (! WaitForRemoteInput(cip)) {	/* could set cancelXfer */
				cip->errNo = result = kErrDataTimedOut;
				FTPLogError(cip, kDontPerror, "Remote read timed out.\n");
				break;
			}
			if (cip->cancelXfer > 0) {
				FTPAbortDataTransfer(cip);
				result = cip->errNo = kErrDataTransferAborted;
				break;
			}
#ifdef TESTING_ABOR
			if (cip->bytesTransferred > 0) {
				cip->cancelXfer = 1;
				FTPAbortDataTransfer(cip);
				result = cip->errNo = kErrDataTransferAborted;
				break;
			}
#endif /* TESTING_ABOR */
#ifdef NO_SIGNALS
			nread = (read_return_t) SRead(cip->dataSocket, buf, bufSize, (int) cip->xferTimeout, kFullBufferNotRequired|kNoFirstSelect);
			if (nread == kTimeoutErr) {
				cip->errNo = result = kErrDataTimedOut;
				FTPLogError(cip, kDontPerror, "Remote read timed out.\n");
				break;
			} else if (nread < 0) {
				if (errno == EPIPE) {
					result = cip->errNo = kErrSocketReadFailed;
					errno = EPIPE;
					FTPLogError(cip, kDoPerror, "Lost data connection to remote host.\n");
				} else if (errno == EINTR) {
					continue;
				} else {
					FTPLogError(cip, kDoPerror, "Remote read failed.\n");
					result = kErrSocketReadFailed;
					cip->errNo = kErrSocketReadFailed;
				}
				break;
			} else if (nread == 0) {
				break;
			}
#else
			gCanBrokenDataJmp = 1;
			if (cip->xferTimeout > 0)
				(void) alarm(cip->xferTimeout);
			nread = read(cip->dataSocket, buf, (read_size_t) bufSize);
			if (nread < 0) {
				if ((gGotBrokenData != 0) || (errno == EPIPE)) {
					result = cip->errNo = kErrSocketReadFailed;
					errno = EPIPE;
					FTPLogError(cip, kDoPerror, "Lost data connection to remote host.\n");
					(void) shutdown(cip->dataSocket, 2);
				} else if (errno == EINTR) {
					continue;
				} else {
					result = cip->errNo = kErrSocketReadFailed;
					FTPLogError(cip, kDoPerror, "Remote read failed.\n");
					(void) shutdown(cip->dataSocket, 2);
				}
				break;
			} else if (nread == 0) {
				break;
			}

			gCanBrokenDataJmp = 0;
#endif	/* NO_SIGNALS */

			src = buf;
			srclim = src + nread;
			dst = outbuf;
			dstlim = dst + sizeof(outbuf);
			while (src < srclim) {
				if (dst >= dstlim) {
					nwrote = write(fd, outbuf, (write_size_t) (dst - outbuf));
					if (nwrote == (write_return_t) (dst - outbuf)) {
						/* Success. */
						dst = outbuf;
					} else if (errno == EPIPE) {
						result = kErrWriteFailed;
						cip->errNo = kErrWriteFailed;
						errno = EPIPE;
						(void) shutdown(cip->dataSocket, 2);
						goto brk;
					} else {
						FTPLogError(cip, kDoPerror, "Local write failed.\n");
						result = kErrWriteFailed;
						cip->errNo = kErrWriteFailed;
						(void) shutdown(cip->dataSocket, 2);
						goto brk;
					}
				}

				/*
				 * The FTP protocol says we are to receive
				 * CR+LF as the end-of-line (EOLN) sequence for
				 * all files sent in ASCII mode.
				 *
				 * However, many servers only convert from
				 * their native EOLN format to CR+LF.
				 * For example, a Windows ISP being used
				 * to host Mac OS 9 text files (whose EOLN
				 * is just a CR) could violate protocol
				 * and send the file with the raw CRs only.
				 * It should be converting the file from CR
				 * to CR+LF for us, but many do not, so we
				 * try to convert raw CRs to our EOLN.
				 *
				 * We also look for raw LFs and convert them
				 * if needed.
				 */
				if (crcr != 0) {
					/* Leftover CR from previous buffer */
					crcr = 0;
					crcr_offset = 0;
					goto have_char_after_crcr;
				}
				if (cr != 0) {
					/* Leftover CR from previous buffer */
					cr = 0;
					goto have_char_after_cr;
				}

				if (*src == '\n') {
					/* protocol violation: raw LF */
					src++;
					goto add_eoln;
				}
				if (*src == '\r') {
					src++;
					if (src < srclim) {
have_char_after_cr:
						if (*src == '\n') {
							/* Normal case:
							 * CR+LF eoln.
							 */
							src++;
						} else if (*src == '\r') {
							/* Try to work-around
							 * server bug which
							 * sends CR+CR+LF
							 * eolns.
							 */
							crcr_offset = 1;
have_char_after_crcr:
							if ((src + crcr_offset) < srclim) {
								if (src[crcr_offset] == '\n') {
									src += 1 + crcr_offset;
								} else {
									/* CR+CR+some other character ==
									 * Two blank lines for systems
									 * that use CR only as EOLN.
									 */
									add_another_eoln++;
									src += crcr_offset;
								}
							} else {
								crcr = 1;
								src++;
								continue;
							}
						}
						/* else {protocol violation: raw CR} */
					} else {
						cr = 1;
						continue;
					}
add_eoln:
					if ((dst + 2) >= dstlim) {
						/* Write out buffer before
						 * adding one or two chars.
						 */
						nwrote = write(fd, outbuf, (write_size_t) (dst - outbuf));
						if (nwrote == (write_return_t) (dst - outbuf)) {
							/* Success. */
							dst = outbuf;
						} else if (errno == EPIPE) {
							result = kErrWriteFailed;
							cip->errNo = kErrWriteFailed;
							errno = EPIPE;
							(void) shutdown(cip->dataSocket, 2);
							goto brk;
						} else {
							FTPLogError(cip, kDoPerror, "Local write failed.\n");
							result = kErrWriteFailed;
							cip->errNo = kErrWriteFailed;
							(void) shutdown(cip->dataSocket, 2);
							goto brk;
						}
					}
					*dst++ = cip->textEOLN[0];
					if (cip->textEOLN[1] != '\0')
						*dst++ = cip->textEOLN[1];
					if (add_another_eoln != 0) {
						--add_another_eoln;
						goto add_eoln;
					}
					continue;
				}

				*dst++ = *src++;
			}
			if (dst > outbuf) {
				nwrote = write(fd, outbuf, (write_size_t) (dst - outbuf));
				if (nwrote != (write_return_t) (dst - outbuf)) {
					if (errno == EPIPE) {
						result = kErrWriteFailed;
						cip->errNo = kErrWriteFailed;
						errno = EPIPE;
						(void) shutdown(cip->dataSocket, 2);
						goto brk;
					} else {
						FTPLogError(cip, kDoPerror, "Local write failed.\n");
						result = kErrWriteFailed;
						cip->errNo = kErrWriteFailed;
						(void) shutdown(cip->dataSocket, 2);
						goto brk;
					}
				}
			}

			if (mdtm != kModTimeUnknown) {
				(void) utime(dstfile, &ut);
			}
			cip->bytesTransferred += (longest_int) nread;
			FTPUpdateIOTimer(cip);
		}
		if (crcr != 0)
			cr = 2;
		while (--cr >= 0) {
			/* Last block ended with a raw CR.
			 * Write out one last end-of-line.
			 */
			nwrote = write(fd, cip->textEOLN, (write_size_t) strlen(cip->textEOLN));
			if (nwrote == (write_return_t) strlen(cip->textEOLN)) {
				/* Success. */
				if (mdtm != kModTimeUnknown) {
					(void) utime(dstfile, &ut);
				}
			} else if (errno == EPIPE) {
				result = kErrWriteFailed;
				cip->errNo = kErrWriteFailed;
				errno = EPIPE;
				(void) shutdown(cip->dataSocket, 2);
				goto brk;
			} else {
				FTPLogError(cip, kDoPerror, "Local write failed.\n");
				result = kErrWriteFailed;
				cip->errNo = kErrWriteFailed;
				(void) shutdown(cip->dataSocket, 2);
				goto brk;
			}
		}
	} else
#endif	/* ASCII_TRANSLATION */
	{
		/* Binary */
		for (;;) {
			if (! WaitForRemoteInput(cip)) {	/* could set cancelXfer */
				cip->errNo = result = kErrDataTimedOut;
				FTPLogError(cip, kDontPerror, "Remote read timed out.\n");
				break;
			}
			if (cip->cancelXfer > 0) {
				FTPAbortDataTransfer(cip);
				result = cip->errNo = kErrDataTransferAborted;
				break;
			}
#ifdef TESTING_ABOR
			if (cip->bytesTransferred > 0) {
				cip->cancelXfer = 1;
				FTPAbortDataTransfer(cip);
				result = cip->errNo = kErrDataTransferAborted;
				break;
			}
#endif /* TESTING_ABOR */
#ifdef NO_SIGNALS
			nread = (read_return_t) SRead(cip->dataSocket, buf, bufSize, (int) cip->xferTimeout, kFullBufferNotRequired|kNoFirstSelect);
			if (nread == kTimeoutErr) {
				cip->errNo = result = kErrDataTimedOut;
				FTPLogError(cip, kDontPerror, "Remote read timed out.\n");
				break;
			} else if (nread < 0) {
				if (errno == EPIPE) {
					result = cip->errNo = kErrSocketReadFailed;
					errno = EPIPE;
					FTPLogError(cip, kDoPerror, "Lost data connection to remote host.\n");
				} else if (errno == EINTR) {
					continue;
				} else {
					FTPLogError(cip, kDoPerror, "Remote read failed.\n");
					result = kErrSocketReadFailed;
					cip->errNo = kErrSocketReadFailed;
				}
				break;
			} else if (nread == 0) {
				break;
			}
#else			
			gCanBrokenDataJmp = 1;
			if (cip->xferTimeout > 0)
				(void) alarm(cip->xferTimeout);
			nread = read(cip->dataSocket, buf, (read_size_t) bufSize);
			if (nread < 0) {
				if ((gGotBrokenData != 0) || (errno == EPIPE)) {
					result = cip->errNo = kErrSocketReadFailed;
					errno = EPIPE;
					FTPLogError(cip, kDoPerror, "Lost data connection to remote host.\n");
				} else if (errno == EINTR) {
					continue;
				} else {
					result = cip->errNo = kErrSocketReadFailed;
					FTPLogError(cip, kDoPerror, "Remote read failed.\n");
				}
				(void) shutdown(cip->dataSocket, 2);
				break;
			} else if (nread == 0) {
				break;
			}
			gCanBrokenDataJmp = 0;
#endif	/* NO_SIGNALS */

			nwrote = write(fd, buf, (write_size_t) nread);
			if (nwrote != nread) {
				if (errno == EPIPE) {
					result = kErrWriteFailed;
					cip->errNo = kErrWriteFailed;
					errno = EPIPE;
				} else {
					FTPLogError(cip, kDoPerror, "Local write failed.\n");
					result = kErrWriteFailed;
					cip->errNo = kErrWriteFailed;
				}
				(void) shutdown(cip->dataSocket, 2);
				break;
			}

			/* Ugggh... do this after each write operation
			 * so it minimizes the chance of a user killing
			 * the process before we reset the timestamps.
			 */
			if (mdtm != kModTimeUnknown) {
				(void) utime(dstfile, &ut);
			}
			cip->bytesTransferred += (longest_int) nread;
			FTPUpdateIOTimer(cip);
		}
	}

#if ASCII_TRANSLATION
brk:
#endif

#if !defined(NO_SIGNALS)
	if (cip->xferTimeout > 0)
		(void) alarm(0);
	gCanBrokenDataJmp = 0;
#endif	/* NO_SIGNALS */

	if (fdtouse < 0) {
		/* If they gave us a descriptor (fdtouse >= 0),
		 * leave it open, otherwise we opened it, so
		 * we need to close it.
		 */
		(void) close(fd);
		fd = -1;
	}

	tmpResult = FTPEndDataCmd(cip, 1);
	if ((tmpResult < 0) && (result == 0)) {
		result = kErrRETRFailed;
		cip->errNo = kErrRETRFailed;
	}
	FTPStopIOTimer(cip);
#if !defined(NO_SIGNALS)
	(void) signal(SIGPIPE, (FTPSigProc) osigpipe);
#endif	/* NO_SIGNALS */

	if (mdtm != kModTimeUnknown) {
		(void) utime(dstfile, &ut);
	}

	if (result == kNoErr) {
		cip->numDownloads++;

		if (deleteflag == kDeleteYes) {
			result = FTPDelete(cip, file, kRecursiveNo, kGlobNo);
		}
	}

	return (result);
}	/* FTPGetOneF */
