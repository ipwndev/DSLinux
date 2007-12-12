/* io_put.c
 *
 * Copyright (c) 1996-2006 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#ifndef O_BINARY
	/* Needed for platforms using different EOLN sequence (i.e. DOS) */
#	ifdef _O_BINARY
#		define O_BINARY _O_BINARY
#	else
#		define O_BINARY 0
#	endif
#endif

static int
FTPASCIILocalFileSeek(const int fd, const longest_int howMuchToSkip, char *const inbuf, const size_t bufsize)
{
	int c;
	longest_int count = howMuchToSkip;
	longest_int startpos, endpos, backskip;
	const char *src = NULL, *srclim = NULL;
	read_return_t nread;

	if (count == 0)
		return (0);
	if (count < 0)
		return (-1);
	startpos = (longest_int) Lseek(fd, 0, SEEK_CUR);
	if (startpos == (longest_int) -1)
		return (-1);

	while (count > 0) {
		nread = read(fd, inbuf, (read_size_t) bufsize);
		if (nread <= 0) {
			count = -1;
			break;
		}
		for (src = inbuf, srclim = inbuf + nread; src < srclim; ) {
			c = (int) *src++;
			if (c == '\r') {
				/* Oh dear... what's this doing in here? */
				count = -1;
			} else if (c == '\n') {
				count -= 2;
				/* If count is now < 0, then the file
				 * was corrupt and cannot be resumed.
				 */
			} else {
				--count;
			}
			if (count <= 0)
				break;
		}
	}
	if (count < 0) {
		endpos = (longest_int) Lseek(fd, startpos, SEEK_SET);
		if (endpos != startpos)
			return (-2);
		return (-1);
	}

	if (src != NULL) {
		backskip = (longest_int) -(srclim - src);
		if (backskip != 0) {
			endpos = (longest_int) Lseek(fd, backskip,  SEEK_CUR);
			if (endpos == (longest_int) -1)
				return (-2);
		}
	}

	return (0);
}	/* FTPASCIILocalFileSeek */



static int
FTPPutBlock(
	const FTPCIPtr cip,
	const char *cp,
	write_size_t ntowrite
)
{
	write_return_t nwrote;
	int result = kNoErr;
	
	do {
		if (! WaitForRemoteOutput(cip)) {	/* could set cancelXfer */
			cip->errNo = result = kErrDataTimedOut;
			FTPLogError(cip, kDontPerror, "Remote write timed out.\n");
			return (result);
		}
		if (cip->cancelXfer > 0) {
			FTPAbortDataTransfer(cip);
			result = cip->errNo = kErrDataTransferAborted;
			return (result);
		}

		nwrote = (write_return_t) SWrite(cip->dataSocket, cp, (size_t) ntowrite, (int) cip->xferTimeout, kNoFirstSelect);
		if (nwrote < 0) {
			if (nwrote == kTimeoutErr) {
				cip->errNo = result = kErrDataTimedOut;
				FTPLogError(cip, kDontPerror, "Remote write timed out.\n");
			} else if (errno == EPIPE) {
				cip->errNo = result = kErrSocketWriteFailed;
				errno = EPIPE;
				FTPLogError(cip, kDoPerror, "Lost data connection to remote host.\n");
			} else if (errno == EINTR) {
				continue;
			} else {
				cip->errNo = result = kErrSocketWriteFailed;
				FTPLogError(cip, kDoPerror, "Remote write failed.\n");
			}
			(void) shutdown(cip->dataSocket, 2);
			return (result);
		}
		cp += nwrote;
		ntowrite -= (write_size_t) nwrote;
	} while (ntowrite != 0);
	FTPUpdateIOTimer(cip);

	return (result);
}	/* FTPPutBlock */




int
FTPPutOneF(
	const FTPCIPtr cip,
	const char *const file,
	const char *volatile dstfile,
	int xtype,
	const int fdtouse,
	const int appendflag,
	const char *volatile tmppfx,
	const char *volatile tmpsfx,
	const int resumeflag,
	const int deleteflag,
	const FTPConfirmResumeUploadProc resumeProc)
{
	char *buf, *cp;
	const char *cmd;
	const char *odstfile;
	const char *tdstfile;
	size_t bufSize;
	size_t l;
	int tmpResult, result, pbrc;
	read_return_t nread;
	volatile int fd;
	char dstfile2[512];
	char *src, *srclim, *dst;
	write_size_t ntowrite;
	char inbuf[256], crlf[4];
	int lastch_of_prev_block, lastch_of_cur_block;
	int fstatrc, statrc;
	longest_int startPoint = 0;
	longest_int localsize;
	struct Stat st;
	time_t mdtm;
	volatile int vzaction;
	int zaction = kConfirmResumeProcSaidBestGuess;
	int sameAsRemote;
	int skiprc;
	size_t skipbufsize;
	longstring cmdStr;
	
	if (cip->buf == NULL) {
		FTPLogError(cip, kDoPerror, "Transfer buffer not allocated.\n");
		cip->errNo = kErrNoBuf;
		return (cip->errNo);
	}

	cip->usingTAR = 0;
	if (fdtouse < 0) {
		fd = Open(file, O_RDONLY|O_BINARY, 0);
		if (fd < 0) {
			FTPLogError(cip, kDoPerror, "Cannot open local file %s for reading.\n", file);
			cip->errNo = kErrOpenFailed;
			return (cip->errNo);
		}
	} else {
		fd = fdtouse;
	}

	fstatrc = Fstat(fd, &st);
	if ((fstatrc == 0) && (S_ISDIR(st.st_mode))) {
		if (fdtouse < 0) {
			(void) close(fd);
		}
		FTPLogError(cip, kDontPerror, "%s is a directory.\n", (file != NULL) ? file : "that");
		cip->errNo = kErrOpenFailed;
		return (cip->errNo);
	}

	localsize = (longest_int) st.st_size;
	if ((xtype == kTypeAscii) && (file != NULL) && (file[0] != '\0')) {
		localsize = FTPLocalASCIIFileSize(file, cip->buf, cip->bufSize);
	}

	/* For Put, we can't recover very well if it turns out restart
	 * didn't work, so check beforehand.
	 */
	if ((resumeflag == kResumeYes) || (resumeProc != kNoFTPConfirmResumeUploadProc)) {
		FTPCheckForRestartModeAvailability(cip); 
	}

	odstfile = dstfile;
	if (((tmppfx != NULL) && (tmppfx[0] != '\0')) || ((tmpsfx != NULL) && (tmpsfx[0] != '\0'))) {
		cp = strrchr(dstfile, '/');
		if (cp == NULL)
			cp = strrchr(dstfile, '\\');
		if (cp == NULL) {
			(void) STRNCPY(dstfile2, ((tmppfx == NULL) ? "" : tmppfx));
			(void) STRNCAT(dstfile2, dstfile);
			(void) STRNCAT(dstfile2, ((tmpsfx == NULL) ? "" : tmpsfx));
		} else {
			cp++;
			l = (size_t) (cp - dstfile);
			(void) STRNCPY(dstfile2, dstfile);
			dstfile2[l] = '\0';	/* Nuke stuff after / */
			(void) STRNCAT(dstfile2, ((tmppfx == NULL) ? "" : tmppfx));
			(void) STRNCAT(dstfile2, cp);
			(void) STRNCAT(dstfile2, ((tmpsfx == NULL) ? "" : tmpsfx));
		}
		dstfile = dstfile2;
	}

	if (fdtouse < 0) {
		AutomaticallyUseASCIIModeDependingOnExtension(cip, dstfile, &xtype);
		mdtm = kModTimeUnknown;
		if ((cip->progress != (FTPProgressMeterProc) 0) || (resumeflag == kResumeYes) || (resumeProc != kNoFTPConfirmResumeUploadProc)) {
			(void) FTPFileSizeAndModificationTime(cip, dstfile, &startPoint, xtype, &mdtm);
		}

		if (appendflag == kAppendYes) {
			zaction = kConfirmResumeProcSaidAppend;
		} else if (
		/*		(cip->hasREST == kCommandNotAvailable) || We can now try to use APPE when REST is not availble. */
		/*		(xtype != kTypeBinary) || We can now resume in ASCII too. */
				(fstatrc < 0)
		) {
			zaction = kConfirmResumeProcSaidOverwrite;
		} else if (resumeflag == kResumeYes) {
			zaction = kConfirmResumeProcSaidBestGuess;
		} else {
			zaction = kConfirmResumeProcSaidOverwrite;
		}

		statrc = -1;
		if ((mdtm != kModTimeUnknown) || (startPoint != kSizeUnknown)) {
			/* Then we know the file exists.  We will
			 * ask the user what to do, if possible, below.
			 */
			statrc = 0;
		} else if ((resumeProc != kNoFTPConfirmResumeUploadProc) && (cip->hasMDTM != kCommandAvailable) && (cip->hasSIZE != kCommandAvailable)) {
			/* We already checked if the file had a filesize
			 * or timestamp above, but if the server indicated
			 * it did not support querying those directly,
			 * we now need to try to determine if the file
			 * exists in a few other ways.
			 */
			statrc = FTPFileExists2(cip, dstfile, 0, 0, 0, 1, 1);
		}

		sameAsRemote = 0;
		if (
			(resumeProc != kNoFTPConfirmResumeUploadProc) &&
			(statrc == 0)
		) {
			tdstfile = dstfile;
			zaction = (*resumeProc)(cip, file, (longest_int) st.st_size, st.st_mtime, &tdstfile, startPoint, mdtm, &startPoint);
			dstfile = tdstfile;
		}

		if (zaction == kConfirmResumeProcSaidCancel) {
			/* User wants to cancel this file and any
			 * remaining in batch.
			 */
			cip->errNo = kErrUserCanceled;
			return (cip->errNo);
		}

		if (zaction == kConfirmResumeProcSaidBestGuess) {
			if ((mdtm != kModTimeUnknown) && (st.st_mtime > (mdtm + 1))) {
				/* Local file is newer than remote,
				 * overwrite the remote file instead
				 * of trying to resume it.
				 *
				 * Note:  Add one second fudge factor
				 * for Windows' file timestamps being
				 * imprecise to one second.
				 */
				zaction = kConfirmResumeProcSaidOverwrite; 
			} else if (localsize == startPoint) {
				/* Already sent file, done. */
				zaction = kConfirmResumeProcSaidSkip; 
				sameAsRemote = 1;
			} else if ((startPoint != kSizeUnknown) && (localsize > startPoint)) {
				zaction = kConfirmResumeProcSaidResume; 
			} else {
				zaction = kConfirmResumeProcSaidOverwrite; 
			}
		}

		if (zaction == kConfirmResumeProcSaidSkip) {
			/* Nothing done, but not an error. */
			if (fdtouse < 0) {
				(void) close(fd);
			}
			if (deleteflag == kDeleteYes) {
				if (unlink(file) < 0) {
					cip->errNo = kErrLocalDeleteFailed;
					return (cip->errNo);
				}
			}
			if (sameAsRemote != 0) {
				cip->errNo = kErrRemoteSameAsLocal;
				return (cip->errNo);
			}
			return (kNoErr);
		} else if (zaction == kConfirmResumeProcSaidResume) {
			/* Resume; proc set the startPoint. */
			if (localsize == startPoint) {
				/* Already sent file, done. */
				if (fdtouse < 0) {
					(void) close(fd);
				}

				if (deleteflag == kDeleteYes) {
					if (unlink(file) < 0) {
						cip->errNo = kErrLocalDeleteFailed;
						return (cip->errNo);
					}
				}
				return (kNoErr);
			} else if (xtype == kTypeAscii) {
				skipbufsize = cip->bufSize;
				if ((fdtouse >= 0) && (skipbufsize > 4096))
					skipbufsize = 4096;

				skiprc = FTPASCIILocalFileSeek(fd, startPoint, cip->buf, skipbufsize);
				if (skiprc == -2) {
					cip->errNo = kErrAsciiSeekErr;
					return (cip->errNo);
				} else if (skiprc == -1) {
					/* Overwrite */
					cip->startPoint = startPoint = 0;
				} else {
					/* It worked */
					/* We could do: cip->startPoint = startPoint;
					 * and let it do a REST with STOR,
					 * but it is safer to use APPE only.
					 */
					zaction = kConfirmResumeProcSaidAppend;
					cip->startPoint = startPoint = 0;
				}
			} else if (Lseek(fd, startPoint, SEEK_SET) != -1) {
				cip->startPoint = startPoint;
			}
		} else if (zaction == kConfirmResumeProcSaidAppend) {
			/* append: leave startPoint at zero, we will append everything. */
			cip->startPoint = startPoint = 0;
		} else /* if (zaction == kConfirmResumeProcSaidOverwrite) */ {
			/* overwrite: leave startPoint at zero */
			cip->startPoint = startPoint = 0;
		}
	} else if (appendflag == kAppendYes) {
		zaction = kConfirmResumeProcSaidAppend;
	}

	FTPSetUploadSocketBufferSize(cip);

	vzaction = zaction;
	if (vzaction == kConfirmResumeProcSaidAppend) {
		cmd = "APPE ";
		tmppfx = "";	/* Can't use that here. */
		tmpsfx = "";
	} else {
		cmd = "STOR ";
		if (tmppfx == NULL)
			tmppfx = "";
		if (tmpsfx == NULL)
			tmpsfx = "";
	}

	memcpy(cmdStr, cmd, 4 + 1 /* space */ + 1 /* nul byte */);
	STRNCAT(cmdStr, dstfile);

	tmpResult = FTPStartDataCmd2(
		cip,
		kNetWriting,
		xtype,
		startPoint,
		cmdStr,
		sizeof(cmdStr),
		"(not used)"
	);

	if (tmpResult < 0) {
		cip->errNo = tmpResult;
		if (fdtouse < 0) {
			(void) close(fd);
		}
		return (cip->errNo);
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
			return (cip->errNo);
		}
		startPoint = 0;
	}

	result = kNoErr;
	buf = cip->buf;
	bufSize = cip->bufSize;

	FTPInitIOTimer(cip);
	if ((fstatrc == 0) && (S_ISREG(st.st_mode) != 0)) {
		cip->expectedSize = st.st_size;
		cip->mdtm = st.st_mtime;
	}
	cip->lname = file;	/* could be NULL */
	cip->rname = odstfile;
	if (fdtouse >= 0)
		cip->useProgressMeter = 0;
	FTPStartIOTimer(cip);

	/* Note: On Windows, we don't have to do anything special
	 * for ASCII mode, since Net ASCII's end-of-line sequence
	 * corresponds to the same thing used for DOS/Windows.
	 */

	if (xtype == kTypeAscii) {
		/* ascii */

		if (cip->asciiTranslationMode == kAsciiTranslationModeNone) {
			/* Warning: this is really just for testing FTP server software.
			 * Do not use this mode for your FTP client program!
			 */
			for (;;) {
				nread = read(fd, inbuf, (read_size_t) sizeof(inbuf));
				if (nread < 0) {
					if (errno == EINTR) {
						continue;
					} else {
						result = kErrReadFailed;
						cip->errNo = kErrReadFailed;
						FTPLogError(cip, kDoPerror, "Local read failed.\n");
					}
					break;
				} else if (nread == 0) {
					break;
				}
				cip->bytesTransferred += (longest_int) nread;
	
				ntowrite = (write_size_t) nread;
				cp = inbuf;
	
				if ((pbrc = FTPPutBlock(cip, cp, ntowrite)) < 0) {
					result = pbrc;
					goto brk;
				}
			}
		} else if ((cip->asciiTranslationMode == kAsciiTranslationModeStripCRs) || (cip->asciiTranslationMode == kAsciiTranslationModeFixEOLNs)) {
			/* TO-DO: Really add the fix mode, kAsciiTranslationModeFixEOLNs. */
			lastch_of_prev_block = 0;
			for (;;) {
				nread = read(fd, inbuf, (read_size_t) sizeof(inbuf));
				if (nread < 0) {
					if (errno == EINTR) {
						continue;
					} else {
						result = kErrReadFailed;
						cip->errNo = kErrReadFailed;
						FTPLogError(cip, kDoPerror, "Local read failed.\n");
					}
					break;
				} else if (nread == 0) {
					break;
				}
				cip->bytesTransferred += (longest_int) nread;
				
				src = inbuf;
				srclim = src + nread;
				lastch_of_cur_block = srclim[-1];
				if (lastch_of_cur_block == '\r') {
					srclim[-1] = '\0';
					srclim--;
					nread--;
					if (nread == 0) {
						lastch_of_prev_block = lastch_of_cur_block;
						break;
					}
				}
				dst = cip->buf;		/* must be 2x sizeof inbuf or more. */
	
				if (*src == '\n') {
					src++;
					*dst++ = '\r';
					*dst++ = '\n';
				} else if (lastch_of_prev_block == '\r') {
					/* Raw CR at end of last block,
					 * no LF at the start of this block.
					 */
					*dst++ = '\r';
					*dst++ = '\n';
				}
	
				/* Prepare the buffer, converting end-of-lines
				 * to CR+LF format as required by protocol.
				 */
				while (src < srclim) {
					if (*src == '\r') {
						if (src[1] == '\n') {
							/* CR+LF pair */
							*dst++ = *src++;
							*dst++ = *src++;
						} else {
							/* raw CR */
							*dst++ = *src++;
							*dst++ = '\n';
						}
					} else if (*src == '\n') {
						/* LF only; expected for UNIX text. */
						*dst++ = '\r';
						*dst++ = *src++;
					} else {
						*dst++ = *src++;
					}
				}
				lastch_of_prev_block = lastch_of_cur_block;
	
				ntowrite = (write_size_t) (dst - cip->buf);
				cp = cip->buf;
	
				if ((pbrc = FTPPutBlock(cip, cp, ntowrite)) < 0) {
					result = pbrc;
					goto brk;
				}
			}
	
			if (lastch_of_prev_block == '\r') {
				/* Very rare, but if the file's last byte is a raw CR
				 * we need to write out one more line since we
				 * skipped it earlier.
				 */
				crlf[0] = '\r';
				crlf[1] = '\n';
				crlf[2] = '\0';
				cp = crlf;
				ntowrite = 2;
	
				if ((pbrc = FTPPutBlock(cip, cp, ntowrite)) < 0) {
					result = pbrc;
					goto brk;
				}
			}
		}
	} else {
		/* binary */
		for (;;) {
			cp = buf;
			nread = read(fd, cp, (read_size_t) bufSize);
			if (nread < 0) {
				if (errno == EINTR) {
					continue;
				} else {
					result = kErrReadFailed;
					cip->errNo = kErrReadFailed;
					FTPLogError(cip, kDoPerror, "Local read failed.\n");
				}
				break;
			} else if (nread == 0) {
				break;
			}
			cip->bytesTransferred += (longest_int) nread;
	
			ntowrite = (write_size_t) nread;
			if ((pbrc = FTPPutBlock(cip, cp, ntowrite)) < 0) {
				result = pbrc;
				goto brk;
			}
		}
	}
brk:

	/* This looks very bizarre, since
	 * we will be checking the socket
	 * for readability here!
	 *
	 * The reason for this is that we
	 * want to be able to timeout a
	 * small put.  So, we close the
	 * write end of the socket first,
	 * which tells the server we're
	 * done writing.  We then wait
	 * for the server to close down
	 * the whole socket (we know this
	 * when the socket is ready for
	 * reading an EOF), which tells
	 * us that the file was completed.
	 */
	(void) shutdown(cip->dataSocket, 1);
	(void) WaitForRemoteInput(cip);

	tmpResult = FTPEndDataCmd(cip, 1);
	if ((tmpResult < 0) && (result == kNoErr)) {
		cip->errNo = result = kErrSTORFailed;
	}
	FTPStopIOTimer(cip);

	if (fdtouse < 0) {
		/* If they gave us a descriptor (fdtouse >= 0),
		 * leave it open, otherwise we opened it, so
		 * we need to dispose of it.
		 */
		(void) Fstat(fd, &st);
		(void) close(fd);
		fd = -1;
	}

	if (result == kNoErr) {
		/* The store succeeded;  If we were
		 * uploading to a temporary file,
		 * move the new file to the new name.
		 */
		cip->numUploads++;

		if ((tmppfx[0] != '\0') || (tmpsfx[0] != '\0')) {
			if ((result = FTPRename(cip, dstfile, odstfile)) < 0) {
				/* May fail if file was already there,
				 * so delete the old one so we can move
				 * over it.
				 */
				if (FTPDelete(cip, odstfile, kRecursiveNo, kGlobNo) == kNoErr) {
					result = FTPRename(cip, dstfile, odstfile);
					if (result < 0) {
						FTPLogError(cip, kDontPerror, "Could not rename %s to %s: %s.\n", dstfile, odstfile, FTPStrError(cip->errNo));
					}
				} else {
					FTPLogError(cip, kDontPerror, "Could not delete old %s, so could not rename %s to that: %s\n", odstfile, dstfile, FTPStrError(cip->errNo));
				}
			}
		}

		if (FTPUtime(cip, odstfile, st.st_atime, st.st_mtime, st.st_ctime) != kNoErr) {
			if (cip->errNo != kErrUTIMENotAvailable)
				FTPLogError(cip, kDontPerror, "Could not preserve times for %s: %s.\n", odstfile, FTPStrError(cip->errNo));
		}

		if ((result == kNoErr) && (deleteflag == kDeleteYes)) {
			if (unlink(file) < 0) {
				result = cip->errNo = kErrLocalDeleteFailed;
			}
		}
	}

	return (result);
}	/* FTPPutOneF */
