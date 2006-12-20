/*
 * read_bb_file.c --- read a list of bad blocks from a FILE *
 *
 * Copyright (C) 1994, 1995, 2000 Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

#include <stdio.h>
#include <string.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <fcntl.h>
#include <time.h>
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include "ext2_fs.h"
#include "ext2fs.h"

/*
 * Reads a list of bad blocks from  a FILE *
 */
errcode_t ext2fs_read_bb_FILE2(ext2_filsys fs, FILE *f, 
			       ext2_badblocks_list *bb_list,
			       void *private,
			       void (*invalid)(ext2_filsys fs,
					       blk_t blk,
					       char *badstr,
					       void *private))
{
	errcode_t	retval;
	blk_t		blockno;
	int		count;
	char		buf[128];

	if (fs)
		EXT2_CHECK_MAGIC(fs, EXT2_ET_MAGIC_EXT2FS_FILSYS);

	if (!*bb_list) {
		retval = ext2fs_badblocks_list_create(bb_list, 10);
		if (retval)
			return retval;
	}

	while (!feof (f)) {
		if (fgets(buf, sizeof(buf), f) == NULL)
			break;
		count = sscanf(buf, "%u", &blockno);
		if (count <= 0)
			continue;
		if (fs &&
		    ((blockno < fs->super->s_first_data_block) ||
		    (blockno >= fs->super->s_blocks_count))) {
			if (invalid)
				(invalid)(fs, blockno, buf, private);
			continue;
		}
		retval = ext2fs_badblocks_list_add(*bb_list, blockno);
		if (retval)
			return retval;
	}
	return 0;
}

static void call_compat_invalid(ext2_filsys fs, blk_t blk,
				char *badstr, void *private)
{
	void (*invalid)(ext2_filsys, blk_t);

	invalid = (void (*)(ext2_filsys, blk_t)) private;
	if (invalid)
		invalid(fs, blk);
}


/*
 * Reads a list of bad blocks from  a FILE *
 */
errcode_t ext2fs_read_bb_FILE(ext2_filsys fs, FILE *f, 
			      ext2_badblocks_list *bb_list,
			      void (*invalid)(ext2_filsys fs, blk_t blk))
{
	return ext2fs_read_bb_FILE2(fs, f, bb_list, (void *) invalid,
				    call_compat_invalid);
}


