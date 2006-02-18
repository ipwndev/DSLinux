/*
 * resize2fs.c --- ext2 main routine
 *
 * Copyright (C) 1997, 1998 by Theodore Ts'o and
 * 	PowerQuest, Inc.
 *
 * Copyright (C) 1999, 2000 by Theosore Ts'o
 * 
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

/*
 * Resizing a filesystem consists of the following phases:
 *
 *	1.  Adjust superblock and write out new parts of the inode
 * 		table
 * 	2.  Determine blocks which need to be relocated, and copy the
 * 		contents of blocks from their old locations to the new ones.
 * 	3.  Scan the inode table, doing the following:
 * 		a.  If blocks have been moved, update the block
 * 			pointers in the inodes and indirect blocks to
 * 			point at the new block locations.
 * 		b.  If parts of the inode table need to be evacuated,
 * 			copy inodes from their old locations to their
 * 			new ones.
 * 		c.  If (b) needs to be done, note which blocks contain
 * 			directory information, since we will need to
 * 			update the directory information.
 * 	4.  Update the directory blocks with the new inode locations.
 * 	5.  Move the inode tables, if necessary.
 */

#include "resize2fs.h"

#ifdef linux			/* Kludge for debugging */
#define RESIZE2FS_DEBUG
#endif

static errcode_t adjust_superblock(ext2_resize_t rfs, blk_t new_size);
static errcode_t blocks_to_move(ext2_resize_t rfs);
static errcode_t block_mover(ext2_resize_t rfs);
static errcode_t inode_scan_and_fix(ext2_resize_t rfs);
static errcode_t inode_ref_fix(ext2_resize_t rfs);
static errcode_t move_itables(ext2_resize_t rfs);
static errcode_t ext2fs_calculate_summary_stats(ext2_filsys fs);

/*
 * Some helper CPP macros
 */
#define FS_BLOCK_BM(fs, i) ((fs)->group_desc[(i)].bg_block_bitmap)
#define FS_INODE_BM(fs, i) ((fs)->group_desc[(i)].bg_inode_bitmap)
#define FS_INODE_TB(fs, i) ((fs)->group_desc[(i)].bg_inode_table)

#define IS_BLOCK_BM(fs, i, blk) ((blk) == FS_BLOCK_BM((fs),(i)))
#define IS_INODE_BM(fs, i, blk) ((blk) == FS_INODE_BM((fs),(i)))

#define IS_INODE_TB(fs, i, blk) (((blk) >= FS_INODE_TB((fs), (i))) && \
				 ((blk) < (FS_INODE_TB((fs), (i)) + \
					   (fs)->inode_blocks_per_group)))



/*
 * This is the top-level routine which does the dirty deed....
 */
errcode_t resize_fs(ext2_filsys fs, blk_t new_size, int flags,
		    errcode_t (*progress)(ext2_resize_t rfs, int pass,
				     unsigned long cur,
				     unsigned long max_val))
{
	ext2_resize_t	rfs;
	errcode_t	retval;

	retval = ext2fs_read_bitmaps(fs);
	if (retval)
		return retval;
	
	/*
	 * Create the data structure
	 */
	retval = ext2fs_get_mem(sizeof(struct ext2_resize_struct),
				(void **) &rfs);
	if (retval)
		return retval;
	memset(rfs, 0, sizeof(struct ext2_resize_struct));

	rfs->old_fs = fs;
	rfs->flags = flags;
	rfs->itable_buf	 = 0;
	rfs->progress = progress;
	retval = ext2fs_dup_handle(fs, &rfs->new_fs);
	if (retval)
		goto errout;

	retval = adjust_superblock(rfs, new_size);
	if (retval)
		goto errout;

	retval = blocks_to_move(rfs);
	if (retval)
		goto errout;

#ifdef RESIZE2FS_DEBUG
	if (rfs->flags & RESIZE_DEBUG_BMOVE)
		printf(_("Number of free blocks: %d/%d, Needed: %d\n"),
		       rfs->old_fs->super->s_free_blocks_count,
		       rfs->new_fs->super->s_free_blocks_count,
		       rfs->needed_blocks);
#endif
	
	retval = block_mover(rfs);
	if (retval)
		goto errout;

	retval = inode_scan_and_fix(rfs);
	if (retval)
		goto errout;

	retval = inode_ref_fix(rfs);
	if (retval)
		goto errout;

	retval = ext2fs_calculate_summary_stats(rfs->new_fs);
	if (retval)
		goto errout;
	
	retval = move_itables(rfs);
	if (retval)
		goto errout;

	retval = ext2fs_close(rfs->new_fs);
	if (retval)
		goto errout;

	rfs->flags = flags;
	
	ext2fs_free(rfs->old_fs);
	if (rfs->itable_buf)
		ext2fs_free_mem((void **) &rfs->itable_buf);
	ext2fs_free_mem((void **) &rfs);
	
	return 0;

errout:
	if (rfs->new_fs)
		ext2fs_free(rfs->new_fs);
	if (rfs->itable_buf)
		ext2fs_free_mem((void **) &rfs->itable_buf);
	ext2fs_free_mem((void **) &rfs);
	return retval;
}

/* --------------------------------------------------------------------
 *
 * Resize processing, phase 1.
 *
 * In this phase we adjust the in-memory superblock information, and
 * initialize any new parts of the inode table.  The new parts of the
 * inode table are created in virgin disk space, so we can abort here
 * without any side effects.
 * --------------------------------------------------------------------
 */

/*
 * This routine adjusts the superblock and other data structures...
 */
static errcode_t adjust_superblock(ext2_resize_t rfs, blk_t new_size)
{
	ext2_filsys fs;
	int		overhead = 0;
	int		rem, adj = 0;
	errcode_t	retval;
	ext2_ino_t	real_end;
	blk_t		blk, group_block;
	unsigned long	i, j;
	int		old_numblocks, numblocks, adjblocks;
	unsigned long	max_group;
	
	fs = rfs->new_fs;
	fs->super->s_blocks_count = new_size;
	ext2fs_mark_super_dirty(fs);
	ext2fs_mark_bb_dirty(fs);
	ext2fs_mark_ib_dirty(fs);

retry:
	fs->group_desc_count = (fs->super->s_blocks_count -
				fs->super->s_first_data_block +
				EXT2_BLOCKS_PER_GROUP(fs->super) - 1)
		/ EXT2_BLOCKS_PER_GROUP(fs->super);
	if (fs->group_desc_count == 0)
		return EXT2_ET_TOOSMALL;
	fs->desc_blocks = (fs->group_desc_count +
			   EXT2_DESC_PER_BLOCK(fs->super) - 1)
		/ EXT2_DESC_PER_BLOCK(fs->super);

	/*
	 * Overhead is the number of bookkeeping blocks per group.  It
	 * includes the superblock backup, the group descriptor
	 * backups, the inode bitmap, the block bitmap, and the inode
	 * table.
	 *
	 * XXX Not all block groups need the descriptor blocks, but
	 * being clever is tricky...
	 */
	overhead = 3 + fs->desc_blocks + fs->inode_blocks_per_group;
	
	/*
	 * See if the last group is big enough to support the
	 * necessary data structures.  If not, we need to get rid of
	 * it.
	 */
	rem = (fs->super->s_blocks_count - fs->super->s_first_data_block) %
		fs->super->s_blocks_per_group;
	if ((fs->group_desc_count == 1) && rem && (rem < overhead))
		return EXT2_ET_TOOSMALL;
	if (rem && (rem < overhead+50)) {
		fs->super->s_blocks_count -= rem;
		goto retry;
	}
	/*
	 * Adjust the number of inodes
	 */
	fs->super->s_inodes_count = fs->super->s_inodes_per_group *
		fs->group_desc_count;

	/*
	 * Adjust the number of free blocks
	 */
	blk = rfs->old_fs->super->s_blocks_count;
	if (blk > fs->super->s_blocks_count)
		fs->super->s_free_blocks_count -=
			(blk - fs->super->s_blocks_count);
	else
		fs->super->s_free_blocks_count +=
			(fs->super->s_blocks_count - blk);

	/*
	 * Adjust the number of reserved blocks
	 */
	blk = rfs->old_fs->super->s_r_blocks_count * 100 /
		rfs->old_fs->super->s_blocks_count;
	fs->super->s_r_blocks_count = ((fs->super->s_blocks_count * blk)
				       / 100);

	/*
	 * Adjust the bitmaps for size
	 */
	retval = ext2fs_resize_inode_bitmap(fs->super->s_inodes_count,
					    fs->super->s_inodes_count,
					    fs->inode_map);
	if (retval) goto errout;
	
	real_end = ((EXT2_BLOCKS_PER_GROUP(fs->super)
		     * fs->group_desc_count)) - 1 +
			     fs->super->s_first_data_block;
	retval = ext2fs_resize_block_bitmap(fs->super->s_blocks_count-1,
					    real_end, fs->block_map);

	if (retval) goto errout;

	/*
	 * Reallocate the group descriptors as necessary.
	 */
	if (rfs->old_fs->desc_blocks != fs->desc_blocks) {
		retval = ext2fs_resize_mem(rfs->old_fs->desc_blocks *
					   fs->blocksize,
					   fs->desc_blocks * fs->blocksize,
					   (void **) &fs->group_desc);
		if (retval)
			goto errout;
	}

	/*
	 * Check to make sure there are enough inodes
	 */
	if ((rfs->old_fs->super->s_inodes_count -
	     rfs->old_fs->super->s_free_inodes_count) >
	    rfs->new_fs->super->s_inodes_count) {
		retval = ENOSPC;
		goto errout;
	}

	/*
	 * If we are shrinking the number block groups, we're done and
	 * can exit now.
	 */
	if (rfs->old_fs->group_desc_count > fs->group_desc_count) {
		retval = 0;
		goto errout;
	}
	/*
	 * Fix the count of the last (old) block group
	 */
	old_numblocks = (rfs->old_fs->super->s_blocks_count -
			 rfs->old_fs->super->s_first_data_block) %
				 rfs->old_fs->super->s_blocks_per_group;
	if (!old_numblocks)
		old_numblocks = rfs->old_fs->super->s_blocks_per_group;
	if (rfs->old_fs->group_desc_count == fs->group_desc_count) {
		numblocks = (rfs->new_fs->super->s_blocks_count -
			     rfs->new_fs->super->s_first_data_block) %
				     rfs->new_fs->super->s_blocks_per_group;
		if (!numblocks)
			numblocks = rfs->new_fs->super->s_blocks_per_group;
	} else
		numblocks = rfs->new_fs->super->s_blocks_per_group;
	i = rfs->old_fs->group_desc_count - 1;
	fs->group_desc[i].bg_free_blocks_count += (numblocks-old_numblocks);
		
	/*
	 * If the number of block groups is staying the same, we're
	 * done and can exit now.  (If the number block groups is
	 * shrinking, we had exited earlier.)
	 */
	if (rfs->old_fs->group_desc_count >= fs->group_desc_count) {
		retval = 0;
		goto errout;
	}
	/*
	 * Initialize the new block group descriptors
	 */
	retval = ext2fs_get_mem(fs->blocksize * fs->inode_blocks_per_group,
				(void **) &rfs->itable_buf);
	if (retval)
		goto errout;

	memset(rfs->itable_buf, 0, fs->blocksize * fs->inode_blocks_per_group);
	group_block = fs->super->s_first_data_block +
		rfs->old_fs->group_desc_count * fs->super->s_blocks_per_group;

	adj = rfs->old_fs->group_desc_count;
	max_group = fs->group_desc_count - adj;
	if (rfs->progress) {
		retval = rfs->progress(rfs, E2_RSZ_EXTEND_ITABLE_PASS,
				       0, max_group);
		if (retval)
			goto errout;
	}
	for (i = rfs->old_fs->group_desc_count;
	     i < fs->group_desc_count; i++) {
		memset(&fs->group_desc[i], 0,
		       sizeof(struct ext2_group_desc));
		adjblocks = 0;

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
			adjblocks = 1 + fs->desc_blocks;
		}
		adjblocks += 2 + fs->inode_blocks_per_group;
		
		numblocks -= adjblocks;
		fs->super->s_free_blocks_count -= adjblocks;
		fs->super->s_free_inodes_count +=
			fs->super->s_inodes_per_group;
		fs->group_desc[i].bg_free_blocks_count = numblocks;
		fs->group_desc[i].bg_free_inodes_count =
			fs->super->s_inodes_per_group;
		fs->group_desc[i].bg_used_dirs_count = 0;

		retval = ext2fs_allocate_group_table(fs, i, 0);
		if (retval) goto errout;

		/*
		 * Write out the new inode table
		 */
		retval = io_channel_write_blk(fs->io,
					      fs->group_desc[i].bg_inode_table,
					      fs->inode_blocks_per_group,
					      rfs->itable_buf);
		if (retval) goto errout;

		io_channel_flush(fs->io);
		if (rfs->progress) {
			retval = rfs->progress(rfs, E2_RSZ_EXTEND_ITABLE_PASS,
					       i - adj + 1, max_group);
			if (retval)
				goto errout;
		}
		group_block += fs->super->s_blocks_per_group;
	}
	io_channel_flush(fs->io);
	retval = 0;

errout:
	return retval;
}

/* --------------------------------------------------------------------
 *
 * Resize processing, phase 2.
 *
 * In this phase we adjust determine which blocks need to be moved, in
 * blocks_to_move().  We then copy the blocks to their ultimate new
 * destinations using block_mover().  Since we are copying blocks to
 * their new locations, again during this pass we can abort without
 * any problems.
 * --------------------------------------------------------------------
 */

/*
 * This helper function creates a block bitmap with all of the
 * filesystem meta-data blocks.
 */
static errcode_t mark_table_blocks(ext2_filsys fs,
				   ext2fs_block_bitmap *ret_bmap)
{
	blk_t			block, b;
	int			i,j;
	ext2fs_block_bitmap	bmap;
	errcode_t		retval;

	retval = ext2fs_allocate_block_bitmap(fs, _("meta-data blocks"), 
					      &bmap);
	if (retval)
		return retval;
	
	block = fs->super->s_first_data_block;
	for (i = 0; i < fs->group_desc_count; i++) {
		if (ext2fs_bg_has_super(fs, i)) {
			/*
			 * Mark this group's copy of the superblock
			 */
			ext2fs_mark_block_bitmap(bmap, block);
		
			/*
			 * Mark this group's copy of the descriptors
			 */
			for (j = 0; j < fs->desc_blocks; j++)
				ext2fs_mark_block_bitmap(bmap, block + j + 1);
		}
		
		/*
		 * Mark the blocks used for the inode table
		 */
		for (j = 0, b = fs->group_desc[i].bg_inode_table;
		     j < fs->inode_blocks_per_group;
		     j++, b++)
			ext2fs_mark_block_bitmap(bmap, b);
			    
		/*
		 * Mark block used for the block bitmap 
		 */
		ext2fs_mark_block_bitmap(bmap,
					 fs->group_desc[i].bg_block_bitmap);
		/*
		 * Mark block used for the inode bitmap 
		 */
		ext2fs_mark_block_bitmap(bmap,
					 fs->group_desc[i].bg_inode_bitmap);
		block += fs->super->s_blocks_per_group;
	}
	*ret_bmap = bmap;
	return 0;
}

/*
 * This routine marks and unmarks reserved blocks in the new block
 * bitmap.  It also determines which blocks need to be moved and
 * places this information into the move_blocks bitmap.
 */
static errcode_t blocks_to_move(ext2_resize_t rfs)
{
	int	i, j, max_groups;
	blk_t	blk, group_blk;
	unsigned long old_blocks, new_blocks;
	errcode_t	retval;
	ext2_filsys 	fs, old_fs;
	ext2fs_block_bitmap	meta_bmap;

	fs = rfs->new_fs;
	old_fs = rfs->old_fs;
	if (old_fs->super->s_blocks_count > fs->super->s_blocks_count)
		fs = rfs->old_fs;
	
	retval = ext2fs_allocate_block_bitmap(fs, _("reserved blocks"),
					      &rfs->reserve_blocks);
	if (retval)
		return retval;

	retval = ext2fs_allocate_block_bitmap(fs, _("blocks to be moved"),
					      &rfs->move_blocks);
	if (retval)
		return retval;

	retval = mark_table_blocks(old_fs, &meta_bmap);
	if (retval)
		return retval;

	fs = rfs->new_fs;
	
	/*
	 * If we're shrinking the filesystem, we need to move all of
	 * the blocks that don't fit any more
	 */
	for (blk = fs->super->s_blocks_count;
	     blk < old_fs->super->s_blocks_count; blk++) {
		if (ext2fs_test_block_bitmap(old_fs->block_map, blk) &&
		    !ext2fs_test_block_bitmap(meta_bmap, blk)) {
			ext2fs_mark_block_bitmap(rfs->move_blocks, blk);
			rfs->needed_blocks++;
		}
		ext2fs_mark_block_bitmap(rfs->reserve_blocks, blk);
	}
	
	old_blocks = old_fs->desc_blocks;
	new_blocks = fs->desc_blocks;

	if (old_blocks == new_blocks) {
		retval = 0;
		goto errout;
	}

	max_groups = fs->group_desc_count;
	if (max_groups > old_fs->group_desc_count)
		max_groups = old_fs->group_desc_count;
	group_blk = old_fs->super->s_first_data_block;
	/*
	 * If we're reducing the number of descriptor blocks, this
	 * makes life easy.  :-)   We just have to mark some extra
	 * blocks as free.
	 */
	if (old_blocks > new_blocks) {
		for (i = 0; i < max_groups; i++) {
			if (!ext2fs_bg_has_super(fs, i)) {
				group_blk += fs->super->s_blocks_per_group;
				continue;
			}
			for (blk = group_blk+1+new_blocks;
			     blk < group_blk+1+old_blocks; blk++) {
				ext2fs_unmark_block_bitmap(fs->block_map,
							   blk);
				rfs->needed_blocks--;
			}
			group_blk += fs->super->s_blocks_per_group;
		}
		retval = 0;
		goto errout;
	}
	/*
	 * If we're increasing the number of descriptor blocks, life
	 * gets interesting....  
	 */
	for (i = 0; i < max_groups; i++) {
		if (!ext2fs_bg_has_super(fs, i))
			goto next_group;

		for (blk = group_blk;
		     blk < group_blk + 1 + new_blocks; blk++) {
			ext2fs_mark_block_bitmap(rfs->reserve_blocks, blk);
			ext2fs_mark_block_bitmap(fs->block_map, blk);

			/*
			 * Check to see if we overlap with the inode
			 * or block bitmap, or the inode tables.  If
			 * not, and the block is in use, then mark it
			 * as a block to be moved.
			 */
			if (IS_BLOCK_BM(fs, i, blk)) {
				FS_BLOCK_BM(fs, i) = 0;
				rfs->needed_blocks++;
			} else if (IS_INODE_BM(fs, i, blk)) {
				FS_INODE_BM(fs, i) = 0;
				rfs->needed_blocks++;
			} else if (IS_INODE_TB(fs, i, blk)) {
				FS_INODE_TB(fs, i) = 0;
				rfs->needed_blocks++;
			} else if (ext2fs_test_block_bitmap(old_fs->block_map,
							    blk) &&
				   !ext2fs_test_block_bitmap(meta_bmap, blk)) {
				ext2fs_mark_block_bitmap(rfs->move_blocks,
							 blk);
				rfs->needed_blocks++;
			}
		}
		if (fs->group_desc[i].bg_inode_table &&
		    fs->group_desc[i].bg_inode_bitmap &&
		    fs->group_desc[i].bg_block_bitmap)
			goto next_group;

		/*
		 * Reserve the existing meta blocks that we know
		 * aren't to be moved.
		 */
		if (fs->group_desc[i].bg_block_bitmap)
			ext2fs_mark_block_bitmap(rfs->reserve_blocks,
				 fs->group_desc[i].bg_block_bitmap);
		if (fs->group_desc[i].bg_inode_bitmap)
			ext2fs_mark_block_bitmap(rfs->reserve_blocks,
				 fs->group_desc[i].bg_inode_bitmap);
		if (fs->group_desc[i].bg_inode_table)
			for (blk = fs->group_desc[i].bg_inode_table, j=0;
			     j < fs->inode_blocks_per_group ; j++, blk++)
				ext2fs_mark_block_bitmap(rfs->reserve_blocks,
							 blk);

		/*
		 * Allocate the missing data structures
		 */
		retval = ext2fs_allocate_group_table(fs, i,
						     rfs->reserve_blocks);
		if (retval)
			goto errout;

		/*
		 * For those structures that have changed, we need to
		 * do bookkeepping.
		 */
		if (FS_BLOCK_BM(old_fs, i) !=
		    (blk = FS_BLOCK_BM(fs, i))) {
			ext2fs_mark_block_bitmap(fs->block_map, blk);
			if (ext2fs_test_block_bitmap(old_fs->block_map, blk) &&
			    !ext2fs_test_block_bitmap(meta_bmap, blk))
				ext2fs_mark_block_bitmap(rfs->move_blocks,
							 blk);
		}
		if (FS_INODE_BM(old_fs, i) !=
		    (blk = FS_INODE_BM(fs, i))) {
			ext2fs_mark_block_bitmap(fs->block_map, blk);
			if (ext2fs_test_block_bitmap(old_fs->block_map, blk) &&
			    !ext2fs_test_block_bitmap(meta_bmap, blk))
				ext2fs_mark_block_bitmap(rfs->move_blocks,
							 blk);
		}

		/*
		 * The inode table, if we need to relocate it, is
		 * handled specially.  We have to reserve the blocks
		 * for both the old and the new inode table, since we
		 * can't have the inode table be destroyed during the
		 * block relocation phase.
		 */
		if (FS_INODE_TB(fs, i) == FS_INODE_TB(old_fs, i))
			goto next_group; /* inode table not moved */

		rfs->needed_blocks += fs->inode_blocks_per_group;

		/*
		 * Mark the new inode table as in use in the new block
		 * allocation bitmap, and move any blocks that might 
		 * be necessary.
		 */
		for (blk = fs->group_desc[i].bg_inode_table, j=0;
		     j < fs->inode_blocks_per_group ; j++, blk++) {
			ext2fs_mark_block_bitmap(fs->block_map, blk);
			if (ext2fs_test_block_bitmap(old_fs->block_map, blk) &&
			    !ext2fs_test_block_bitmap(meta_bmap, blk))
				ext2fs_mark_block_bitmap(rfs->move_blocks,
							 blk);
		}
		
		/*
		 * Make sure the old inode table is reserved in the
		 * block reservation bitmap.
		 */
		for (blk = rfs->old_fs->group_desc[i].bg_inode_table, j=0;
		     j < fs->inode_blocks_per_group ; j++, blk++)
			ext2fs_mark_block_bitmap(rfs->reserve_blocks, blk);
		
	next_group:
		group_blk += rfs->new_fs->super->s_blocks_per_group;
	}
	retval = 0;

errout:
	if (meta_bmap)
		ext2fs_free_block_bitmap(meta_bmap);
	
	return retval;
}

/*
 * This helper function tries to allocate a new block.  We try to
 * avoid hitting the original group descriptor blocks at least at
 * first, since we want to make it possible to recover from a badly
 * aborted resize operation as much as possible.
 *
 * In the future, I may further modify this routine to balance out
 * where we get the new blocks across the various block groups.
 * Ideally we would allocate blocks that corresponded with the block
 * group of the containing inode, and keep contiguous blocks
 * together.  However, this very difficult to do efficiently, since we
 * don't have the necessary information up front.
 */

#define AVOID_OLD	1
#define DESPERATION	2

static void init_block_alloc(ext2_resize_t rfs)
{
	rfs->alloc_state = AVOID_OLD;
	rfs->new_blk = rfs->new_fs->super->s_first_data_block;
#if 0
	/* HACK for testing */
	if (rfs->new_fs->super->s_blocks_count >
	    rfs->old_fs->super->s_blocks_count)
		rfs->new_blk = rfs->old_fs->super->s_blocks_count;
#endif
}

static blk_t get_new_block(ext2_resize_t rfs)
{
	ext2_filsys	fs = rfs->new_fs;
	
	while (1) {
		if (rfs->new_blk >= fs->super->s_blocks_count) {
			if (rfs->alloc_state == DESPERATION)
				return 0;

#ifdef RESIZE2FS_DEBUG
			if (rfs->flags & RESIZE_DEBUG_BMOVE)
				printf(_("Going into desperation "
				       "mode for block allocations\n"));
#endif			
			rfs->alloc_state = DESPERATION;
			rfs->new_blk = fs->super->s_first_data_block;
			continue;
		}
		if (ext2fs_test_block_bitmap(fs->block_map, rfs->new_blk) ||
		    ext2fs_test_block_bitmap(rfs->reserve_blocks,
					     rfs->new_blk) ||
		    ((rfs->alloc_state == AVOID_OLD) &&
		     (rfs->new_blk < rfs->old_fs->super->s_blocks_count) &&
		     ext2fs_test_block_bitmap(rfs->old_fs->block_map,
					      rfs->new_blk))) {
			rfs->new_blk++;
			continue;
		}
		return rfs->new_blk;
	}
}

static errcode_t block_mover(ext2_resize_t rfs)
{
	blk_t			blk, old_blk, new_blk;
	ext2_filsys		fs = rfs->new_fs;
	ext2_filsys		old_fs = rfs->old_fs;
	errcode_t		retval;
	int			size, c;
	int			to_move, moved;

	new_blk = fs->super->s_first_data_block;
	if (!rfs->itable_buf) {
		retval = ext2fs_get_mem(fs->blocksize *
					fs->inode_blocks_per_group,
					(void **) &rfs->itable_buf);
		if (retval)
			return retval;
	}
	retval = ext2fs_create_extent_table(&rfs->bmap, 0);
	if (retval)
		return retval;

	/*
	 * The first step is to figure out where all of the blocks
	 * will go.
	 */
	to_move = moved = 0;
	init_block_alloc(rfs);
	for (blk = old_fs->super->s_first_data_block;
	     blk < old_fs->super->s_blocks_count; blk++) {
		if (!ext2fs_test_block_bitmap(old_fs->block_map, blk))
			continue;
		if (!ext2fs_test_block_bitmap(rfs->move_blocks, blk))
			continue;

		new_blk = get_new_block(rfs);
		if (!new_blk) {
			retval = ENOSPC;
			goto errout;
		}
		ext2fs_mark_block_bitmap(fs->block_map, new_blk);
		ext2fs_add_extent_entry(rfs->bmap, blk, new_blk);
		to_move++;
	}
	
	if (to_move == 0) {
		retval = 0;
		goto errout;
	}

	/*
	 * Step two is to actually move the blocks
	 */
	retval =  ext2fs_iterate_extent(rfs->bmap, 0, 0, 0);
	if (retval) goto errout;

	if (rfs->progress) {
		retval = (rfs->progress)(rfs, E2_RSZ_BLOCK_RELOC_PASS,
					 0, to_move);
		if (retval)
			goto errout;
	}
	while (1) {
		retval = ext2fs_iterate_extent(rfs->bmap, &old_blk, &new_blk, &size);
		if (retval) goto errout;
		if (!size)
			break;
#ifdef RESIZE2FS_DEBUG
		if (rfs->flags & RESIZE_DEBUG_BMOVE)
			printf(_("Moving %d blocks %u->%u\n"), size,
			       old_blk, new_blk);
#endif
		do {
			c = size;
			if (c > fs->inode_blocks_per_group)
				c = fs->inode_blocks_per_group;
			retval = io_channel_read_blk(fs->io, old_blk, c,
						     rfs->itable_buf);
			if (retval) goto errout;
			retval = io_channel_write_blk(fs->io, new_blk, c,
						      rfs->itable_buf);
			if (retval) goto errout;
			size -= c;
			new_blk += c;
			old_blk += c;
			moved += c;
			if (rfs->progress) {
				io_channel_flush(fs->io);
				retval = (rfs->progress)(rfs,
						E2_RSZ_BLOCK_RELOC_PASS,
						moved, to_move);
				if (retval)
					goto errout;
			}
		} while (size > 0);
		io_channel_flush(fs->io);
	}

errout:
	return retval;
}


/* --------------------------------------------------------------------
 *
 * Resize processing, phase 3
 *
 * --------------------------------------------------------------------
 */


struct process_block_struct {
	ext2_resize_t 		rfs;
	ext2_ino_t		ino;
	struct ext2_inode *	inode;
	errcode_t		error;
	int			is_dir;
	int			changed;
};

static int process_block(ext2_filsys fs, blk_t	*block_nr,
			 e2_blkcnt_t blockcnt, blk_t ref_block,
			 int ref_offset, void *priv_data)
{
	struct process_block_struct *pb;
	errcode_t	retval;
	blk_t		block, new_block;
	int		ret = 0;

	pb = (struct process_block_struct *) priv_data;
	block = *block_nr;
	if (pb->rfs->bmap) {
		new_block = ext2fs_extent_translate(pb->rfs->bmap, block);
		if (new_block) {
			*block_nr = new_block;
			ret |= BLOCK_CHANGED;
			pb->changed = 1;
#ifdef RESIZE2FS_DEBUG
			if (pb->rfs->flags & RESIZE_DEBUG_BMOVE)
				printf(_("ino=%ld, blockcnt=%ld, %u->%u\n"), 
				       pb->ino, blockcnt, block, new_block);
#endif
			block = new_block;
		}
	}
	if (pb->is_dir) {
		retval = ext2fs_add_dir_block(fs->dblist, pb->ino,
					      block, (int) blockcnt);
		if (retval) {
			pb->error = retval;
			ret |= BLOCK_ABORT;
		}
	}
	return ret;
}

/*
 * Progress callback
 */
static errcode_t progress_callback(ext2_filsys fs, ext2_inode_scan scan,
				   dgrp_t group, void * priv_data)
{
	ext2_resize_t rfs = (ext2_resize_t) priv_data;
	errcode_t		retval;

	/*
	 * This check is to protect against old ext2 libraries.  It
	 * shouldn't be needed against new libraries.
	 */
	if ((group+1) == 0)
		return 0;

	if (rfs->progress) {
		io_channel_flush(fs->io);
		retval = (rfs->progress)(rfs, E2_RSZ_INODE_SCAN_PASS,
					 group+1, fs->group_desc_count);
		if (retval)
			return retval;
	}
	
	return 0;
}

static errcode_t inode_scan_and_fix(ext2_resize_t rfs)
{
	struct process_block_struct	pb;
	ext2_ino_t		ino, new_inode;
	struct ext2_inode 	inode;
	ext2_inode_scan 	scan = NULL;
	errcode_t		retval;
	int			group;
	char			*block_buf = 0;
	ext2_ino_t		start_to_move;
	blk_t			orig_size;
	
	if ((rfs->old_fs->group_desc_count <=
	     rfs->new_fs->group_desc_count) &&
	    !rfs->bmap)
		return 0;

	/*
	 * Save the original size of the old filesystem, and
	 * temporarily set the size to be the new size if the new size
	 * is larger.  We need to do this to avoid catching an error
	 * by the block iterator routines
	 */
	orig_size = rfs->old_fs->super->s_blocks_count;
	if (orig_size < rfs->new_fs->super->s_blocks_count)
		rfs->old_fs->super->s_blocks_count =
			rfs->new_fs->super->s_blocks_count;

	retval = ext2fs_open_inode_scan(rfs->old_fs, 0, &scan);
	if (retval) goto errout;

	retval = ext2fs_init_dblist(rfs->old_fs, 0);
	if (retval) goto errout;
	retval = ext2fs_get_mem(rfs->old_fs->blocksize * 3,
				(void **) &block_buf);
	if (retval) goto errout;

	start_to_move = (rfs->new_fs->group_desc_count *
			 rfs->new_fs->super->s_inodes_per_group);
	
	if (rfs->progress) {
		retval = (rfs->progress)(rfs, E2_RSZ_INODE_SCAN_PASS,
					 0, rfs->old_fs->group_desc_count);
		if (retval)
			goto errout;
	}
	ext2fs_set_inode_callback(scan, progress_callback, (void *) rfs);
	pb.rfs = rfs;
	pb.inode = &inode;
	pb.error = 0;
	new_inode = EXT2_FIRST_INODE(rfs->new_fs->super);
	/*
	 * First, copy all of the inodes that need to be moved
	 * elsewhere in the inode table
	 */
	while (1) {
		retval = ext2fs_get_next_inode(scan, &ino, &inode);
		if (retval) goto errout;
		if (!ino)
			break;

		if (inode.i_links_count == 0)
			continue; /* inode not in use */

		pb.is_dir = LINUX_S_ISDIR(inode.i_mode);
		pb.changed = 0;

		if (ext2fs_inode_has_valid_blocks(&inode) &&
		    (rfs->bmap || pb.is_dir)) {
			pb.ino = ino;
			retval = ext2fs_block_iterate2(rfs->old_fs,
						       ino, 0, block_buf,
						       process_block, &pb);
			if (retval)
				goto errout;
			if (pb.error) {
				retval = pb.error;
				goto errout;
			}
		}

		if (ino <= start_to_move)
			continue; /* Don't need to move it. */

		/*
		 * Find a new inode
		 */
		while (1) { 
			if (!ext2fs_test_inode_bitmap(rfs->new_fs->inode_map, 
						      new_inode))
				break;
			new_inode++;
			if (new_inode > rfs->new_fs->super->s_inodes_count) {
				retval = ENOSPC;
				goto errout;
			}
		}
		ext2fs_mark_inode_bitmap(rfs->new_fs->inode_map, new_inode);
		if (pb.changed) {
			/* Get the new version of the inode */
			retval = ext2fs_read_inode(rfs->old_fs, ino, &inode);
			if (retval) goto errout;
		}
		retval = ext2fs_write_inode(rfs->old_fs, new_inode, &inode);
		if (retval) goto errout;

		group = (new_inode-1) / EXT2_INODES_PER_GROUP(rfs->new_fs->super);
		if (LINUX_S_ISDIR(inode.i_mode))
			rfs->new_fs->group_desc[group].bg_used_dirs_count++;
		
#ifdef RESIZE2FS_DEBUG
		if (rfs->flags & RESIZE_DEBUG_INODEMAP)
			printf(_("Inode moved %ld->%ld\n"), ino, new_inode);
#endif
		if (!rfs->imap) {
			retval = ext2fs_create_extent_table(&rfs->imap, 0);
			if (retval)
				goto errout;
		}
		ext2fs_add_extent_entry(rfs->imap, ino, new_inode);
	}
	io_channel_flush(rfs->old_fs->io);

errout:
	rfs->old_fs->super->s_blocks_count = orig_size;
	if (rfs->bmap) {
		ext2fs_free_extent_table(rfs->bmap);
		rfs->bmap = 0;
	}
	if (scan)
		ext2fs_close_inode_scan(scan);
	if (block_buf)
		ext2fs_free_mem((void **) &block_buf);
	return retval;
}

/* --------------------------------------------------------------------
 *
 * Resize processing, phase 4.
 *
 * --------------------------------------------------------------------
 */

struct istruct {
	ext2_resize_t rfs;
	errcode_t	err;
	unsigned long	max_dirs;
	int		num;
};

static int check_and_change_inodes(ext2_ino_t dir, int entry,
				   struct ext2_dir_entry *dirent, int offset,
				   int	blocksize, char *buf, void *priv_data)
{
	struct istruct *is = (struct istruct *) priv_data;
	ext2_ino_t	new_inode;

	if (is->rfs->progress && offset == 0) {
		io_channel_flush(is->rfs->old_fs->io);
		is->err = (is->rfs->progress)(is->rfs,
					      E2_RSZ_INODE_REF_UPD_PASS,
					      ++is->num, is->max_dirs);
		if (is->err)
			return DIRENT_ABORT;
	}

	if (!dirent->inode)
		return 0;

	new_inode = ext2fs_extent_translate(is->rfs->imap, dirent->inode);

	if (!new_inode)
		return 0;
#ifdef RESIZE2FS_DEBUG
	if (is->rfs->flags & RESIZE_DEBUG_INODEMAP)
		printf(_("Inode translate (dir=%ld, name=%.*s, %u->%ld)\n"),
		       dir, dirent->name_len, dirent->name,
		       dirent->inode, new_inode);
#endif

	dirent->inode = new_inode;

	return DIRENT_CHANGED;
}

static errcode_t inode_ref_fix(ext2_resize_t rfs)
{
	errcode_t		retval;
	struct istruct 		is;
	
	if (!rfs->imap)
		return 0;
       
	/*
	 * Now, we iterate over all of the directories to update the
	 * inode references
	 */
	is.num = 0;
	is.max_dirs = ext2fs_dblist_count(rfs->old_fs->dblist);
	is.rfs = rfs;
	is.err = 0;

	if (rfs->progress) {
		retval = (rfs->progress)(rfs, E2_RSZ_INODE_REF_UPD_PASS,
					 0, is.max_dirs);
		if (retval)
			goto errout;
	}
	
	retval = ext2fs_dblist_dir_iterate(rfs->old_fs->dblist,
					   DIRENT_FLAG_INCLUDE_EMPTY, 0,
					   check_and_change_inodes, &is);
	if (retval)
		goto errout;
	if (is.err) {
		retval = is.err;
		goto errout;
	}

errout:
	ext2fs_free_extent_table(rfs->imap);
	rfs->imap = 0;
	return retval;
}


/* --------------------------------------------------------------------
 *
 * Resize processing, phase 5.
 *
 * In this phase we actually move the inode table around, and then
 * update the summary statistics.  This is scary, since aborting here
 * will potentially scramble the filesystem.  (We are moving the
 * inode tables around in place, and so the potential for lost data,
 * or at the very least scrambling the mapping between filenames and
 * inode numbers is very high in case of a power failure here.)
 * --------------------------------------------------------------------
 */


/*
 * A very scary routine --- this one moves the inode table around!!!
 *
 * After this you have to use the rfs->new_fs file handle to read and
 * write inodes.
 */
static errcode_t move_itables(ext2_resize_t rfs)
{
	int		i, n, num, max_groups, size, diff;
	ext2_filsys	fs = rfs->new_fs;
	char		*cp;
	blk_t		old_blk, new_blk;
	errcode_t	retval;
	int		to_move, moved;

	max_groups = fs->group_desc_count;
	if (max_groups > rfs->old_fs->group_desc_count)
		max_groups = rfs->old_fs->group_desc_count;

	size = fs->blocksize * fs->inode_blocks_per_group;
	if (!rfs->itable_buf) {
		retval = ext2fs_get_mem(size, (void **) &rfs->itable_buf);
		if (retval)
			return retval;
	}

	/*
	 * Figure out how many inode tables we need to move
	 */
	to_move = moved = 0;
	for (i=0; i < max_groups; i++)
		if (rfs->old_fs->group_desc[i].bg_inode_table !=
		    fs->group_desc[i].bg_inode_table)
			to_move++;

	if (to_move == 0)
		return 0;

	if (rfs->progress) {
		retval = rfs->progress(rfs, E2_RSZ_MOVE_ITABLE_PASS,
				       0, to_move);
		if (retval)
			goto errout;
	}

	rfs->old_fs->flags |= EXT2_FLAG_MASTER_SB_ONLY;

	for (i=0; i < max_groups; i++) {
		old_blk = rfs->old_fs->group_desc[i].bg_inode_table;
		new_blk = fs->group_desc[i].bg_inode_table;
		diff = new_blk - old_blk;
		
#ifdef RESIZE2FS_DEBUG
		if (rfs->flags & RESIZE_DEBUG_ITABLEMOVE) 
			printf(_("Itable move group %d block "
			       "%u->%u (diff %d)\n"), 
			       i, old_blk, new_blk, diff);
#endif
		
		if (!diff)
			continue;

		retval = io_channel_read_blk(fs->io, old_blk,
					     fs->inode_blocks_per_group,
					     rfs->itable_buf);
		if (retval) 
			goto errout;
		/*
		 * The end of the inode table segment often contains
		 * all zeros, and we're often only moving the inode
		 * table down a block or two.  If so, we can optimize
		 * things by not rewriting blocks that we know to be zero
		 * already.
		 */
		for (cp = rfs->itable_buf+size, n=0; n < size; n++, cp--)
			if (*cp)
				break;
		n = n >> EXT2_BLOCK_SIZE_BITS(fs->super);
#ifdef RESIZE2FS_DEBUG
		if (rfs->flags & RESIZE_DEBUG_ITABLEMOVE) 
			printf(_("%d blocks of zeros...\n"), n);
#endif
		num = fs->inode_blocks_per_group;
		if (n > diff)
			num -= n;

		retval = io_channel_write_blk(fs->io, new_blk,
					      num, rfs->itable_buf);
		if (retval) {
			io_channel_write_blk(fs->io, old_blk,
					     num, rfs->itable_buf);
			goto errout;
		}
		if (n > diff) {
			retval = io_channel_write_blk(fs->io,
			      old_blk + fs->inode_blocks_per_group,
			      diff, (rfs->itable_buf +
				     (fs->inode_blocks_per_group - diff) *
				     fs->blocksize));
			if (retval)
				goto errout;
		}
		rfs->old_fs->group_desc[i].bg_inode_table = new_blk;
		ext2fs_mark_super_dirty(rfs->old_fs);
		if (rfs->progress) {
			ext2fs_flush(rfs->old_fs);
			retval = rfs->progress(rfs, E2_RSZ_MOVE_ITABLE_PASS,
					       ++moved, to_move);
			if (retval)
				goto errout;
		}
	}
	ext2fs_flush(fs);
#ifdef RESIZE2FS_DEBUG
	if (rfs->flags & RESIZE_DEBUG_ITABLEMOVE) 
		printf(_("Inode table move finished.\n"));
#endif
	return 0;
	
errout:
	return retval;
}

/*
 * Finally, recalculate the summary information
 */
static errcode_t ext2fs_calculate_summary_stats(ext2_filsys fs)
{
	blk_t		blk;
	ext2_ino_t	ino;
	int		group = 0;
	int		count = 0;
	int		total_free = 0;
	int		group_free = 0;

	/*
	 * First calculate the block statistics
	 */
	for (blk = fs->super->s_first_data_block;
	     blk < fs->super->s_blocks_count; blk++) {
		if (!ext2fs_fast_test_block_bitmap(fs->block_map, blk)) {
			group_free++;
			total_free++;
		}
		count++;
		if ((count == fs->super->s_blocks_per_group) ||
		    (blk == fs->super->s_blocks_count-1)) {
			fs->group_desc[group++].bg_free_blocks_count =
				group_free;
			count = 0;
			group_free = 0;
		}
	}
	fs->super->s_free_blocks_count = total_free;
	
	/*
	 * Next, calculate the inode statistics
	 */
	group_free = 0;
	total_free = 0;
	count = 0;
	group = 0;
	for (ino = 1; ino <= fs->super->s_inodes_count; ino++) {
		if (!ext2fs_fast_test_inode_bitmap(fs->inode_map, ino)) {
			group_free++;
			total_free++;
		}
		count++;
		if ((count == fs->super->s_inodes_per_group) ||
		    (ino == fs->super->s_inodes_count)) {
			fs->group_desc[group++].bg_free_inodes_count =
				group_free;
			count = 0;
			group_free = 0;
		}
	}
	fs->super->s_free_inodes_count = total_free;
	ext2fs_mark_super_dirty(fs);
	return 0;
}
