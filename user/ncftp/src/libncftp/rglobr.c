/* rglobr.c
 *
 * Copyright (c) 1996-2005 Mike Gleason, NcFTP Software.
 * All rights reserved.
 *
 */

#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma hdrstop
#endif

#if 0
static void
printls(LineListPtr list)
{
	LinePtr fip;
	int i;

	for (i = 1, fip = list->first; fip != NULL; fip = fip->next, i++)
		printf("%4d: %s\n", i, fip->line == NULL ? "NULL" : fip->line);
}

static void
print1(FTPFileInfoListPtr list)
{
	FTPFileInfoPtr fip;
	int i;

	for (i = 1, fip = list->first; fip != NULL; fip = fip->next, i++)
		printf("%4d: %s\n", i, fip->relname == NULL ? "NULL" : fip->relname);
}



static void
print2(FTPFileInfoListPtr list)
{
	FTPFileInfoPtr fip;
	int i, n;

	n = list->nFileInfos;
	for (i=0; i<n; i++) {
		fip = list->vec[i];
		printf("%4d: %s\n", i + 1, fip->relname == NULL ? "NULL" : fip->relname);
	}
}




static void
SortRecursiveFileList(FTPFileInfoListPtr files)
{
	VectorizeFileInfoList(files);
	SortFileInfoList(files, 'b', '?');
	UnvectorizeFileInfoList(files);
}	/* SortRecursiveFileList */
#endif




int
FTPRemoteRecursiveFileList1(FTPCIPtr cip, char *const rdir, FTPFileInfoListPtr files)
{
	FTPLineList dirContents;
	FTPFileInfoList fil;
	int result;
	char rcwd[512];

	if ((result = FTPGetCWD(cip, rcwd, sizeof(rcwd))) < 0)
		return (result);

	InitFileInfoList(files);

	if (rdir == NULL)
		return (-1);

	if (FTPChdir(cip, rdir) < 0) {
		/* Probably not a directory.
		 * Just add it as a plain file
		 * to the list.
		 */
		(void) ConcatFileToFileInfoList(files, rdir);
		return (kNoErr);
	}

	/* Paths collected must be relative. */
	if ((result = FTPListToMemory2(cip, "", &dirContents, "-lRa", 1, (int *) 0)) < 0) {
		if ((result = FTPChdir(cip, rcwd)) < 0) {
			rcwd[0] = '\0';
			return (result);
		}
	}

#if 0
DisposeLineListContents(&dirContents);
InitLineList(&dirContents);
AddLine(&dirContents, "drwx------   2 ftpuser  ftpuser       4096 Oct 13 02:12 in");
AddLine(&dirContents, "drwx------   2 ftpuser  ftpuser       4096 Oct 13 02:07 ../out");
AddLine(&dirContents, "drwx------   2 ftpuser  ftpuser       4096 Oct 13 02:11 out2");
AddLine(&dirContents, "drwx------   2 ftpuser  ftpuser       4096 Oct 13 02:11 /usr/tmp/zzzin");
AddLine(&dirContents, "");
AddLine(&dirContents, "./in:");
AddLine(&dirContents, "-rw-r--r--   1 ftpuser  ftpuser      18475 Oct 13 01:58 test_dos.txt");
AddLine(&dirContents, "-rw-r--r--   1 ftpuser  ftpuser      17959 Oct 13 01:57 ../test_mac.txt");
AddLine(&dirContents, "-rw-------   1 ftpuser  ftpuser      17959 Oct 13 01:56 test_unix.txt");
AddLine(&dirContents, "");
AddLine(&dirContents, "./../out:");
AddLine(&dirContents, "-rw-------   1 ftpuser  ftpuser      17969 Oct 13 01:58 test_dos.txt");
AddLine(&dirContents, "-rw-------   1 ftpuser  ftpuser      17959 Oct 13 01:57 test_mac.txt");
AddLine(&dirContents, "-rw-------   1 ftpuser  ftpuser      17959 Oct 13 01:56 test_unix.txt");
AddLine(&dirContents, "");
AddLine(&dirContents, "./out2/../foob:");
/* AddLine(&dirContents, "/tmp/out2:"); */
AddLine(&dirContents, "-rw-------   1 ftpuser  ftpuser      17969 Oct 13 02:08 test_dos.txt");
AddLine(&dirContents, "-rw-------   1 ftpuser  ftpuser      17959 Oct 13 02:08 test_mac.txt");
AddLine(&dirContents, "-rw-------   1 ftpuser  ftpuser      17959 Oct 13 02:08 test_unix.txt");
AddLine(&dirContents, "");
AddLine(&dirContents, "/usr/tmp/zzzin:");
AddLine(&dirContents, "-rw-r--r--   1 ftpuser  ftpuser      18475 Oct 13 01:58 test_dos.txt");
AddLine(&dirContents, "-rw-r--r--   1 ftpuser  ftpuser      17959 Oct 13 01:57 test_mac.txt");
AddLine(&dirContents, "-rw-------   1 ftpuser  ftpuser      17959 Oct 13 01:56 test_unix.txt");
#endif

	/* printls(&dirContents); */
	(void) UnLslR(cip, &fil, &dirContents, cip->serverType);
	DisposeLineListContents(&dirContents);
	/* print1(&fil); */
	/* Could sort it to breadth-first here. */
	/* (void) SortRecursiveFileList(&fil); */
	(void) ComputeRNames(&fil, rdir, 1, 1);
	(void) ConcatFileInfoList(files, &fil);
	DisposeFileInfoListContents(&fil);

	if ((result = FTPChdir(cip, rcwd)) < 0) {
		rcwd[0] = '\0';
		return (result);
	}
	return (kNoErr);
}	/* FTPRemoteRecursiveFileList1 */




int
FTPRemoteRecursiveFileList(FTPCIPtr cip, FTPLineListPtr fileList, FTPFileInfoListPtr files)
{
	FTPLinePtr filePtr, nextFilePtr;
	FTPLineList dirContents;
	FTPFileInfoList fil;
	int result;
	char *rdir;
	char rcwd[512];

	if ((result = FTPGetCWD(cip, rcwd, sizeof(rcwd))) < 0)
		return (result);

	InitFileInfoList(files);

	for (filePtr = fileList->first;
		filePtr != NULL;
		filePtr = nextFilePtr)
	{
		nextFilePtr = filePtr->next;

		rdir = filePtr->line;
		if (rdir == NULL)
			continue;

		if (FTPChdir(cip, rdir) < 0) {
			/* Probably not a directory.
			 * Just add it as a plain file
			 * to the list.
			 */
			(void) ConcatFileToFileInfoList(files, rdir);
			continue;
		}

		/* Paths collected must be relative. */
		if ((result = FTPListToMemory2(cip, "", &dirContents, "-lRa", 1, (int *) 0)) < 0) {
			goto goback;
		}

		(void) UnLslR(cip, &fil, &dirContents, cip->serverType);
		DisposeLineListContents(&dirContents);
		(void) ComputeRNames(&fil, rdir, 1, 1);
		(void) ConcatFileInfoList(files, &fil);
		DisposeFileInfoListContents(&fil);

goback:
		if ((result = FTPChdir(cip, rcwd)) < 0) {
			rcwd[0] = '\0';
			return (result);
		}
	}	
	return (kNoErr);
}	/* FTPRemoteRecursiveFileList */
