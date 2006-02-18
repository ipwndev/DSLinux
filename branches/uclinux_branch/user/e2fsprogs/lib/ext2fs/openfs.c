/*
 * openfs.c --- open an ext2 filesystem
 * 
 * Copyright (C) 1993, 1994, 1995, 1996 Theodore Ts'o.
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
#include "e2image.h"

/*
 *  Note: if superblock is non-zero, block-size must also be non-zero.
 * 	Superblock and block_size can be zero to use the default size.
 *
 * Valid flags for ext2fs_open()
 * 
 * 	EXT2_FLAG_RW	- Open the filesystem for read/write.
 * 	EXT2_FLAG_FORCE - Open the filesystem even if some of the
 *				features aren't supported.
 *	EXT2_FLAG_JOURNAL_DEV_OK - Open an ext3 journal device
 */
errcode_t ext2fs_open(const char *name, int flags, int superblock,
		      int block_size, io_manager manager, ext2_filsys *ret_fs)
{
	ext2_filsys	fs;
	errcode_t	retval;
	int		i, j, groups_per_block;
	blk_t		group_block;
	char		*dest;
	struct ext2_group_desc *gdp;
	
	EXT2_CHECK_MAGIC(manager, EXT2_ET_MAGIC_IO_MANAGER);

	retval = ext2fs_get_mem(sizeof(struct struct_ext2_filsys),
				(void **) &fs);
	if (retval)
		return retval;
	
	memset(fs, 0, sizeof(struct struct_ext2_filsys));
	fs->magic = EXT2_ET_MAGIC_EXT2FS_FILSYS;
	fs->flags = flags;
	retval = manager->open(name, (flags & EXT2_FLAG_RW) ? IO_FLAG_RW : 0,
			       &fs->io);
	if (retval)
		goto cleanup;
	fs->io->app_data = fs;
	retval = ext2fs_get_mem(strlen(name)+1, (void **) &fs->device_name);
	if (retval)
		goto cleanup;
	strcpy(fs->device_name, name);
	retval = ext2fs_get_mem(SUPERBLOCK_SIZE, (void **) &fs->super);
	if (retval)
		goto cleanup;
	if (flags & EXT2_FLAG_IMAGE_FILE) {
		retval = ext2fs_get_mem(sizeof(struct ext2_image_hdr),
					(void **) &fs->image_header);
		if (retval)
			goto cleanup;
		retval = io_channel_read_blk(fs->io, 0,
					     -sizeof(struct ext2_image_hdr),
					     fs->image_header);
		if (retval)
			goto cleanup;
		if (fs->image_header->magic_number != EXT2_ET_MAGIC_E2IMAGE)
			return EXT2_ET_MAGIC_E2IMAGE;
		superblock = 1;
		block_size = fs->image_header->fs_blocksize;
	}

	/*
	 * If the user specifies a specific block # for the
	 * superblock, then he/she must also specify the block size!
	 * Otherwise, read the master superblock located at offset
	 * SUPERBLOCK_OFFSET from the start of the partition.
	 *
	 * Note: we only save a backup copy of the superblock if we
	 * are reading the superblock from the primary superblock location.
	 */
	if (superblock) {
		if (!block_size) {
			retval = EXT2_ET_INVALID_ARGUMENT;
			goto cleanup;
		}
		io_channel_set_blksize(fs->io, block_size);
		group_block = superblock + 1;
		fs->orig_super = 0;
	} else {
		io_channel_set_blksize(fs->io, SUPERBLOCK_OFFSET);
		superblock = 1;
		group_block = 0;
		retval = ext2fs_get_mem(SUPERBLOCK_SIZE,
					(void **) &fs->orig_super);
		if (retval)
			goto cleanup;
	}
	retval = io_channel_read_blk(fs->io, superblock, -SUPERBLOCK_SIZE,
				     fs->super);
	if (retval)
		goto cleanup;
	if (fs->orig_super)
		memcpy(fs->orig_super, fs->super, SUPERBLOCK_SIZE);

#ifdef EXT2FS_ENABLE_SWAPFS
	if ((fs->super->s_magic == ext2fs_swab16(EXT2_SUPER_MAGIC)) ||
	    (fs->flags & EXT2_FLAG_SWAP_BYTES)) {
		fs->flags |= EXT2_FLAG_SWAP_BYTES;

		ext2fs_swap_super(fs->super);
	}
#endif
	
	if (fs->super->s_magic != EXT2_SUPER_MAGIC) {
		retval = EXT2_ET_BAD_MAGIC;
		goto cleanup;
	}
	if (fs->super->s_rev_level > EXT2_LIB_CURRENT_REV) {
		retval = EXT2_ET_REV_TOO_HIGH;
		goto cleanup;
	}

	/*
	 * Check for feature set incompatibility
	 */
	if (!(flags & EXT2_FLAG_FORCE)) {
		if (fs->super->s_feature_incompat &
		    ~EXT2_LIB_FEATURE_INCOMPAT_SUPP) {
			retval = EXT2_ET_UNSUPP_FEATURE;
			goto cleanup;
		}
		if ((flags & EXT2_FLAG_RW) &&
		    (fs->super->s_feature_ro_compat &
		     ~EXT2_LIB_FEATURE_RO_COMPAT_SUPP)) {
			retval = EXT2_ET_RO_UNSUPP_FEATURE;
			goto cleanup;
		}
		if (!(flags & EXT2_FLAG_JOURNAL_DEV_OK) &&
		    (fs->super->s_feature_incompat &
		     EXT3_FEATURE_INCOMPAT_JOURNAL_DEV)) {
			retval = EXT2_ET_UNSUPP_FEATURE;
			goto cleanup;
		}
	}
	
	fs->blocksize = EXT2_BLOCK_SIZE(fs->super);
	if (fs->blocksize == 0) {
		retval = EXT2_ET_CORRUPT_SUPERBLOCK;
		goto cleanup;
	}
	fs->fragsize = EXT2_FRAG_SIZE(fs->super);
	fs->inode_blocks_per_group = ((fs->super->s_inodes_per_group *
				       EXT2_INODE_SIZE(fs->super) +
				       EXT2_BLOCK_SIZE(fs->super) - 1) /
				      EXT2_BLOCK_SIZE(fs->super));
	if (block_size) {
		if (block_size != fs->blocksize) {
			retval = EXT2_ET_UNEXPECTED_BLOCK_SIZE;
			goto cleanup;
		}
	}
	/*
	 * Set the blocksize to the filesystem's blocksize.
	 */
	io_channel_set_blksize(fs->io, fs->blocksize);

	/*
	 * If this is an external journal device, don't try to read
	 * the group descriptors, because they're not there.
	 */
	if (fs->super->s_feature_incompat &
	    EXT3_FEATURE_INCOMPAT_JOURNAL_DEV) {
		fs->group_desc_count = 0;
		*ret_fs = fs;
		return 0;
	}
	
	/*
	 * Read group descriptors
	 */
	if ((EXT2_BLOCKS_PER_GROUP(fs->super)) == 0) {
		retval = EXT2_ET_CORRUPT_SUPERBLOCK;
		goto cleanup;
	}
	fs->group_desc_count = (fs->super->s_blocks_count -
				fs->super->s_first_data_block +
				EXT2_BLOCKS_PER_GROUP(fs->super) - 1)
		/ EXT2_BLOCKS_PER_GROUP(fs->super);
	fs->desc_blocks = (fs->group_desc_count +
			   EXT2_DESC_PER_BLOCK(fs->super) - 1)
		/ EXT2_DESC_PER_BLOCK(fs->super);
	retval = ext2fs_get_mem(fs->desc_blocks * fs->blocksize,
				(void **) &fs->group_desc);
	if (retval)
		goto cleanup;
	if (!group_block)
		group_block = fs->super->s_first_data_block + 1;
	dest = (char *) fs->group_desc;
	for (i=0 ; i < fs->desc_blocks; i++) {
		retval = io_channel_read_blk(fs->io, group_block, 1, dest);
		if (retval)
			goto cleanup;
		group_block++;
#ifdef EXT2FS_ENABLE_SWAPFS
		if (fs->flags & EXT2_FLAG_SWAP_BYTES) {
			gdp = (struct ext2_group_desc *) dest;
			groups_per_block = fs->blocksize /
				sizeof(struct ext2_group_desc);
			for (j=0; j < groups_per_block; j++)
				ext2fs_swap_group_desc(gdp++);
		}
#endif
		dest += fs->blocksize;
	}

	*ret_fs = fs;
	return 0;
cleanup:
	ext2fs_free(fs);
	return retval;
}

