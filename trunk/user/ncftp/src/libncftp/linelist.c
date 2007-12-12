/* linelist.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

/* Dynamically make a copy of a string. */
char *
StrDup(const char *buf)
{
	char *cp;
	size_t len;

	if (buf == NULL)
		return (NULL);

	len = strlen(buf) + 1;
	cp = (char *) malloc(len);
	if (cp != NULL)
		(void) memcpy(cp, buf, len);
	return (cp);
}	/* StrDup */



/* Disposes each node of a FTPLineList.  Does a few extra things
 * so the disposed memory won't be very useful after it is freed.
 */
void
DisposeLineListContents(FTPLineListPtr list)
{
	FTPLinePtr lp, lp2;
	
	for (lp = list->first; lp != NULL; ) {
		lp2 = lp;
		lp = lp->next;
		if (lp2->line != NULL) {
			lp2->line[0] = '\0';
			free(lp2->line);
		}
		free(lp2);
	}
	InitLineList(list);
}	/* DisposeLineListContents */




void
InitLineList(FTPLineListPtr list)
{
	list->nLines = 0;
	list->first = list->last = NULL;
}	/* InitLineList */




FTPLinePtr
RemoveLine(FTPLineListPtr list, FTPLinePtr killMe)
{
	FTPLinePtr nextLine, prevLine;
	
	nextLine = killMe->next;	
	prevLine = killMe->prev;	
	if (killMe->line != NULL) {
		killMe->line[0] = '\0';		/* Make it useless just in case. */
		free(killMe->line);
	}

	if (list->first == killMe)
		list->first = nextLine;
	if (list->last == killMe)
		list->last = prevLine;

	if (nextLine != NULL)
		nextLine->prev = prevLine;
	if (prevLine != NULL)
		prevLine->next = nextLine;

	free(killMe);	
	list->nLines--;
	return (nextLine);
}	/* RemoveLine */




/* Adds a string to the FTPLineList specified. */
FTPLinePtr
AddLine(FTPLineListPtr list, const char *buf1)
{
	FTPLinePtr lp;
	char *buf;
	
	lp = (FTPLinePtr) malloc(sizeof(FTPLine));
	if (lp != NULL) {
		buf = StrDup(buf1);
		if (buf == NULL) {
			free(lp);
			lp = NULL;
		} else {
			lp->line = buf;
			lp->next = NULL;
			if (list->first == NULL) {
				list->first = list->last = lp;
				lp->prev = NULL;
				list->nLines = 1;
			} else {
				lp->prev = list->last;
				list->last->next = lp;
				list->last = lp;
				list->nLines++;
			}
		}
	}
	return lp;
}	/* AddLine */




int
CopyLineList(FTPLineListPtr dst, FTPLineListPtr src)
{
	FTPLinePtr lp, lp2;
	
	InitLineList(dst);
	for (lp = src->first; lp != NULL; ) {
		lp2 = lp;
		lp = lp->next;
		if (lp2->line != NULL) {
			if (AddLine(dst, lp2->line) == NULL) {
				DisposeLineListContents(dst);
				return (-1);
			}
		}
	}
	return (0);
}	/* CopyLineList */




/* Disposes each node of a FTPFileInfoList.  Does a few extra things
 * so the disposed memory won't be very useful after it is freed.
 */
void
DisposeFileInfoListContents(FTPFileInfoListPtr list)
{
	FTPFileInfoPtr lp, lp2;
	
	for (lp = list->first; lp != NULL; ) {
		lp2 = lp;
		lp = lp->next;
		if (lp2->relname != NULL) {
			lp2->relname[0] = '\0';
			free(lp2->relname);
		}
		if (lp2->lname != NULL) {
			lp2->lname[0] = '\0';
			free(lp2->lname);
		}
		if (lp2->rname != NULL) {
			lp2->rname[0] = '\0';
			free(lp2->rname);
		}
		if (lp2->rlinkto != NULL) {
			lp2->rlinkto[0] = '\0';
			free(lp2->rlinkto);
		}
		if (lp2->plug != NULL) {
			lp2->plug[0] = '\0';
			free(lp2->plug);
		}
		free(lp2);
	}

	if (list->vec != NULL)
		free(list->vec);

	InitFileInfoList(list);
}	/* DisposeFileInfoListContents */




void
InitFileInfoList(FTPFileInfoListPtr list)
{
	(void) memset(list, 0, sizeof(FTPFileInfoList));

	/* Redundant, but needed to shush BoundsChecker. */
	list->first = list->last = NULL;
	list->vec = (FTPFileInfoVec) 0;
}	/* InitFileInfoList */




static int
TimeCmp(const void *a, const void *b)
{
	const FTPFileInfo *const *fipa;
	const FTPFileInfo *const *fipb;

	fipa = (const FTPFileInfo *const *) a;
	fipb = (const FTPFileInfo *const *) b;
	if ((**fipb).mdtm == (**fipa).mdtm)
		return (0);
	else if ((**fipb).mdtm < (**fipa).mdtm)
		return (-1);
	return (1);
}	/* TimeCmp */




static int
ReverseTimeCmp(const void *a, const void *b)
{
	const FTPFileInfo *const *fipa;
	const FTPFileInfo *const *fipb;

	fipa = (const FTPFileInfo *const *) a;
	fipb = (const FTPFileInfo *const *) b;
	if ((**fipa).mdtm == (**fipb).mdtm)
		return (0);
	else if ((**fipa).mdtm < (**fipb).mdtm)
		return (-1);
	return (1);
}	/* ReverseTimeCmp */




static int
SizeCmp(const void *a, const void *b)
{
	const FTPFileInfo *const *fipa;
	const FTPFileInfo *const *fipb;

	fipa = (const FTPFileInfo *const *) a;
	fipb = (const FTPFileInfo *const *) b;
	if ((**fipb).size == (**fipa).size)
		return (0);
	else if ((**fipb).size < (**fipa).size)
		return (-1);
	return (1);
}	/* SizeCmp */




static int
ReverseSizeCmp(const void *a, const void *b)
{
	const FTPFileInfo *const *fipa;
	const FTPFileInfo *const *fipb;

	fipa = (const FTPFileInfo *const *) a;
	fipb = (const FTPFileInfo *const *) b;
	if ((**fipa).size == (**fipb).size)
		return (0);
	else if ((**fipa).size < (**fipb).size)
		return (-1);
	return (1);
}	/* ReverseSizeCmp */




static int
ReverseNameCmp(const void *a, const void *b)
{
	const FTPFileInfo *const *fipa;
	const FTPFileInfo *const *fipb;

	fipa = (const FTPFileInfo *const *) a;
	fipb = (const FTPFileInfo *const *) b;
#ifdef HAVE_SETLOCALE
	return (strcoll((**fipb).relname, (**fipa).relname));
#else
	return (strcmp((**fipb).relname, (**fipa).relname));
#endif
}	/* ReverseNameCmp */




static int
NameCmp(const void *a, const void *b)
{
	const FTPFileInfo *const *fipa;
	const FTPFileInfo *const *fipb;

	fipa = (const FTPFileInfo *const *) a;
	fipb = (const FTPFileInfo *const *) b;
#ifdef HAVE_SETLOCALE
	return (strcoll((**fipa).relname, (**fipb).relname));
#else
	return (strcmp((**fipa).relname, (**fipb).relname));
#endif
}	/* NameCmp */




static int
BreadthFirstCmp(const void *a, const void *b)
{
	char *cp, *cpa, *cpb;
	int depth, deptha, depthb;
	int c;
	const FTPFileInfo *const *fipa;
	const FTPFileInfo *const *fipb;

	fipa = (const FTPFileInfo *const *) a;
	fipb = (const FTPFileInfo *const *) b;

	cpa = (**fipa).relname;
	cpb = (**fipb).relname;

	for (cp = cpa, depth = 0;;) {
		c = *cp++;
		if (c == '\0')
			break;
		if ((c == '/') || (c == '\\')) {
			depth++;
		}
	}
	deptha = depth;

	for (cp = cpb, depth = 0;;) {
		c = *cp++;
		if (c == '\0')
			break;
		if ((c == '/') || (c == '\\')) {
			depth++;
		}
	}
	depthb = depth;

	if (deptha < depthb)
		return (-1);
	else if (deptha > depthb)
		return (1);

#ifdef HAVE_SETLOCALE
	return (strcoll(cpa, cpb));
#else
	return (strcmp(cpa, cpb));
#endif
}	/* BreadthFirstCmp */




void
SortFileInfoList(FTPFileInfoListPtr list, int sortKey, int sortOrder)
{
	FTPFileInfoVec fiv;
	FTPFileInfoPtr fip;
	int i, j, n, n2;

	fiv = list->vec;
	if (fiv == NULL)
		return;

	if (list->sortKey == sortKey) {
		if (list->sortOrder == sortOrder)
			return;		/* Already sorted they you want. */

		/* Reverse the sort. */
		n = list->nFileInfos;
		if (n > 1) {
			n2 = n / 2;
			for (i=0; i<n2; i++) {
				j = n - i - 1;
				fip = fiv[i];
				fiv[i] = fiv[j];
				fiv[j] = fip;
			}
		}

		list->sortOrder = sortOrder;
	} else if ((sortKey == 'n') && (sortOrder == 'a')) {
		qsort(fiv, (size_t) list->nFileInfos, sizeof(FTPFileInfoPtr),
			NameCmp);
		list->sortKey = sortKey;
		list->sortOrder = sortOrder;
	} else if ((sortKey == 'n') && (sortOrder == 'd')) {
		qsort(fiv, (size_t) list->nFileInfos, sizeof(FTPFileInfoPtr),
			ReverseNameCmp);
		list->sortKey = sortKey;
		list->sortOrder = sortOrder;
	} else if ((sortKey == 't') && (sortOrder == 'a')) {
		qsort(fiv, (size_t) list->nFileInfos, sizeof(FTPFileInfoPtr),
			TimeCmp);
		list->sortKey = sortKey;
		list->sortOrder = sortOrder;
	} else if ((sortKey == 't') && (sortOrder == 'd')) {
		qsort(fiv, (size_t) list->nFileInfos, sizeof(FTPFileInfoPtr),
			ReverseTimeCmp);
		list->sortKey = sortKey;
		list->sortOrder = sortOrder;
	} else if ((sortKey == 's') && (sortOrder == 'a')) {
		qsort(fiv, (size_t) list->nFileInfos, sizeof(FTPFileInfoPtr),
			SizeCmp);
		list->sortKey = sortKey;
		list->sortOrder = sortOrder;
	} else if ((sortKey == 's') && (sortOrder == 'd')) {
		qsort(fiv, (size_t) list->nFileInfos, sizeof(FTPFileInfoPtr),
			ReverseSizeCmp);
		list->sortKey = sortKey;
		list->sortOrder = sortOrder;
	} else if (sortKey == 'b') {
		/* This is different from the rest. */
		list->sortKey = sortKey;
		list->sortOrder = sortOrder;
		qsort(fiv, (size_t) list->nFileInfos, sizeof(FTPFileInfoPtr),
			BreadthFirstCmp);
	}
}	/* SortFileInfoList */




void
VectorizeFileInfoList(FTPFileInfoListPtr list)
{
	FTPFileInfoVec fiv;
	FTPFileInfoPtr fip;
	int i;

	fiv = (FTPFileInfoVec) calloc((size_t) (list->nFileInfos + 1), sizeof(FTPFileInfoPtr));
	if (fiv != (FTPFileInfoVec) 0) {
		for (i = 0, fip = list->first; fip != NULL; fip = fip->next, i++)
			fiv[i] = fip;
		list->vec = fiv;
	}
}	/* VectorizeFileInfoList */




void
UnvectorizeFileInfoList(FTPFileInfoListPtr list)
{
	FTPFileInfoVec fiv;
	FTPFileInfoPtr fip;
	int i, n;

	fiv = list->vec;
	if (fiv != (FTPFileInfoVec) 0) {
		list->first = fiv[0];
		n = list->nFileInfos;
		if (n > 0) {
			list->last = fiv[n - 1];
			fip = fiv[0];
			fip->prev = NULL;
			fip->next = fiv[1];
			for (i = 1; i < n; i++) {
				fip = fiv[i];
				fip->prev = fiv[i - 1];
				fip->next = fiv[i + 1];
			}
		}
		free(fiv);
		list->vec = (FTPFileInfoVec) 0;
	}
}	/* UnvectorizeFileInfoList */




void
InitFileInfo(FTPFileInfoPtr fip)
{
	(void) memset(fip, 0, sizeof(FTPFileInfo));

	fip->type = '-';
	fip->size = kSizeUnknown;
	fip->mdtm = kModTimeUnknown;

	/* Redundant, but needed to shush BoundsChecker. */
	fip->relname = fip->rname = fip->rlinkto = fip->lname = fip->plug = NULL;
	fip->prev = fip->next = NULL;
}	/* InitFileInfo */




FTPFileInfoPtr
RemoveFileInfo(FTPFileInfoListPtr list, FTPFileInfoPtr killMe)
{
	FTPFileInfoPtr nextFileInfo, prevFileInfo;
	
	nextFileInfo = killMe->next;	
	prevFileInfo = killMe->prev;	
	if (killMe->lname != NULL) {
		killMe->lname[0] = '\0';		/* Make it useless just in case. */
		free(killMe->lname);
	}
	if (killMe->relname != NULL) {
		killMe->relname[0] = '\0';
		free(killMe->relname);
	}
	if (killMe->rname != NULL) {
		killMe->rname[0] = '\0';
		free(killMe->rname);
	}
	if (killMe->rlinkto != NULL) {
		killMe->rlinkto[0] = '\0';
		free(killMe->rlinkto);
	}
	if (killMe->plug != NULL) {
		killMe->plug[0] = '\0';
		free(killMe->plug);
	}

	if (list->first == killMe)
		list->first = nextFileInfo;
	if (list->last == killMe)
		list->last = prevFileInfo;

	if (nextFileInfo != NULL)
		nextFileInfo->prev = prevFileInfo;
	if (prevFileInfo != NULL)
		prevFileInfo->next = nextFileInfo;

	free(killMe);	
	list->nFileInfos--;
	return (nextFileInfo);
}	/* RemoveFileInfo */




/* Adds a string to the FTPFileInfoList specified. */
FTPFileInfoPtr
AddFileInfo(FTPFileInfoListPtr list, FTPFileInfoPtr src)
{
	FTPFileInfoPtr lp;
	
	lp = (FTPFileInfoPtr) malloc(sizeof(FTPFileInfo));
	if (lp != NULL) {
		(void) memcpy(lp, src, sizeof(FTPFileInfo));
		lp->next = NULL;
		if (list->first == NULL) {
			list->first = list->last = lp;
			lp->prev = NULL;
			list->nFileInfos = 1;
		} else {
			lp->prev = list->last;
			list->last->next = lp;
			list->last = lp;
			list->nFileInfos++;
		}
	}
	return lp;
}	/* AddFileInfo */




int
ConcatFileInfoList(FTPFileInfoListPtr dst, FTPFileInfoListPtr src)
{
	FTPFileInfoPtr lp, lp2;
	FTPFileInfo newfi;
	
	for (lp = src->first; lp != NULL; lp = lp2) {
		lp2 = lp->next;
		newfi = *lp;
		newfi.relname = StrDup(lp->relname);
		newfi.lname = StrDup(lp->lname);
		newfi.rname = StrDup(lp->rname);
		newfi.rlinkto = StrDup(lp->rlinkto);
		newfi.plug = StrDup(lp->plug);
		if (AddFileInfo(dst, &newfi) == NULL)
			return (-1);
	}
	return (0);
}	/* ConcatFileInfoList */




int
ComputeRNames(FTPFileInfoListPtr dst, const char *dstdir, int pflag, int nochop)
{
	FTPFileInfoPtr lp, lp2;
	char *buf;
	char *cp;

	if (dstdir == NULL)
		dstdir = ".";

	for (lp = dst->first; lp != NULL; lp = lp2) {
		lp2 = lp->next;

		buf = NULL;
		if (nochop != 0) {
			if ((dstdir[0] != '\0') && (strcmp(dstdir, "."))) {
				if (Dynscat(&buf, dstdir, "/", lp->relname, 0) == NULL)
					goto memerr;

				if (pflag != 0) {
					/* Init lname to parent dir name of remote dir */
					cp = strrchr(dstdir, '/');
					if (cp == NULL)
						cp = strrchr(dstdir, '\\');
					if (cp != NULL) {
						if (Dynscat(&lp->lname, cp + 1, 0) == NULL)
							goto memerr;
						TVFSPathToLocalPath(lp->lname);
					}
				}
			} else {
				if (Dynscat(&buf, lp->relname, 0) == NULL)
					goto memerr;
			}
		} else {
			if ((dstdir[0] != '\0') && (strcmp(dstdir, "."))) {
				cp = strrchr(lp->relname, '/');
				if (cp == NULL)
					cp = strrchr(lp->relname, '\\');
				if (cp != NULL) {
					cp++;
				} else {
					cp = lp->relname;
				}
				if (Dynscat(&buf, dstdir, "/", cp, 0) == NULL)
					goto memerr;

				if (pflag != 0) {
					/* Init lname to parent dir name of remote dir */
					cp = strrchr(dstdir, '/');
					if (cp == NULL)
						cp = strrchr(dstdir, '\\');
					if (cp != NULL) {
						if (Dynscat(&lp->lname, cp + 1, 0) == NULL)
							goto memerr;
						TVFSPathToLocalPath(lp->lname);
					}
				}
			} else {
				cp = strrchr(lp->relname, '/');
				if (cp == NULL)
					cp = strrchr(lp->relname, '\\');
				if (cp != NULL) {
					cp++;
				} else {
					cp = lp->relname;
				}
				if (Dynscat(&buf, cp, 0) == NULL)
					goto memerr;
			}
		}
		lp->rname = buf;
		if (lp->rname == NULL) {
memerr:
			return (-1);
		}
		LocalPathToTVFSPath(lp->rname);
	}
	return (0);
}	/* ComputeRNames */




int
ComputeLNames(FTPFileInfoListPtr dst, const char *srcdir, const char *dstdir, int nochop)
{
	FTPFileInfoPtr lp, lp2;
	char *buf;
	char *cp;

	if (srcdir != NULL) {
		cp = strrchr(srcdir, '/');
		if (cp == NULL)
			cp = strrchr(srcdir, '\\');
		if (cp != NULL)
			srcdir = cp + 1;
	}
	if (dstdir == NULL)
		dstdir = ".";

	for (lp = dst->first; lp != NULL; lp = lp2) {
		lp2 = lp->next;

		buf = NULL;
		if (nochop != 0) {
			if ((dstdir[0] != '\0') && (strcmp(dstdir, "."))) {
				if (Dynscat(&buf, dstdir, "/", 0) == NULL)
					goto memerr;
			}
			if (lp->lname != NULL) {
				if (Dynscat(&buf, lp->lname, "/", 0) == NULL)
					goto memerr;
			} else if (srcdir != NULL) {
				if (Dynscat(&buf, srcdir, "/", 0) == NULL)
					goto memerr;
			}
			if (Dynscat(&buf, lp->relname, 0) == NULL)
				goto memerr;
		} else {
			if ((dstdir[0] != '\0') && (strcmp(dstdir, "."))) {
				cp = strrchr(lp->relname, '/');
				if (cp == NULL)
					cp = strrchr(lp->relname, '\\');
				if (cp == NULL) {
					cp = lp->relname;
				} else {
					cp++;
				}
				if (Dynscat(&buf, dstdir, "/", cp, 0) == NULL)
					goto memerr;
			} else {
				cp = strrchr(lp->relname, '/');
				if (cp == NULL)
					cp = strrchr(lp->relname, '\\');
				if (cp == NULL) {
					cp = lp->relname;
				} else {
					cp++;
				}
				if (Dynscat(&buf, cp, 0) == NULL)
					goto memerr;
			}
		}
		if (buf == NULL) {
memerr:
			return (-1);
		}
		if (lp->lname != NULL) {
			free(lp->lname);
			lp->lname = NULL;
		}
		lp->lname = buf;
		TVFSPathToLocalPath(lp->lname);
	}
	return (0);
}	/* ComputeLNames */




int
ConcatFileToFileInfoList(FTPFileInfoListPtr dst, char *rfile)
{
	FTPFileInfo newfi;

	InitFileInfo(&newfi);	/* Use defaults. */
	newfi.relname = StrDup(rfile);
	newfi.rname = NULL;
	newfi.lname = NULL;

	if (AddFileInfo(dst, &newfi) == NULL)
		return (-1);
	return (0);
}	/* ConcatFileToFileInfoList */




int
LineListToFileInfoList(FTPLineListPtr src, FTPFileInfoListPtr dst)
{
	FTPLinePtr lp, lp2;

	InitFileInfoList(dst);
	for (lp = src->first; lp != NULL; lp = lp2) {
		lp2 = lp->next;
		if (ConcatFileToFileInfoList(dst, lp->line) < 0)
			return (-1);
	}
	return (0);
}	/* LineListToFileList */




int
LineToFileInfoList(FTPLinePtr lp, FTPFileInfoListPtr dst)
{
	InitFileInfoList(dst);
	if (ConcatFileToFileInfoList(dst, lp->line) < 0)
		return (-1);
	return (0);
}	/* LineToFileInfoList */

/* eof */
