/*
 * util.c --- utilities for the debugfs program
 * 
 * Copyright (C) 1993, 1994 Theodore Ts'o.  This file may be
 * redistributed under the terms of the GNU Public License.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "debugfs.h"

FILE *open_pager(void)
{
	FILE *outfile;
	const char *pager = getenv("PAGER");

	signal(SIGPIPE, SIG_IGN);
	if (!pager)
		pager = "more";

	outfile = popen(pager, "w");
	if (!outfile)
		outfile = stdout;

	return (outfile);
}

void close_pager(FILE *stream)
{
	if (stream && stream != stdout) fclose(stream);
}

/*
 * This routine is used whenever a command needs to turn a string into
 * an inode.
 */
ext2_ino_t string_to_inode(char *str)
{
	ext2_ino_t	ino;
	int		len = strlen(str);
	char		*end;
	int		retval;

	/*
	 * If the string is of the form <ino>, then treat it as an
	 * inode number.
	 */
	if ((len > 2) && (str[0] == '<') && (str[len-1] == '>')) {
		ino = strtoul(str+1, &end, 0);
		if (*end=='>')
			return ino;
	}

	retval = ext2fs_namei(current_fs, root, cwd, str, &ino);
	if (retval) {
		com_err(str, retval, "");
		return 0;
	}
	return ino;
}

/*
 * This routine returns 1 if the filesystem is not open, and prints an
 * error message to that effect.
 */
int check_fs_open(char *name)
{
	if (!current_fs) {
		com_err(name, 0, "Filesystem not open");
		return 1;
	}
	return 0;
}

/*
 * This routine returns 1 if a filesystem is open, and prints an
 * error message to that effect.
 */
int check_fs_not_open(char *name)
{
	if (current_fs) {
		com_err(name, 0,
			"Filesystem %s is still open.  Close it first.\n",
			current_fs->device_name);
		return 1;
	}
	return 0;
}

/*
 * This routine returns 1 if a filesystem is not opened read/write,
 * and prints an error message to that effect.
 */
int check_fs_read_write(char *name)
{
	if (!(current_fs->flags & EXT2_FLAG_RW)) {
		com_err(name, 0, "Filesystem opened read/only");
		return 1;
	}
	return 0;
}

/*
 * This routine returns 1 if a filesystem is doesn't have its inode
 * and block bitmaps loaded, and prints an error message to that
 * effect.
 */
int check_fs_bitmaps(char *name)
{
	if (!current_fs->block_map || !current_fs->inode_map) {
		com_err(name, 0, "Filesystem bitmaps not loaded");
		return 1;
	}
	return 0;
}

/*
 * This function takes a __u32 time value and converts it to a string,
 * using ctime
 */
char *time_to_string(__u32 cl)
{
	time_t	t = (time_t) cl;

	return ctime(&t);
}


	

