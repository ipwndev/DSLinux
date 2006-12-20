/*
 * ext_attr.c --- extended attribute blocks
 * 
 * Copyright (C) Andreas Gruenbacher, <a.gruenbacher@computer.org>
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

#include <stdio.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <time.h>

#include "ext2_fs.h"
#include "ext2_ext_attr.h"

#include "ext2fs.h"

#ifdef EXT2FS_ENABLE_SWAPFS
void ext2fs_swap_ext_attr(ext2_filsys fs, char *to, char *from)
{
	struct ext2_ext_attr_header *from_header =
		(struct ext2_ext_attr_header *)from;
	struct ext2_ext_attr_header *to_header =
		(struct ext2_ext_attr_header *)to;
	struct ext2_ext_attr_entry *from_entry, *to_entry;
	char *from_end = (char *)from_header + fs->blocksize;
	int n;

	if (to_header != from_header)
		memcpy(to_header, from_header, fs->blocksize);

	to_header->h_magic    = ext2fs_swab32(from_header->h_magic);
	to_header->h_blocks   = ext2fs_swab32(from_header->h_blocks);
	to_header->h_refcount = ext2fs_swab32(from_header->h_refcount);
	for (n=0; n<4; n++)
		to_header->h_reserved[n] =
			ext2fs_swab32(from_header->h_reserved[n]);
	
	from_entry = (struct ext2_ext_attr_entry *)(from_header+1);
	to_entry   = (struct ext2_ext_attr_entry *)(to_header+1);
	while ((char *)from_entry < from_end && *(__u32 *)from_entry) {
		to_entry->e_value_offs  =	
			ext2fs_swab16(from_entry->e_value_offs);
		to_entry->e_value_block =	
			ext2fs_swab32(from_entry->e_value_block);
		to_entry->e_value_size  =	
			ext2fs_swab32(from_entry->e_value_size);
		from_entry = EXT2_EXT_ATTR_NEXT(from_entry);
		to_entry   = EXT2_EXT_ATTR_NEXT(to_entry);
	}
}
#endif

errcode_t ext2fs_read_ext_attr(ext2_filsys fs, blk_t block, void *buf)
{
	errcode_t	retval;
	struct ext2_ext_attr_entry *entry;

 	retval = io_channel_read_blk(fs->io, block, 1, buf);
	if (retval)
		return retval;
#ifdef EXT2FS_ENABLE_SWAPFS
	if ((fs->flags & (EXT2_FLAG_SWAP_BYTES|
			  EXT2_FLAG_SWAP_BYTES_READ)) != 0)
		ext2fs_swap_ext_attr(fs, buf, buf);
#endif
	return 0;
}

errcode_t ext2fs_write_ext_attr(ext2_filsys fs, blk_t block, void *inbuf)
{
	errcode_t	retval;
	char		*p, *end, *write_buf;
	char		*buf = NULL;
	struct ext2_dir_entry *dirent;

#ifdef EXT2FS_ENABLE_SWAPFS
	if ((fs->flags & EXT2_FLAG_SWAP_BYTES) ||
	    (fs->flags & EXT2_FLAG_SWAP_BYTES_WRITE)) {
		retval = ext2fs_get_mem(fs->blocksize, (void **) &buf);
		if (retval)
			return retval;
		write_buf = buf;
		ext2fs_swap_ext_attr(fs, buf, inbuf);
	} else
#endif
		write_buf = (char *) inbuf;
 	retval = io_channel_write_blk(fs->io, block, 1, write_buf);
	if (buf)
		ext2fs_free_mem((void **) &buf);
	if (!retval)
		ext2fs_mark_changed(fs);
	return retval;
}
