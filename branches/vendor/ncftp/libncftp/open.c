/* open.c
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
FTPDeallocateHost(const FTPCIPtr cip)
{
	/* Requires the cip->bufSize field set,
	 * and the cip->buf set if the
	 * buffer is allocated.
	 */
	if (cip->buf != NULL) {
		(void) memset(cip->buf, 0, cip->bufSize);
		if (cip->doAllocBuf != 0) {
			free(cip->buf);
			cip->buf = NULL;
		}
	}

	if (cip->startingWorkingDirectory != NULL) {
		free(cip->startingWorkingDirectory);
		cip->startingWorkingDirectory = NULL;
	}

#if USE_SIO
	DisposeSReadlineInfo(&cip->ctrlSrl);
#endif
	DisposeLineListContents(&cip->lastFTPCmdResultLL);
}	/* FTPDeallocateHost */




int
FTPAllocateHost(const FTPCIPtr cip)
{
	char *buf;

	/* Requires the cip->bufSize field set,
	 * and the cip->buf cleared if the
	 * buffer is not allocated.
	 */
	if (cip->buf == NULL) {
		if (cip->doAllocBuf == 0) {
			/* User must supply buffer! */
			cip->errNo = kErrBadParameter;
			return (kErrBadParameter);
		} else {
			buf = (char *) calloc((size_t) 1, cip->bufSize);
			if (buf == NULL) {
				FTPLogError(cip, kDontPerror, "Malloc failed.\n");
				cip->errNo = kErrMallocFailed;
				return (kErrMallocFailed);
			}
			cip->buf = buf;
		}
	} else {
		(void) memset(cip->buf, 0, cip->bufSize);
	}
	return (kNoErr);
}	/* FTPAllocateHost */




void
FTPInitializeAnonPassword(const FTPLIPtr lip)
{
	if (lip == NULL)
		return;
	if (strcmp(lip->magic, kLibraryMagic))
		return;

	if (lip->defaultAnonPassword[0] == '\0')
		(void) STRNCPY(lip->defaultAnonPassword, "NcFTP@");
}	/* FTPInitializeAnonPassword */




int
FTPLoginHost(const FTPCIPtr cip)
{
	ResponsePtr rp;
	int result = kErrLoginFailed;
	int anonLogin;
	int sentpass = 0, fwsentpass = 0;
	int fwloggedin;
	int firstTime;
	char cwd[512];

	if (cip == NULL)
		return (kErrBadParameter);
	if ((cip->firewallType < kFirewallNotInUse) || (cip->firewallType > kFirewallLastType))
		return (kErrBadParameter);

	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	anonLogin = 0;
	if (cip->user[0] == '\0')
		(void) STRNCPY(cip->user, "anonymous");
	if ((strcmp(cip->user, "anonymous") == 0) || (strcmp(cip->user, "ftp") == 0)) {
		anonLogin = 1;
		/* Try to get the email address if you didn't specify
		 * a password when the user is anonymous.
		 */
		if ((cip->pass[0] == '\0') && (cip->passIsEmpty == 0)) {
			FTPInitializeAnonPassword(cip->lip);
			(void) STRNCPY(cip->pass, cip->lip->defaultAnonPassword);
		}
	}

	rp = InitResponse();
	if (rp == NULL) {
		result = kErrMallocFailed;
		cip->errNo = kErrMallocFailed;
		goto done2;
	}

	for (firstTime = 1, fwloggedin = 0; ; ) {
		/* Here's a mini finite-automaton for the login process.
		 *
		 * Originally, the FTP protocol was designed to be entirely
		 * implementable from a FA.  It could be done, but I don't think
		 * it's something an interactive process could be the most
		 * effective with.
		 */

		if (firstTime != 0) {
			rp->code = 220;
			firstTime = 0;
		} else if (result < 0) {
			goto done;
		}

		switch (rp->code) {
			case 220:	/* Welcome, ready for new user. */
				if ((cip->firewallType == kFirewallNotInUse) || (fwloggedin != 0) || (fwsentpass != 0)) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s", cip->user);
				} else if (cip->firewallType == kFirewallUserAtSite) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s@%s", cip->user, cip->host);
				} else if (cip->firewallType == kFirewallUserAtSitePort) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s@%s:%u", cip->user, cip->host, cip->port);
				} else if (cip->firewallType == kFirewallUserAtSitePort2) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s@%s %u", cip->user, cip->host, cip->port);
				} else if (cip->firewallType == kFirewallUserAtUserPassAtPass) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s@%s@%s", cip->user, cip->firewallUser, cip->host);
				} else if (cip->firewallType == kFirewallUserAtSiteFwuPassFwp) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s@%s %s", cip->user, cip->host, cip->firewallUser);
				} else if (cip->firewallType == kFirewallFwuAtSiteFwpUserPass) {
					/* only reached when !fwloggedin */
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s@%s", cip->firewallUser, cip->host);
				} else if (cip->firewallType > kFirewallNotInUse) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s", cip->firewallUser);
				} else {
					goto unknown;
				}
				break;

			case 230:	/* 230 User logged in, proceed. */
			case 231:	/* User name accepted. */
			case 202:	/* Command not implemented, superfluous at this site. */
				if ((cip->firewallType == kFirewallNotInUse) || (fwloggedin != 0))
					goto okay;

				/* Now logged in to the firewall. */
				fwloggedin++;

				if (cip->firewallType == kFirewallLoginThenUserAtSite) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s@%s", cip->user, cip->host);
				} else if (cip->firewallType == kFirewallUserAtUserPassAtPass) {
					goto okay;
				} else if (cip->firewallType == kFirewallOpenSite) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "OPEN %s", cip->host);
				} else if (cip->firewallType == kFirewallSiteSite) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "SITE %s", cip->host);
				} else if (cip->firewallType == kFirewallFwuAtSiteFwpUserPass) {
					/* only reached when !fwloggedin */
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "USER %s", cip->user);
				} else /* kFirewallUserAtSite[Port[2]] */ {
					goto okay;
				}
				break;

			case 421:	/* 421 Service not available, closing control connection. */
				result = kErrHostDisconnectedDuringLogin;
				goto done;
				
			case 331:	/* 331 User name okay, need password. */
				if ((cip->firewallType == kFirewallNotInUse) || (fwloggedin != 0) || (fwsentpass != 0)) {
					if ((cip->pass[0] == '\0') && (cip->passIsEmpty == 0) && (cip->passphraseProc != kNoFTPGetPassphraseProc))
						(*cip->passphraseProc)(cip, &rp->msg, cip->pass, sizeof(cip->pass));
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "PASS %s", cip->pass);
					sentpass++;
				} else if ((cip->firewallType == kFirewallUserAtSite) || (cip->firewallType == kFirewallUserAtSitePort) || (cip->firewallType == kFirewallUserAtSitePort2)) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "PASS %s", cip->pass);
					sentpass++;
				} else if (cip->firewallType == kFirewallUserAtUserPassAtPass) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "PASS %s@%s", cip->pass, cip->firewallPass);
					fwsentpass++;
				} else if (cip->firewallType == kFirewallUserAtSiteFwuPassFwp) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "PASS %s", cip->pass);
					sentpass++;
				} else if (cip->firewallType == kFirewallFwuAtSiteFwpUserPass) {
					/* only reached when !fwloggedin */
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "PASS %s", cip->firewallPass);
					fwsentpass++;
				} else if (cip->firewallType > kFirewallNotInUse) {
					ReInitResponse(cip, rp);
					result = RCmd(cip, rp, "PASS %s", cip->firewallPass);
					fwsentpass++;
				} else {
					goto unknown;
				}
				break;

			case 332:	/* 332 Need account for login. */
			case 532: 	/* 532 Need account for storing files. */
				ReInitResponse(cip, rp);
				result = RCmd(cip, rp, "ACCT %s", (cip->acct[0] != '\0') ? cip->acct : cip->firewallPass);
				break;

			case 530:	/* Not logged in. */
				result = (sentpass != 0) ? kErrBadRemoteUserOrPassword : kErrBadRemoteUser;
				goto done;

			case 501:	/* Syntax error in parameters or arguments. */
			case 503:	/* Bad sequence of commands. */
			case 550:	/* Can't set guest privileges. */
				goto done;
				
			default:
			unknown:
				if (rp->msg.first == NULL) {
					FTPLogError(cip, kDontPerror, "Lost connection during login.\n");
				} else {
					FTPLogError(cip, kDontPerror, "Unexpected response: %s\n",
						rp->msg.first->line
					);
				}
				goto done;
		}
	}

okay:
	/* Do the application's connect message callback, if present. */
	if (cip->onLoginMsgProc != 0)
		(*cip->onLoginMsgProc)(cip, rp);
	DoneWithResponse(cip, rp);
	result = kNoErr;
	cip->loggedIn = 1;

	/* Make a note of what our root directory is.
	 * This is often different from "/" when not
	 * logged in anonymously.
	 */
	if (cip->startingWorkingDirectory != NULL) {
		free(cip->startingWorkingDirectory);
		cip->startingWorkingDirectory = NULL;
	}
	if ((cip->doNotGetStartingWorkingDirectory == 0) &&
		(FTPGetCWD(cip, cwd, sizeof(cwd)) == kNoErr))
	{
		cip->startingWorkingDirectory = StrDup(cwd);
	}

	/* When a new site is opened, ASCII mode is assumed (by protocol). */
	cip->curTransferType = 'A';
	PrintF(cip, "Logged in to %s as %s.\n", cip->host, cip->user);

	/* Don't leave cleartext password in memory, since we
	 * are logged in and do not need it any more.
	 */
	if ((anonLogin == 0) && (cip->leavePass == 0))
		(void) memset(cip->pass, '*', sizeof(cip->pass) - 1);

	(void) gettimeofday(&cip->loginTime, NULL);
	return result;	/* kNoErr */

done:
	DoneWithResponse(cip, rp);

done2:
	if ((anonLogin == 0) && (cip->leavePass == 0)) {
		switch (result) {
			case kErrConnectRetryableErr:
			case kErrConnectRefused:
			case kErrRemoteHostClosedConnection:
			case kErrHostDisconnectedDuringLogin:
				break;
			default:
				/* Don't leave cleartext password in memory,
				 * since we won't be redialing.
				 */
				(void) memset(cip->pass, '*', sizeof(cip->pass) - 1);
		}
	}
	if (result > 0) {
		result = cip->errNo = kErrLoginHostMiscErr;
	}
	if (result < 0) {
		cip->errNo = result;
	} else {
		(void) gettimeofday(&cip->loginTime, NULL);
	}
	return result;
}	/* FTPLoginHost */




static void
FTPExamineMlstFeatures(const FTPCIPtr cip, const char *features)
{
	char buf[256], *feat;
	int flags;
	char *ctext;

	flags = 0;
	STRNCPY(buf, features);
	ctext = NULL;
	feat = strtokc(buf, ";*", &ctext);
	while (feat != NULL) {
		if (ISTRNEQ(feat, "OS.", 3))
			feat += 3;
		if (ISTREQ(feat, "type")) {
			flags |= kMlsOptType;
		} else if (ISTREQ(feat, "size")) {
			flags |= kMlsOptSize;
		} else if (ISTREQ(feat, "modify")) {
			flags |= kMlsOptModify;
		} else if (ISTREQ(feat, "UNIX.mode")) {
			flags |= kMlsOptUNIXmode;
		} else if (ISTREQ(feat, "UNIX.owner")) {
			flags |= kMlsOptUNIXowner;
		} else if (ISTREQ(feat, "UNIX.group")) {
			flags |= kMlsOptUNIXgroup;
		} else if (ISTREQ(feat, "perm")) {
			flags |= kMlsOptPerm;
		} else if (ISTREQ(feat, "UNIX.uid")) {
			flags |= kMlsOptUNIXuid;
		} else if (ISTREQ(feat, "UNIX.gid")) {
			flags |= kMlsOptUNIXgid;
		} else if (ISTREQ(feat, "UNIX.gid")) {
			flags |= kMlsOptUnique;
		}
		feat = strtokc(NULL, ";*", &ctext);
	}

	cip->mlsFeatures = flags;
}	/* FTPExamineMlstFeatures */




int
FTPQueryFeatures(const FTPCIPtr cip)
{
	ResponsePtr rp;
	int result;
	FTPLinePtr lp;
	char *cp, *p;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if (cip->serverType == kServerTypeNetWareFTP) {
		/* NetWare 5.00 server freaks out when
		 * you give it a command it doesn't
		 * recognize, so cheat here and return.
		 */
		cip->hasPASV = kCommandAvailable;
		cip->hasSIZE = kCommandNotAvailable;
		cip->hasMDTM = kCommandNotAvailable;
		cip->hasMDTM_set = kCommandNotAvailable;
		cip->hasREST = kCommandNotAvailable;
		cip->NLSTfileParamWorks = kCommandAvailable;
		cip->hasCLNT = kCommandNotAvailable;
		cip->hasMLST = kCommandNotAvailable;
		cip->hasMLSD = kCommandNotAvailable;
		cip->hasSITE_UTIME = kCommandNotAvailable;
		cip->hasHELP_SITE = kCommandNotAvailable;
		return (kNoErr);
	}

	if (cip->serverType == kServerTypeProFTPD) {
		/* They won't fix a bug where "NLST -a" outputs as "LIST -a" */
		cip->hasNLST_a = kCommandNotAvailable;
	}
	
	/* Older ftpd implementations have problems. */
	if (	(cip->serverType == kServerTypeDguxFTP) ||
		(cip->serverType == kServerTypePyramid) ||
		(cip->serverType == kServerTypeIBMFTPCS))
	{
		cip->hasCLNT = kCommandNotAvailable;
		cip->hasMLST = kCommandNotAvailable;
		cip->hasMLSD = kCommandNotAvailable;
		cip->hasSITE_UTIME = kCommandNotAvailable;
		cip->hasHELP_SITE = kCommandNotAvailable;
	}
	
	rp = InitResponse();
	if (rp == NULL) {
		cip->errNo = kErrMallocFailed;
		result = cip->errNo;
	} else if (cip->hasFEAT != kCommandNotAvailable) {
		rp->printMode = (kResponseNoPrint|kResponseNoSave);
		result = RCmd(cip, rp, "FEAT");
		if (result < kNoErr) {
			DoneWithResponse(cip, rp);
			return (result);
		} else if (result != 2) {
			/* We cheat here and pre-populate some
			 * fields when the server is wu-ftpd.
			 * This server is very common and we
			 * know it has always had these.
			 */
			 if (cip->serverType == kServerTypeWuFTPd) {
				cip->hasPASV = kCommandAvailable;
				cip->hasSIZE = kCommandAvailable;
				cip->hasMDTM = kCommandAvailable;
				cip->hasMDTM_set = kCommandAvailable;
				cip->hasREST = kCommandAvailable;
				cip->NLSTfileParamWorks = kCommandAvailable;
			} else if (cip->serverType == kServerTypeNcFTPd) {
				cip->hasPASV = kCommandAvailable;
				cip->hasSIZE = kCommandAvailable;
				cip->hasMDTM = kCommandAvailable;
				cip->hasREST = kCommandAvailable;
				cip->NLSTfileParamWorks = kCommandAvailable;
			}

			/* Newer commands are only shown in FEAT,
			 * so we don't have to do the "try it,
			 * then save that it didn't work" thing.
			 */
			cip->hasMLST = kCommandNotAvailable;
			cip->hasMLSD = kCommandNotAvailable;
		} else {
			cip->hasFEAT = kCommandAvailable;

			for (lp = rp->msg.first; lp != NULL; lp = lp->next) {
				/* If first character was not a space it is
				 * either:
				 *
				 * (a) The header line in the response;
				 * (b) The trailer line in the response;
				 * (c) A protocol violation.
				 */
				cp = lp->line;
				if (*cp++ != ' ')
					continue;
				if (ISTRNCMP(cp, "PASV", 4) == 0) {
					cip->hasPASV = kCommandAvailable;
				} else if (ISTRNCMP(cp, "SIZE", 4) == 0) {
					cip->hasSIZE = kCommandAvailable;
				} else if (ISTRNCMP(cp, "MDTM", 4) == 0) {
					cip->hasMDTM = kCommandAvailable;
				} else if (ISTRNCMP(cp, "REST", 4) == 0) {
					cip->hasREST = kCommandAvailable;
				} else if (ISTRNCMP(cp, "UTIME", 5) == 0) {
					cip->hasSITE_UTIME = kCommandAvailable;
				} else if (ISTRNCMP(cp, "MLST", 4) == 0) {
					cip->hasMLST = kCommandAvailable;
					cip->hasMLSD = kCommandAvailable;
					FTPExamineMlstFeatures(cip, cp + 5);
				} else if (ISTRNCMP(cp, "CLNT", 4) == 0) {
					cip->hasCLNT = kCommandAvailable;
				} else if (ISTRNCMP(cp, "Compliance Level: ", 18) == 0) {
					/* Probably only NcFTPd will ever implement this.
					 * But we use it internally to differentiate
					 * between different NcFTPd implementations of
					 * IETF extensions.
					 */
					cip->ietfCompatLevel = atoi(cp + 18);
				}
			}
		}

		/* You can set cip->hasHELP_SITE to kCommandNotAvailable
		 * if your host chokes (i.e. IBM Mainframes) when you
		 * do "HELP SITE".
		 */
		ReInitResponse(cip, rp);
		result = (cip->hasHELP_SITE == kCommandNotAvailable) ? (-1) : RCmd(cip, rp, "HELP SITE");
		if (result == 2) {
			cip->hasHELP_SITE = kCommandAvailable;
			for (lp = rp->msg.first; lp != NULL; lp = lp->next) {
				cp = lp->line;
				if (strstr(cp, "RETRBUFSIZE") != NULL)
					cip->hasSITE_RETRBUFSIZE = kCommandAvailable;
				if (strstr(cp, "RBUFSZ") != NULL)
					cip->hasSITE_RBUFSZ = kCommandAvailable;
				/* See if RBUFSIZ matches (but not STORBUFSIZE) */
				if (
					((p = strstr(cp, "RBUFSIZ")) != NULL) &&
					(
					 	(p == cp) ||
						((p > cp) && (!isupper((int) p[-1])))
					)
				)
					cip->hasSITE_RBUFSIZ = kCommandAvailable;
				if (strstr(cp, "STORBUFSIZE") != NULL)
					cip->hasSITE_STORBUFSIZE = kCommandAvailable;
				if (strstr(cp, "SBUFSIZ") != NULL)
					cip->hasSITE_SBUFSIZ = kCommandAvailable;
				if (strstr(cp, "SBUFSZ") != NULL)
					cip->hasSITE_SBUFSZ = kCommandAvailable;
				if (strstr(cp, "BUFSIZE") != NULL)
					cip->hasSITE_BUFSIZE = kCommandAvailable;
			}
		}
		DoneWithResponse(cip, rp);
	}

	return (kNoErr);
}	/* FTPQueryFeatures */



int
FTPCloseHost(const FTPCIPtr cip)
{
	ResponsePtr rp;
	int result;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	/* Data connection shouldn't be open normally. */
	if (cip->dataSocket != kClosedFileDescriptor)
		FTPAbortDataTransfer(cip);

	result = kNoErr;
	if (cip->connected != 0) {
		rp = InitResponse();
		if (rp == NULL) {
			cip->errNo = kErrMallocFailed;
			result = cip->errNo;
		} else {
			rp->eofOkay = 1;	/* We are expecting EOF after this cmd. */
			cip->eofOkay = 1;
			(void) RCmd(cip, rp, "QUIT");
			DoneWithResponse(cip, rp);
		}
	}
	
	FTPCloseControlConnection(cip);

	/* Dispose dynamic data structures, so you won't leak
	 * if you OpenHost with this again.
	 */
	FTPDeallocateHost(cip);

	if (cip->disconnectTime.tv_sec == 0)
		(void) gettimeofday(&cip->disconnectTime, NULL);
	return (result);
}	/* FTPCloseHost */




void
FTPInitialLogEntry(const FTPCIPtr cip)
{
#if defined(HAVE_UNAME) && defined(HAVE_SYS_UTSNAME_H)
	struct utsname u;
#endif

	if (cip->startTime.tv_sec == 0) {
		(void) gettimeofday(&cip->startTime, NULL);

		/* Log some headers for debugging.
		 *
		 * Note that this is the soonest we can do this,
		 * since the user may not have set the debugLog
		 * fields right away.
		 */
		PrintF(cip, "%s compiled for %s\n",
			gLibNcFTPVersion + 5,
#ifdef O_S
			O_S
#elif (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
			"Windows"
#elif defined(__CYGWIN__)
			"Cygwin"
#else
			"UNIX"
#endif
		);
	
#if defined(HAVE_UNAME) && defined(HAVE_SYS_UTSNAME_H)
		memset(&u, 0, sizeof(u));
		if (uname(&u) == 0) {
			PrintF(cip, "Uname: %s|%s|%s|%s|%s\n",
				u.sysname,
				u.nodename,
				u.release,
				u.version,
				u.machine
			);
		}
#endif

#if defined(SOLARIS) || defined(AIX)
		{
			FILE *fp;
			char line[256], *cp;

#if defined(SOLARIS)
			fp = fopen("/etc/release", "r");
#elif defined(AIX)
			fp = fopen("/usr/lpp/bos/aix_release.level", "r");
#endif
			if (fp != NULL) {
				memset(line, 0, sizeof(line));
				(void) fgets(line, sizeof(line) - 1, fp);
				cp = line + strlen(line) - 1;
				while (cp > line) {
					if (! isspace((int) *cp))
						break;
					--cp;
				}
				cp[1] = '\0';
				cp = line;
				while ((*cp != '\0') && (isspace((int) *cp)))
					cp++;
				fclose(fp);
				PrintF(cip, "Release: %s\n", cp);
			}
		}
#endif


#if defined(HAVE_SYSINFO) && defined(SI_ARCHITECTURE) && defined(SI_ISALIST) && defined(SI_PLATFORM)
		{
			char si_platform[64];
			char si_arch[32];
			char si_isalist[256];

			memset(si_platform, 0, sizeof(si_platform));
			memset(si_arch, 0, sizeof(si_arch));
			memset(si_isalist, 0, sizeof(si_isalist));

			(void) sysinfo(SI_PLATFORM, si_platform, sizeof(si_platform) - 1);
			(void) sysinfo(SI_ARCHITECTURE, si_arch, sizeof(si_arch) - 1);
			(void) sysinfo(SI_ISALIST, si_isalist, sizeof(si_isalist) - 1);
			PrintF(cip, "Sysinfo: %s|%s|%s\n",
				si_platform,
				si_arch,
				si_isalist
			);
		}
#endif

#if defined(HAVE_GNU_GET_LIBC_VERSION) && defined(HAVE_GNU_GET_LIBC_RELEASE)
		PrintF(cip, "Glibc: %s (%s)\n",
			gnu_get_libc_version(),
			gnu_get_libc_release()
		);
#endif	/* GLIBC */

#ifdef MACOSX
/*
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist SYSTEM "file://localhost/System/Library/DTDs/PropertyList.dtd">
<plist version="0.9">
<dict>
	<key>ProductBuildVersion</key>
	<string>5S66</string>
	<key>ProductName</key>
	<string>Mac OS X</string>
	<key>ProductVersion</key>
	<string>10.1.5</string>
</dict>
</plist>
*/
		{
			char osx_ver[32], osx_build[32];
			char line[256], *cp, *cp2; 
			FILE *fp;

			memset(osx_ver, 0, sizeof(osx_ver));
			memset(osx_build, 0, sizeof(osx_build));

			fp = fopen("/System/Library/CoreServices/SystemVersion.plist", "r");

			if (fp != NULL) {
				memset(line, 0, sizeof(line));
				while (fgets(line, sizeof(line) - 1, fp) != NULL) {
					cp = strstr(line, "<key>ProductVersion</key>");
					if (cp != NULL) {
						memset(line, 0, sizeof(line));
						if (fgets(line, sizeof(line) - 2, fp) != NULL) {
							for (cp = line; ((*cp != '\0') && (! isdigit(*cp))); ) cp++;
							for (cp2 = cp; ((*cp2 != '\0') && (! isspace(*cp2)) && (*cp2 != '<') && (*cp2 != '>')); ) cp2++;
							cp2[0] = '\0';
							STRNCPY(osx_ver, cp);
						}
					}

					cp = strstr(line, "<key>ProductBuildVersion</key>");
					if (cp != NULL) {
						memset(line, 0, sizeof(line));
						if (fgets(line, sizeof(line) - 2, fp) != NULL) {
							for (cp = line; ((*cp != '\0') && (! isdigit(*cp))); ) cp++;
							for (cp2 = cp; ((*cp2 != '\0') && (! isspace(*cp2)) && (*cp2 != '<') && (*cp2 != '>')); ) cp2++;
							cp2[0] = '\0';
							STRNCPY(osx_build, cp);
						}
					}
				}
				fclose(fp);
			}
			PrintF(cip, "Sysinfo: Mac OS X %s (Build %s)\n",
				osx_ver,
				osx_build
			);
		}
#endif	/* MACOSX */

	} else {
		/* Starting a new set of opens, but simply update
		 * the timestamp rather than re-log the headers.
		 */
		(void) gettimeofday(&cip->startTime, NULL);
	}
}	/* FTPInitialLogEntry */



int
FTPOpenHost(const FTPCIPtr cip)
{
	int result;
	time_t t0, t1;
	int elapsed;

	if (cip == NULL)
		return (kErrBadParameter);
	if (strcmp(cip->magic, kLibraryMagic))
		return (kErrBadMagic);

	if (cip->host[0] == '\0') {
		cip->errNo = kErrNoHostSpecified;
		return (kErrNoHostSpecified);
	}

	FTPManualOverrideFeatures(cip);
	FTPInitialLogEntry(cip);

	for (	result = kErrConnectMiscErr, cip->numDials = 0;
		cip->maxDials < 0 || cip->numDials < cip->maxDials;
	)	
	{
		/* Allocate (or if the host was closed, reallocate)
		 * the transfer data buffer.
		 */
		result = FTPAllocateHost(cip);
		if (result < 0)
			return (result);

		memset(&cip->connectTime, 0, sizeof(cip->connectTime));
		memset(&cip->loginTime, 0, sizeof(cip->loginTime));
		memset(&cip->disconnectTime, 0, sizeof(cip->disconnectTime));

		cip->totalDials++;
		cip->numDials++;
		if (cip->numDials > 1)
			PrintF(cip, "Retry Number: %d\n", cip->numDials - 1);
		if (cip->redialStatusProc != 0)
			(*cip->redialStatusProc)(cip, kRedialStatusDialing, cip->numDials - 1);
		(void) time(&t0);
		result = OpenControlConnection(cip, cip->host, cip->port);
		(void) time(&t1);
		if (result == kNoErr) {
			/* We were hooked up successfully. */
			(void) gettimeofday(&cip->connectTime, NULL);
			PrintF(cip, "Connected to %s.\n", cip->host);

			result = FTPLoginHost(cip);
			if (result == kNoErr) {
				(void) FTPQueryFeatures(cip);
				/* Do this again in case QueryFeatures
				 * changed anything that the user wants
				 * to use.
				 */
				FTPManualOverrideFeatures(cip);
				break;
			}

			/* Close and try again. */
			(void) FTPCloseHost(cip);

			/* Originally we also stopped retyring if
			 * we got kErrBadRemoteUser and non-anonymous,
			 * but some FTP servers apparently do their
			 * max user check after the username is sent.
			 */
			if (result == kErrBadRemoteUserOrPassword /* || (result == kErrBadRemoteUser) */) {
				if (strcmp(cip->user, "anonymous") != 0) {
					/* Non-anonymous login was denied, and
					 * retrying is not going to help.
					 */
					break;
				}
			}
		} else if ((result != kErrConnectRetryableErr) && (result != kErrConnectRefused) && (result != kErrRemoteHostClosedConnection) && (result != kErrHostDisconnectedDuringLogin)) {
			/* Irrecoverable error, so don't bother redialing.
			 * The error message should have already been printed
			 * from OpenControlConnection().
			 */
			PrintF(cip, "Cannot recover from miscellaneous open error %d.\n", result);
			if (result > 0)
				result = kErrOpenHostMiscErr;
			FTPDeallocateHost(cip);
			return result;
		}

		/* Retryable error, wait and then redial. */
		if (cip->redialDelay > 0) {
			/* But don't sleep if this is the last loop. */
			if ((cip->maxDials < 0) || (cip->numDials < (cip->maxDials))) {
				elapsed = (int) (t1 - t0);
				if (elapsed < cip->redialDelay) {
					PrintF(cip, "Sleeping %u seconds.\n",
						(unsigned) cip->redialDelay - elapsed);
					if (cip->redialStatusProc != 0)
						(*cip->redialStatusProc)(cip, kRedialStatusSleeping, cip->redialDelay - elapsed);
					(void) sleep((unsigned) cip->redialDelay - elapsed);
				}
			}
		}
	}
	if (result > 0)
		result = kErrOpenHostMiscErr;
	if (result != kNoErr)
		FTPDeallocateHost(cip);
	return (result);
}	/* FTPOpenHost */




int
FTPInitConnectionInfo2(const FTPLIPtr lip, const FTPCIPtr cip, char *const buf, size_t bufSize)
{
	size_t siz;

	if ((lip == NULL) || (cip == NULL) || (bufSize == 0))
		return (kErrBadParameter);

	siz = sizeof(FTPConnectionInfo);
	(void) memset(cip, 0, siz);

	if (strcmp(lip->magic, kLibraryMagic))
		return (kErrBadMagic);

	cip->bufSize = bufSize;
	if (buf == NULL) {
		/* We're responsible for allocating the buffer. */
		cip->buf = NULL;	/* denote that it needs to be allocated. */
		cip->doAllocBuf = 1;
	} else {
		/* The application is passing us a buffer to use. */
		cip->buf = buf;
		cip->doAllocBuf = 0;
	}
	cip->port = lip->defaultPort;
	cip->firewallPort = lip->defaultPort;
	cip->maxDials = kDefaultMaxDials;
	cip->redialDelay = kDefaultRedialDelay;
	cip->xferTimeout = kDefaultXferTimeout;
	cip->connTimeout = kDefaultConnTimeout;
	cip->ctrlTimeout = kDefaultCtrlTimeout;
	cip->abortTimeout = kDefaultAbortTimeout;
	cip->ctrlSocketR = kClosedFileDescriptor;
	cip->ctrlSocketW = kClosedFileDescriptor;
	cip->dataPortMode = kDefaultDataPortMode;
	cip->dataSocket = kClosedFileDescriptor;
	cip->lip = lip;
	cip->hasPASV = kCommandAvailabilityUnknown;
	cip->hasSIZE = kCommandAvailabilityUnknown;
	cip->hasMDTM = kCommandAvailabilityUnknown;
	cip->hasMDTM_set = kCommandAvailabilityUnknown;
	cip->hasREST = kCommandAvailabilityUnknown;
	cip->hasNLST_a = kCommandAvailabilityUnknown;
	cip->hasNLST_d = kCommandAvailabilityUnknown;
	cip->hasFEAT = kCommandAvailabilityUnknown;
	cip->hasMLSD = kCommandAvailabilityUnknown;
	cip->hasMLST = kCommandAvailabilityUnknown;
	cip->hasCLNT = kCommandAvailabilityUnknown;
	cip->hasHELP_SITE = kCommandAvailabilityUnknown;
	cip->hasSITE_UTIME = kCommandAvailabilityUnknown;
	cip->hasSITE_RETRBUFSIZE = kCommandAvailabilityUnknown;
	cip->hasSITE_RBUFSIZ = kCommandAvailabilityUnknown;
	cip->hasSITE_RBUFSZ = kCommandAvailabilityUnknown;
	cip->hasSITE_STORBUFSIZE = kCommandAvailabilityUnknown;
	cip->hasSITE_SBUFSIZ = kCommandAvailabilityUnknown;
	cip->hasSITE_SBUFSZ = kCommandAvailabilityUnknown;
	cip->STATfileParamWorks = kCommandAvailabilityUnknown;
	cip->NLSTfileParamWorks = kCommandAvailabilityUnknown;
	cip->firewallType = kFirewallNotInUse;
	cip->startingWorkingDirectory = NULL;
	cip->shutdownUnusedSideOfSockets = 0;
	cip->asciiTranslationMode = kAsciiTranslationModeDefault;
	
#ifdef MACOSX
	/* For Mac OS 9 compatibility you could set this to '\r' */
	cip->textEOLN[0] = '\n';
#elif (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
	cip->textEOLN[0] = '\r';
	cip->textEOLN[1] = '\n';
#else	/* UNIX */
	cip->textEOLN[0] = '\n';
#endif

	InitLineList(&cip->lastFTPCmdResultLL);
	(void) STRNCPY(cip->magic, kLibraryMagic);
	(void) STRNCPY(cip->user, "anonymous");
	(void) gettimeofday(&cip->initTime, NULL);
	return (kNoErr);
}	/* FTPInitConnectionInfo2 */




int
FTPInitConnectionInfo(const FTPLIPtr lip, const FTPCIPtr cip, size_t bufSize)
{
	return (FTPInitConnectionInfo2(lip, cip, NULL, bufSize));
}	/* FTPInitConnectionInfo */




int
FTPInitLibrary(const FTPLIPtr lip)
{
	if (lip == NULL)
		return (kErrBadParameter);

	(void) memset(lip, 0, sizeof(FTPLibraryInfo));

	lip->defaultPort = ServiceNameToPortNumber("ftp", 't');
	if (lip->defaultPort == 0)
		lip->defaultPort = (unsigned int) kDefaultFTPPort;

	lip->init = 1;
	(void) STRNCPY(lip->magic, kLibraryMagic);

	/* We'll initialize the defaultAnonPassword field
	 * later when we try the first anon ftp connection.
	 */

#ifdef HAVE_LIBSOCKS
	SOCKSinit("libncftp");
	lip->socksInit = 1;
#endif
	return (kNoErr);
}	/* FTPInitLibrary */

/* Open.c */
