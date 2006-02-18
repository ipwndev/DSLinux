/*
 * initialize.c --- initialize a filesystem handle given superblock
 * 	parameters.  Used by mke2fs when initializing a filesystem.
 * 
 * Copyright (C) 1994, 1995, 1996 Theodore Ts'o.
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

#if  defined(__linux__)    &&	defined(EXT2_OS_LINUX)
#define CREATOR_OS EXT2_OS_LINUX
#elif defined(__GNU__)     &&	defined(EXT2_OS_HURD)
#define CREATOR_OS EXT2_OS_HURD
#elif defined(__FreeBSD__) &&	defined(EXT2_OS_FREEBSD)
#define CREATOR_OS EXT2_OS_FREEBSD
#elif defined(LITES) 	   &&	defined(EXT2_OS_LITES)
#define CREATOR_OS EXT2_OS_LITES
#else
#define CREATOR_OS EXT2_OS_LINUX /* by default */
#endif

/*
 * Note we override the kernel include file's idea of what the default
 * check interval (never) should be.  It's a good idea to check at
 * least *occasionally*, specially since servers will never rarely get
 * to reboot, since Linux is so robust these days.  :-)
 * 
 * 180 days (six months) seems like a good value.
 */
#ifdef EXT2_DFL_CHECKINTERVAL
#undef EXT2_DFL_CHECKINTERVAL
#endif
#define EXT2_DFL_CHECKINTERVAL (86400L * 180L)

errcode_t ext2fs_initialize(const char *name, int flags,
			    struct ext2_super_block *param,
			    io_manager manager, ext2_filsys *ret_fs)
{
	ext2_filsys	fs;
	errcode_t	retval;
	struct ext2_super_block *super;
	int		frags_per_block;
	int		rem;
	int		overhead = 0;
	blk_t		group_block;
	int		i, j;
	blk_t		numblocks;
	char		*buf;

	if (!param || !param->s_blocks_count)
		return EXT2_ET_INVALID_ARGUMENT;
	
	retval = ext2fs_get_mem(sizeof(struct struct_ext2_filsys),
				(void **) &fs);
	if (retval)
		return retval;
	
	memset(fs, 0, sizeof(struct struct_ext2_filsys));
	fs->magic = EXT2_ET_MAGIC_EXT2FS_FILSYS;
	fs->flags = flags | EXT2_FLAG_RW;
#ifdef WORDS_BIGENDIAN
	fs->flags |= EXT2_FLAG_SWAP_BYTES;
#endif
	retval = manager->open(name, IO_FLAG_RW, &fs->io);
	if (retval)
		goto cleanup;
	fs->io->app_data = fs;
	retval = ext2fs_get_mem(strlen(name)+1, (void **) &fs->device_name);
	if (retval)
		goto cleanup;

	strcpy(fs->device_name, name);
	retval = ext2fs_get_mem(SUPERBLOCK_SIZE, (void **) &super);
	if (retval)
		goto cleanup;
	fs->super = super;

	memset(super, 0, SUPERBLOCK_SIZE);

#define set_field(field, default) (super->field = param->field ? \
				   param->field : (default))

	super->s_magic = EXT2_SUPER_MAGIC;
	super->s_state = EXT2_VALID_FS;

	set_field(s_log_block_size, 0);	/* default blocksize: 1024 bytes */
	set_field(s_log_frag_size, 0); /* default fragsize: 1024 bytes */
	set_field(s_first_data_block, super->s_log_block_size ? 0 : 1);
	set_field(s_max_mnt_count, EXT2_DFL_MAX_MNT_COUNT);
	set_field(s_errors, EXT2_ERRORS_DEFAULT);
	set_field(s_feature_compat, 0);
	set_field(s_feature_incompat, 0);
	set_field(s_feature_ro_compat, 0);
	if (super->s_feature_incompat & ~EXT2_LIB_FEATURE_INCOMPAT_SUPP)
		return EXT2_ET_UNSUPP_FEATURE;
	if (super->s_feature_ro_compat & ~EXT2_LIB_FEATURE_RO_COMPAT_SUPP)
		return EXT2_ET_RO_UNSUPP_FEATURE;

	set_field(s_rev_level, EXT2_GOOD_OLD_REV);
	if (super->s_rev_level >= EXT2_DYNAMIC_REV) {
		set_field(s_first_ino, EXT2_GOOD_OLD_FIRST_INO);
		set_field(s_inode_size, EXT2_GOOD_OLD_INODE_SIZE);
	}

	set_field(s_checkinterval, EXT2_DFL_CHECKINTERVAL);
	super->s_lastcheck = time(NULL);

	super->s_creator_os = CREATOR_OS;

	fs->blocksize = EXT2_BLOCK_SIZE(super);
	fs->fragsize = EXT2_FRAG_SIZE(super);
	frags_per_block = fs->blocksize / fs->fragsize;
	
	/* default: (fs->blocksize*8) blocks/group */
	set_field(s_blocks_per_group, fs->blocksize*8); 
	super->s_frags_per_group = super->s_blocks_per_group * frags_per_block;
	
	super->s_blocks_count = param->s_blocks_count;
	super->s_r_blocks_count = param->s_r_blocks_count;
	if (super->s_r_blocks_count >= param->s_blocks_count) {
		retval = EXT2_ET_INVALID_ARGUMENT;
		goto cleanup;
	}

	/*
	 * If we're creating an external journal device, we don't need
	 * to bother with the rest.
	 */
	if (super->s_feature_incompat &
	    EXT3_FEATURE_INCOMPAT_JOURNAL_DEV) {
		fs->group_desc_count = 0;
		ext2fs_mark_super_dirty(fs);
		*ret_fs = fs;
		return 0;
	}

retry:
	fs->group_desc_count = (super->s_blocks_count -
				super->s_first_data_block +
				EXT2_BLOCKS_PER_GROUP(super) - 1)
		/ EXT2_BLOCKS_PER_GROUP(super);
	if (fs->group_desc_count == 0)
		return EXT2_ET_TOOSMALL;
	fs->desc_blocks = (fs->group_desc_count +
			   EXT2_DESC_PER_BLOCK(super) - 1)
		/ EXT2_DESC_PER_BLOCK(super);

	/* n.b., fs->blocksize is <= 4096 */
	set_field(s_inodes_count, super->s_blocks_count/(4096/fs->blocksize));

	/*
	 * Make sure we have at least EXT2_FIRST_INO + 1 inodes, so
	 * that we have enough inodes for the filesystem(!)
	 */
	if (super->s_inodes_count < EXT2_FIRST_INODE(super)+1)
		super->s_inodes_count = EXT2_FIRST_INODE(super)+1;
	
	/*
	 * There should be at least as many inodes as the user
	 * requested.  Figure out how many inodes per group that
	 * should be.  But make sure that we don't allocate more than
	 * one bitmap's worth of inodes
	 */
	super->s_inodes_per_group = (super->s_inodes_count +
				     fs->group_desc_count - 1) /
					     fs->group_desc_count;
	if (super->s_inodes_per_group > fs->blocksize*8)
		super->s_inodes_per_group = fs->blocksize*8;
	
	/*
	 * Make sure the number of inodes per group completely fills
	 * the inode table blocks in the descriptor.  If not, add some
	 * additional inodes/group.  Waste not, want not...
	 */
	fs->inode_blocks_per_group = (((super->s_inodes_per_group *
					EXT2_INODE_SIZE(super)) +
				       EXT2_BLOCK_SIZE(super) - 1) /
				      EXT2_BLOCK_SIZE(super));
	super->s_inodes_per_group = ((fs->inode_blocks_per_group *
				      EXT2_BLOCK_SIZE(super)) /
				     EXT2_INODE_SIZE(super));
	/*
	 * Finally, make sure the number of inodes per group is a
	 * multiple of 8.  This is needed to simplify the bitmap
	 * splicing code.
	 */
	super->s_inodes_per_group &= ~7;
	fs->inode_blocks_per_group = (((super->s_inodes_per_group *
					EXT2_INODE_SIZE(super)) +
				       EXT2_BLOCK_SIZE(super) - 1) /
				      EXT2_BLOCK_SIZE(super));

	/*
	 * adjust inode count to reflect the adjusted inodes_per_group
	 */
	super->s_inodes_count = super->s_inodes_per_group *
		fs->group_desc_count;
	super->s_free_inodes_count = super->s_inodes_count;

	/*
	 * Overhead is the number of bookkeeping blocks per group.  It
	 * includes the superblock backup, the group descriptor
	 * backups, the inode bitmap, the block bitmap, and the inode
	 * table.
	 *
	 * XXX Not all block groups need the descriptor blocks, but
	 * being clever is tricky...
	 */
	overhead = (int) (3 + fs->desc_blocks + fs->inode_blocks_per_group);
	
	/*
	 * See if the last group is big enough to support the
	 * necessary data structures.  If not, we need to get rid of
	 * it.
	 */
	rem = (int) ((super->s_blocks_count - super->s_first_data_block) %
		     super->s_blocks_per_group);
	if ((fs->group_desc_count == 1) && rem && (rem < overhead))
		return EXT2_ET_TOOSMALL;
	if (rem && (rem < overhead+50)) {
		super->s_blocks_count -= rem;
		goto retry;
	}

	/*
	 * At this point we know how big the filesystem will be.  So
	 * we can do any and all allocations that depend on the block
	 * count.
	 */

	retval = ext2fs_get_mem(strlen(fs->device_name) + 80,
				(void **) &buf);
	if (retval)
		goto cleanup;
	
	sprintf(buf, "block bitmap for %s", fs->device_name);
	retval = ext2fs_allocate_block_bitmap(fs, buf, &fs->block_map);
	if (retval)
		goto cleanup;
	
	sprintf(buf, "inode bitmap for %s", fs->device_name);
	retval = ext2fs_allocate_inode_bitmap(fs, buf, &fs->inode_map);
	if (retval)
		goto cleanup;

	ext2fs_free_mem((void **) &buf);

	retval = ext2fs_get_mem((size_t) fs->desc_blocks * fs->blocksize,
				(void **) &fs->group_desc);
	if (retval)
		goto cleanup;

	memset(fs->group_desc, 0, (size_t) fs->desc_blocks * fs->blocksize);

	/*
	 * Reserve the superblock and group descriptors for each
	 * group, and fill in the correct group statistics for group.
	 * Note that although the block bitmap, inode bitmap, and
	 * inode table have not been allocated (and in fact won't be
	 * by this routine), they are accounted for nevertheless.
	 */
	group_block = super->s_first_data_block;
	super->s_free_blocks_count = 0;
	for (i = 0; i < fs->group_desc_count; i++) {
		if (i == fs->group_desc_count-1) {
			numblocks = (fs->super->s_blocks_count -
				     fs->super->s_first_data_block) %
					     fs->super->s_blocks_per_group;
			if (!numblocks)
				numblocks = fs->super->s_blocks_per_group;
		} else
			numblocks = fs->super->s_blocks_per_group;

		if (ext2fs_bg_has_super(fs, i)) {
			for (j=0; j < fs->desc_blocks+1; j++)
				ext2fs_mark_block_bitmap(fs->block_map,
							 group_block + j);
			numblocks -= 1 + fs->desc_blocks;
		}
		
		numblocks -= 2 + fs->inode_blocks_per_group;
		
		super->s_free_blocks_count += numblocks;
		fs->group_desc[i].bg_free_blocks_count = numblocks;
		fs->group_desc[i].bg_free_inodes_count =
			fs->super->s_inodes_per_group;
		fs->group_desc[i].bg_used_dirs_count = 0;
		
		group_block += super->s_blocks_per_group;
	}
	
	ext2fs_mark_super_dirty(fs);
	ext2fs_mark_bb_dirty(fs);
	ext2fs_mark_ib_dirty(fs);
	
	io_channel_set_blksize(fs->io, fs->blocksize);

	*ret_fs = fs;
	return 0;
cleanup:
	ext2fs_free(fs);
	return retval;
}
	


