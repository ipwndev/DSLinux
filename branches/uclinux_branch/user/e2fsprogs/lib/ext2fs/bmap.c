/*
 * bmap.c --- logical to physical block mapping
 *
 * Copyright (C) 1997 Theodore Ts'o.
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

#include "ext2_fs.h"
#include "ext2fs.h"

#if defined(__GNUC__) && !defined(NO_INLINE_FUNCS)
#define _BMAP_INLINE_	__inline__
#else
#define _BMAP_INLINE_
#endif

extern errcode_t ext2fs_bmap(ext2_filsys fs, ext2_ino_t ino,
			     struct ext2_inode *inode, 
			     char *block_buf, int bmap_flags,
			     blk_t block, blk_t *phys_blk);

#define BMAP_ALLOC	1

#define inode_bmap(inode, nr) ((inode)->i_block[(nr)])

static errcode_t _BMAP_INLINE_ block_ind_bmap(ext2_filsys fs, int flags, 
					      blk_t ind, char *block_buf, 
					      int *blocks_alloc,
					      blk_t nr, blk_t *ret_blk)
{
	errcode_t	retval;
	blk_t		b;

	if (!ind) {
		*ret_blk = 0;
		return 0;
	}
	retval = io_channel_read_blk(fs->io, ind, 1, block_buf);
	if (retval)
		return retval;

	b = ((blk_t *) block_buf)[nr];

#ifdef EXT2FS_ENABLE_SWAPFS
	if ((fs->flags & EXT2_FLAG_SWAP_BYTES) ||
	    (fs->flags & EXT2_FLAG_SWAP_BYTES_READ))
		b = ext2fs_swab32(b);
#endif

	if (!b && (flags & BMAP_ALLOC)) {
		b = nr ? ((blk_t *) block_buf)[nr-1] : 0;
		retval = ext2fs_alloc_block(fs, b,
					    block_buf + fs->blocksize, &b);
		if (retval)
			return retval;

#ifdef EXT2FS_ENABLE_SWAPFS
		if ((fs->flags & EXT2_FLAG_SWAP_BYTES) ||
		    (fs->flags & EXT2_FLAG_SWAP_BYTES_WRITE))
			((blk_t *) block_buf)[nr] = ext2fs_swab32(b);
		else
#endif
			((blk_t *) block_buf)[nr] = b;

		retval = io_channel_write_blk(fs->io, ind, 1, block_buf);
		if (retval)
			return retval;

		(*blocks_alloc)++;
	}

	*ret_blk = b;
	return 0;
}

static errcode_t _BMAP_INLINE_ block_dind_bmap(ext2_filsys fs, int flags,
					       blk_t dind, char *block_buf, 
					       int *blocks_alloc,
					       blk_t nr, blk_t *ret_blk)
{
	blk_t		b;
	errcode_t	retval;
	blk_t		addr_per_block;
	
	addr_per_block = (blk_t) fs->blocksize >> 2;

	retval = block_ind_bmap(fs, flags, dind, block_buf, blocks_alloc,
				nr / addr_per_block, &b);
	if (retval)
		return retval;
	retval = block_ind_bmap(fs, flags, b, block_buf, blocks_alloc,
				nr % addr_per_block, ret_blk);
	return retval;
}

static errcode_t _BMAP_INLINE_ block_tind_bmap(ext2_filsys fs, int flags,
					       blk_t tind, char *block_buf, 
					       int *blocks_alloc,
					       blk_t nr, blk_t *ret_blk)
{
	blk_t		b;
	errcode_t	retval;
	blk_t		addr_per_block;
	
	addr_per_block = (blk_t) fs->blocksize >> 2;

	retval = block_dind_bmap(fs, flags, tind, block_buf, blocks_alloc,
				 nr / addr_per_block, &b);
	if (retval)
		return retval;
	retval = block_ind_bmap(fs, flags, b, block_buf, blocks_alloc,
				nr % addr_per_block, ret_blk);
	return retval;
}

errcode_t ext2fs_bmap(ext2_filsys fs, ext2_ino_t ino, struct ext2_inode *inode,
		      char *block_buf, int bmap_flags, blk_t block,
		      blk_t *phys_blk)
{
	struct ext2_inode inode_buf;
	blk_t addr_per_block;
	blk_t	b;
	char	*buf = 0;
	errcode_t	retval = 0;
	int		blocks_alloc = 0;

	*phys_blk = 0;

	/* Read inode structure if necessary */
	if (!inode) {
		retval = ext2fs_read_inode(fs, ino, &inode_buf);
		if (!retval)
			return retval;
		inode = &inode_buf;
	}
	addr_per_block = (blk_t) fs->blocksize >> 2;

	if (!block_buf) {
		retval = ext2fs_get_mem(fs->blocksize * 2, (void **) &buf);
		if (retval)
			return retval;
		block_buf = buf;
	}

	if (block < EXT2_NDIR_BLOCKS) {
		*phys_blk = inode_bmap(inode, block);
		b = block ? inode_bmap(inode, block-1) : 0;
		
		if ((*phys_blk == 0) && (bmap_flags & BMAP_ALLOC)) {
			retval = ext2fs_alloc_block(fs, b, block_buf, &b);
			if (retval)
				goto done;
			inode_bmap(inode, block) = b;
			blocks_alloc++;
			*phys_blk = b;
		}
		goto done;
	}
	
	/* Indirect block */
	block -= EXT2_NDIR_BLOCKS;
	if (block < addr_per_block) {
		b = inode_bmap(inode, EXT2_IND_BLOCK);
		if (!b) {
			if (!(bmap_flags & BMAP_ALLOC))
			    goto done;

			b = inode_bmap(inode, EXT2_IND_BLOCK-1);
 			retval = ext2fs_alloc_block(fs, b, block_buf, &b);
			if (retval)
				goto done;
			inode_bmap(inode, EXT2_IND_BLOCK) = b;
			blocks_alloc++;
		}
		retval = block_ind_bmap(fs, bmap_flags, b, block_buf, 
					&blocks_alloc, block, phys_blk);
		goto done;
	}
	
	/* Doubly indirect block  */
	block -= addr_per_block;
	if (block < addr_per_block * addr_per_block) {
		b = inode_bmap(inode, EXT2_DIND_BLOCK);
		if (!b) {
			if (!(bmap_flags & BMAP_ALLOC))
			    goto done;

			b = inode_bmap(inode, EXT2_IND_BLOCK);
 			retval = ext2fs_alloc_block(fs, b, block_buf, &b);
			if (retval)
				goto done;
			inode_bmap(inode, EXT2_DIND_BLOCK) = b;
			blocks_alloc++;
		}
		retval = block_dind_bmap(fs, bmap_flags, b, block_buf, 
					 &blocks_alloc, block, phys_blk);
		goto done;
	}

	/* Triply indirect block */
	block -= addr_per_block * addr_per_block;
	b = inode_bmap(inode, EXT2_TIND_BLOCK);
	if (!b) {
		if (!(bmap_flags & BMAP_ALLOC))
			goto done;

		b = inode_bmap(inode, EXT2_DIND_BLOCK);
		retval = ext2fs_alloc_block(fs, b, block_buf, &b);
		if (retval)
			goto done;
		inode_bmap(inode, EXT2_TIND_BLOCK) = b;
		blocks_alloc++;
	}
	retval = block_tind_bmap(fs, bmap_flags, b, block_buf, 
				 &blocks_alloc, block, phys_blk);
done:
	if (buf)
		ext2fs_free_mem((void **) &buf);
	if ((retval == 0) && blocks_alloc) {
		inode->i_blocks += (blocks_alloc * fs->blocksize) / 512;
		retval = ext2fs_write_inode(fs, ino, inode);
	}
	return retval;
}



