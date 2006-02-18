/*
 * pass3.c -- pass #3 of e2fsck: Check for directory connectivity
 *
 * Copyright (C) 1993, 1994, 1995, 1996, 1997, 1998, 1999 Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 * 
 * Pass #3 assures that all directories are connected to the
 * filesystem tree, using the following algorithm:
 *
 * First, the root directory is checked to make sure it exists; if
 * not, e2fsck will offer to create a new one.  It is then marked as
 * "done".
 * 
 * Then, pass3 interates over all directory inodes; for each directory
 * it attempts to trace up the filesystem tree, using dirinfo.parent
 * until it reaches a directory which has been marked "done".  If it
 * can not do so, then the directory must be disconnected, and e2fsck
 * will offer to reconnect it to /lost+found.  While it is chasing
 * parent pointers up the filesystem tree, if pass3 sees a directory
 * twice, then it has detected a filesystem loop, and it will again
 * offer to reconnect the directory to /lost+found in to break the
 * filesystem loop.
 * 
 * Pass 3 also contains the subroutine, e2fsck_reconnect_file() to
 * reconnect inodes to /lost+found; this subroutine is also used by
 * pass 4.  e2fsck_reconnect_file() calls get_lost_and_found(), which
 * is responsible for creating /lost+found if it does not exist.
 *
 * Pass 3 frees the following data structures:
 *     	- The dirinfo directory information cache.
 */

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#include "e2fsck.h"
#include "problem.h"

static void check_root(e2fsck_t ctx);
static int check_directory(e2fsck_t ctx, struct dir_info *dir,
			   struct problem_context *pctx);
static ext2_ino_t get_lost_and_found(e2fsck_t ctx);
static void fix_dotdot(e2fsck_t ctx, struct dir_info *dir, ext2_ino_t parent);
static errcode_t adjust_inode_count(e2fsck_t ctx, ext2_ino_t ino, int adj);
static errcode_t expand_directory(e2fsck_t ctx, ext2_ino_t dir);

static ext2_ino_t lost_and_found = 0;
static int bad_lost_and_found = 0;

static ext2fs_inode_bitmap inode_loop_detect = 0;
static ext2fs_inode_bitmap inode_done_map = 0;
	
void e2fsck_pass3(e2fsck_t ctx)
{
	ext2_filsys fs = ctx->fs;
	int		i;
#ifdef RESOURCE_TRACK
	struct resource_track	rtrack;
#endif
	struct problem_context	pctx;
	struct dir_info	*dir;
	unsigned long maxdirs, count;

#ifdef RESOURCE_TRACK
	init_resource_track(&rtrack);
#endif

	clear_problem_context(&pctx);

#ifdef MTRACE
	mtrace_print("Pass 3");
#endif

	if (!(ctx->options & E2F_OPT_PREEN))
		fix_problem(ctx, PR_3_PASS_HEADER, &pctx);

	/*
	 * Allocate some bitmaps to do loop detection.
	 */
	pctx.errcode = ext2fs_allocate_inode_bitmap(fs, _("inode done bitmap"),
						    &inode_done_map);
	if (pctx.errcode) {
		pctx.num = 2;
		fix_problem(ctx, PR_3_ALLOCATE_IBITMAP_ERROR, &pctx);
		ctx->flags |= E2F_FLAG_ABORT;
		goto abort_exit;
	}
#ifdef RESOURCE_TRACK
	if (ctx->options & E2F_OPT_TIME) {
		e2fsck_clear_progbar(ctx);
		print_resource_track(_("Peak memory"), &ctx->global_rtrack);
	}
#endif

	check_root(ctx);
	if (ctx->flags & E2F_FLAG_SIGNAL_MASK)
		goto abort_exit;

	ext2fs_mark_inode_bitmap(inode_done_map, EXT2_ROOT_INO);

	maxdirs = e2fsck_get_num_dirinfo(ctx);
	count = 1;

	if (ctx->progress)
		if ((ctx->progress)(ctx, 3, 0, maxdirs))
			goto abort_exit;
	
	for (i=0; (dir = e2fsck_dir_info_iter(ctx, &i)) != 0;) {
		if (ctx->progress)
			if ((ctx->progress)(ctx, 3, count++, maxdirs))
				goto abort_exit;
		if (ext2fs_test_inode_bitmap(ctx->inode_dir_map, dir->ino))
			if (check_directory(ctx, dir, &pctx))
				goto abort_exit;
	}

	/*
	 * Force the creation of /lost+found if not present
	 */
	if ((ctx->flags & E2F_OPT_READONLY) == 0)
		get_lost_and_found(ctx);

abort_exit:
	e2fsck_free_dir_info(ctx);
	if (inode_loop_detect) {
		ext2fs_free_inode_bitmap(inode_loop_detect);
		inode_loop_detect = 0;
	}
	if (inode_done_map) {
		ext2fs_free_inode_bitmap(inode_done_map);
		inode_done_map = 0;
	}
#ifdef RESOURCE_TRACK
	if (ctx->options & E2F_OPT_TIME2) {
		e2fsck_clear_progbar(ctx);
		print_resource_track(_("Pass 3"), &rtrack);
	}
#endif
}

/*
 * This makes sure the root inode is present; if not, we ask if the
 * user wants us to create it.  Not creating it is a fatal error.
 */
static void check_root(e2fsck_t ctx)
{
	ext2_filsys fs = ctx->fs;
	blk_t			blk;
	struct ext2_inode	inode;
	char *			block;
	struct problem_context	pctx;
	
	clear_problem_context(&pctx);
	
	if (ext2fs_test_inode_bitmap(ctx->inode_used_map, EXT2_ROOT_INO)) {
		/*
		 * If the root inode is not a directory, die here.  The
		 * user must have answered 'no' in pass1 when we
		 * offered to clear it.
		 */
		if (!(ext2fs_test_inode_bitmap(ctx->inode_dir_map,
					       EXT2_ROOT_INO))) {
			fix_problem(ctx, PR_3_ROOT_NOT_DIR_ABORT, &pctx);
			ctx->flags |= E2F_FLAG_ABORT;
		}
		return;
	}

	if (!fix_problem(ctx, PR_3_NO_ROOT_INODE, &pctx)) {
		fix_problem(ctx, PR_3_NO_ROOT_INODE_ABORT, &pctx);
		ctx->flags |= E2F_FLAG_ABORT;
		return;
	}

	e2fsck_read_bitmaps(ctx);
	
	/*
	 * First, find a free block
	 */
	pctx.errcode = ext2fs_new_block(fs, 0, ctx->block_found_map, &blk);
	if (pctx.errcode) {
		pctx.str = "ext2fs_new_block";
		fix_problem(ctx, PR_3_CREATE_ROOT_ERROR, &pctx);
		ctx->flags |= E2F_FLAG_ABORT;
		return;
	}
	ext2fs_mark_block_bitmap(ctx->block_found_map, blk);
	ext2fs_mark_block_bitmap(fs->block_map, blk);
	ext2fs_mark_bb_dirty(fs);

	/*
	 * Now let's create the actual data block for the inode
	 */
	pctx.errcode = ext2fs_new_dir_block(fs, EXT2_ROOT_INO, EXT2_ROOT_INO,
					    &block);
	if (pctx.errcode) {
		pctx.str = "ext2fs_new_dir_block";
		fix_problem(ctx, PR_3_CREATE_ROOT_ERROR, &pctx);
		ctx->flags |= E2F_FLAG_ABORT;
		return;
	}

	pctx.errcode = ext2fs_write_dir_block(fs, blk, block);
	if (pctx.errcode) {
		pctx.str = "ext2fs_write_dir_block";
		fix_problem(ctx, PR_3_CREATE_ROOT_ERROR, &pctx);
		ctx->flags |= E2F_FLAG_ABORT;
		return;
	}
	ext2fs_free_mem((void **) &block);

	/*
	 * Set up the inode structure
	 */
	memset(&inode, 0, sizeof(inode));
	inode.i_mode = 040755;
	inode.i_size = fs->blocksize;
	inode.i_atime = inode.i_ctime = inode.i_mtime = time(0);
	inode.i_links_count = 2;
	inode.i_blocks = fs->blocksize / 512;
	inode.i_block[0] = blk;

	/*
	 * Write out the inode.
	 */
	pctx.errcode = ext2fs_write_inode(fs, EXT2_ROOT_INO, &inode);
	if (pctx.errcode) {
		pctx.str = "ext2fs_write_inode";
		fix_problem(ctx, PR_3_CREATE_ROOT_ERROR, &pctx);
		ctx->flags |= E2F_FLAG_ABORT;
		return;
	}
	
	/*
	 * Miscellaneous bookkeeping...
	 */
	e2fsck_add_dir_info(ctx, EXT2_ROOT_INO, EXT2_ROOT_INO);
	ext2fs_icount_store(ctx->inode_count, EXT2_ROOT_INO, 2);
	ext2fs_icount_store(ctx->inode_link_info, EXT2_ROOT_INO, 2);

	ext2fs_mark_inode_bitmap(ctx->inode_used_map, EXT2_ROOT_INO);
	ext2fs_mark_inode_bitmap(ctx->inode_dir_map, EXT2_ROOT_INO);
	ext2fs_mark_inode_bitmap(fs->inode_map, EXT2_ROOT_INO);
	ext2fs_mark_ib_dirty(fs);
}

/*
 * This subroutine is responsible for making sure that a particular
 * directory is connected to the root; if it isn't we trace it up as
 * far as we can go, and then offer to connect the resulting parent to
 * the lost+found.  We have to do loop detection; if we ever discover
 * a loop, we treat that as a disconnected directory and offer to
 * reparent it to lost+found.
 * 
 * However, loop detection is expensive, because for very large
 * filesystems, the inode_loop_detect bitmap is huge, and clearing it
 * is non-trivial.  Loops in filesystems are also a rare error case,
 * and we shouldn't optimize for error cases.  So we try two passes of
 * the algorithm.  The first time, we ignore loop detection and merely
 * increment a counter; if the counter exceeds some extreme threshold,
 * then we try again with the loop detection bitmap enabled.
 */
static int check_directory(e2fsck_t ctx, struct dir_info *dir,
			   struct problem_context *pctx)
{
	ext2_filsys 	fs = ctx->fs;
	struct dir_info *p = dir;
	int		loop_pass = 0, parent_count = 0;

	if (!p)
		return 0;

	while (1) {
		/*
		 * Mark this inode as being "done"; by the time we
		 * return from this function, the inode we either be
		 * verified as being connected to the directory tree,
		 * or we will have offered to reconnect this to
		 * lost+found.
		 *
		 * If it was marked done already, then we've reached a
		 * parent we've already checked.
		 */
	  	if (ext2fs_mark_inode_bitmap(inode_done_map, p->ino))
			break;

		/*
		 * If this directory doesn't have a parent, or we've
		 * seen the parent once already, then offer to
		 * reparent it to lost+found
		 */
		if (!p->parent ||
		    (loop_pass && 
		     (ext2fs_test_inode_bitmap(inode_loop_detect,
					      p->parent)))) {
			pctx->ino = p->ino;
			if (fix_problem(ctx, PR_3_UNCONNECTED_DIR, pctx)) {
				if (e2fsck_reconnect_file(ctx, p->ino))
					ext2fs_unmark_valid(fs);
				else {
					p->parent = lost_and_found;
					fix_dotdot(ctx, p, lost_and_found);
				}
			}
			break;
		}
		p = e2fsck_get_dir_info(ctx, p->parent);
		if (!p) {
			fix_problem(ctx, PR_3_NO_DIRINFO, pctx);
			return 0;
		}
		if (loop_pass) {
			ext2fs_mark_inode_bitmap(inode_loop_detect,
						 p->ino);
		} else if (parent_count++ > 2048) {
			/*
			 * If we've run into a path depth that's
			 * greater than 2048, try again with the inode
			 * loop bitmap turned on and start from the
			 * top.
			 */
			loop_pass = 1;
			if (inode_loop_detect)
				ext2fs_clear_inode_bitmap(inode_loop_detect);
			else {
				pctx->errcode = ext2fs_allocate_inode_bitmap(fs, _("inode loop detection bitmap"), &inode_loop_detect);
				if (pctx->errcode) {
					pctx->num = 1;
					fix_problem(ctx, 
				    PR_3_ALLOCATE_IBITMAP_ERROR, pctx);
					ctx->flags |= E2F_FLAG_ABORT;
					return -1;
				}
			}
			p = dir;
		}
	}

	/*
	 * Make sure that .. and the parent directory are the same;
	 * offer to fix it if not.
	 */
	if (dir->parent != dir->dotdot) {
		pctx->ino = dir->ino;
		pctx->ino2 = dir->dotdot;
		pctx->dir = dir->parent;
		if (fix_problem(ctx, PR_3_BAD_DOT_DOT, pctx))
			fix_dotdot(ctx, dir, dir->parent);
	}
	return 0;
}	

/*
 * This routine gets the lost_and_found inode, making it a directory
 * if necessary
 */
static ext2_ino_t get_lost_and_found(e2fsck_t ctx)
{
	ext2_filsys fs = ctx->fs;
	ext2_ino_t			ino;
	blk_t			blk;
	errcode_t		retval;
	struct ext2_inode	inode;
	char *			block;
	static const char	name[] = "lost+found";
	struct 	problem_context	pctx;
	struct dir_info 	*dirinfo;

	clear_problem_context(&pctx);
	
	retval = ext2fs_lookup(fs, EXT2_ROOT_INO, name,
			       sizeof(name)-1, 0, &ino);
	if (!retval) {
		if (ext2fs_test_inode_bitmap(ctx->inode_dir_map, ino))
			return ino;
		/* Lost+found isn't a directory! */
		pctx.ino = ino;
		if (!fix_problem(ctx, PR_3_LPF_NOTDIR, &pctx))
			return 0;

		/* OK, unlink the old /lost+found file. */
		pctx.errcode = ext2fs_unlink(fs, EXT2_ROOT_INO, name, ino, 0);
		if (pctx.errcode) {
			pctx.str = "ext2fs_unlink";
			fix_problem(ctx, PR_3_CREATE_LPF_ERROR, &pctx);
			return 0;
		}
		dirinfo = e2fsck_get_dir_info(ctx, ino);
		if (dirinfo)
			dirinfo->parent = 0;
		adjust_inode_count(ctx, ino, -1);
	} else if (retval != EXT2_ET_FILE_NOT_FOUND) {
		pctx.errcode = retval;
		fix_problem(ctx, PR_3_ERR_FIND_LPF, &pctx);
	}
	if (!fix_problem(ctx, PR_3_NO_LF_DIR, 0))
		return 0;

	/*
	 * Read the inode and block bitmaps in; we'll be messing with
	 * them.
	 */
	e2fsck_read_bitmaps(ctx);
	
	/*
	 * First, find a free block
	 */
	retval = ext2fs_new_block(fs, 0, ctx->block_found_map, &blk);
	if (retval) {
		pctx.errcode = retval;
		fix_problem(ctx, PR_3_ERR_LPF_NEW_BLOCK, &pctx);
		return 0;
	}
	ext2fs_mark_block_bitmap(ctx->block_found_map, blk);
	ext2fs_mark_block_bitmap(fs->block_map, blk);
	ext2fs_mark_bb_dirty(fs);

	/*
	 * Next find a free inode.
	 */
	retval = ext2fs_new_inode(fs, EXT2_ROOT_INO, 040755,
				  ctx->inode_used_map, &ino);
	if (retval) {
		pctx.errcode = retval;
		fix_problem(ctx, PR_3_ERR_LPF_NEW_INODE, &pctx);
		return 0;
	}
	ext2fs_mark_inode_bitmap(ctx->inode_used_map, ino);
	ext2fs_mark_inode_bitmap(ctx->inode_dir_map, ino);
	ext2fs_mark_inode_bitmap(fs->inode_map, ino);
	ext2fs_mark_ib_dirty(fs);

	/*
	 * Now let's create the actual data block for the inode
	 */
	retval = ext2fs_new_dir_block(fs, ino, EXT2_ROOT_INO, &block);
	if (retval) {
		pctx.errcode = retval;
		fix_problem(ctx, PR_3_ERR_LPF_NEW_DIR_BLOCK, &pctx);
		return 0;
	}

	retval = ext2fs_write_dir_block(fs, blk, block);
	ext2fs_free_mem((void **) &block);
	if (retval) {
		pctx.errcode = retval;
		fix_problem(ctx, PR_3_ERR_LPF_WRITE_BLOCK, &pctx);
		return 0;
	}

	/*
	 * Set up the inode structure
	 */
	memset(&inode, 0, sizeof(inode));
	inode.i_mode = 040755;
	inode.i_size = fs->blocksize;
	inode.i_atime = inode.i_ctime = inode.i_mtime = time(0);
	inode.i_links_count = 2;
	inode.i_blocks = fs->blocksize / 512;
	inode.i_block[0] = blk;

	/*
	 * Next, write out the inode.
	 */
	pctx.errcode = ext2fs_write_inode(fs, ino, &inode);
	if (pctx.errcode) {
		pctx.str = "ext2fs_write_inode";
		fix_problem(ctx, PR_3_CREATE_LPF_ERROR, &pctx);
		return 0;
	}
	/*
	 * Finally, create the directory link
	 */
	pctx.errcode = ext2fs_link(fs, EXT2_ROOT_INO, name, ino, EXT2_FT_DIR);
	if (pctx.errcode) {
		pctx.str = "ext2fs_link";
		fix_problem(ctx, PR_3_CREATE_LPF_ERROR, &pctx);
		return 0;
	}

	/*
	 * Miscellaneous bookkeeping that needs to be kept straight.
	 */
	e2fsck_add_dir_info(ctx, ino, EXT2_ROOT_INO);
	adjust_inode_count(ctx, EXT2_ROOT_INO, 1);
	ext2fs_icount_store(ctx->inode_count, ino, 2);
	ext2fs_icount_store(ctx->inode_link_info, ino, 2);
#if 0
	printf("/lost+found created; inode #%lu\n", ino);
#endif
	return ino;
}

/*
 * This routine will connect a file to lost+found
 */
int e2fsck_reconnect_file(e2fsck_t ctx, ext2_ino_t ino)
{
	ext2_filsys fs = ctx->fs;
	errcode_t	retval;
	char		name[80];
	struct problem_context	pctx;
	struct ext2_inode 	inode;
	int		file_type = 0;

	clear_problem_context(&pctx);
	pctx.ino = ino;

	if (!bad_lost_and_found && !lost_and_found) {
		lost_and_found = get_lost_and_found(ctx);
		if (!lost_and_found)
			bad_lost_and_found++;
	}
	if (bad_lost_and_found) {
		fix_problem(ctx, PR_3_NO_LPF, &pctx);
		return 1;
	}
	
	sprintf(name, "#%u", ino);
	if (ext2fs_read_inode(fs, ino, &inode) == 0)
		file_type = ext2_file_type(inode.i_mode);
	retval = ext2fs_link(fs, lost_and_found, name, ino, file_type);
	if (retval == EXT2_ET_DIR_NO_SPACE) {
		if (!fix_problem(ctx, PR_3_EXPAND_LF_DIR, &pctx))
			return 1;
		retval = expand_directory(ctx, lost_and_found);
		if (retval) {
			pctx.errcode = retval;
			fix_problem(ctx, PR_3_CANT_EXPAND_LPF, &pctx);
			return 1;
		}
		retval = ext2fs_link(fs, lost_and_found, name, ino, file_type);
	}
	if (retval) {
		pctx.errcode = retval;
		fix_problem(ctx, PR_3_CANT_RECONNECT, &pctx);
		return 1;
	}
	adjust_inode_count(ctx, ino, 1);

	return 0;
}

/*
 * Utility routine to adjust the inode counts on an inode.
 */
static errcode_t adjust_inode_count(e2fsck_t ctx, ext2_ino_t ino, int adj)
{
	ext2_filsys fs = ctx->fs;
	errcode_t		retval;
	struct ext2_inode 	inode;
	
	if (!ino)
		return 0;

	retval = ext2fs_read_inode(fs, ino, &inode);
	if (retval)
		return retval;

#if 0
	printf("Adjusting link count for inode %lu by %d (from %d)\n", ino, adj,
	       inode.i_links_count);
#endif

	if (adj == 1) {
		ext2fs_icount_increment(ctx->inode_count, ino, 0);
		if (inode.i_links_count == (__u16) ~0)
			return 0;
		ext2fs_icount_increment(ctx->inode_link_info, ino, 0);
		inode.i_links_count++;
	} else if (adj == -1) {
		ext2fs_icount_decrement(ctx->inode_count, ino, 0);
		if (inode.i_links_count == 0)
			return 0;
		ext2fs_icount_decrement(ctx->inode_link_info, ino, 0);
		inode.i_links_count--;
	} else {
		/* Should never happen */
		fatal_error(ctx, _("Debug error in e2fsck adjust_inode_count, "
				   "should never happen.\n"));
	}
	
	retval = ext2fs_write_inode(fs, ino, &inode);
	if (retval)
		return retval;

	return 0;
}

/*
 * Fix parent --- this routine fixes up the parent of a directory.
 */
struct fix_dotdot_struct {
	ext2_filsys	fs;
	ext2_ino_t	parent;
	int		done;
	e2fsck_t	ctx;
};

static int fix_dotdot_proc(struct ext2_dir_entry *dirent,
			   int	offset,
			   int	blocksize,
			   char	*buf,
			   void	*priv_data)
{
	struct fix_dotdot_struct *fp = (struct fix_dotdot_struct *) priv_data;
	errcode_t	retval;
	struct problem_context pctx;

	if ((dirent->name_len & 0xFF) != 2)
		return 0;
	if (strncmp(dirent->name, "..", 2))
		return 0;

	clear_problem_context(&pctx);
	
	retval = adjust_inode_count(fp->ctx, dirent->inode, -1);
	if (retval) {
		pctx.errcode = retval;
		fix_problem(fp->ctx, PR_3_ADJUST_INODE, &pctx);
	}
	retval = adjust_inode_count(fp->ctx, fp->parent, 1);
	if (retval) {
		pctx.errcode = retval;
		fix_problem(fp->ctx, PR_3_ADJUST_INODE, &pctx);
	}
	dirent->inode = fp->parent;

	fp->done++;
	return DIRENT_ABORT | DIRENT_CHANGED;
}

static void fix_dotdot(e2fsck_t ctx, struct dir_info *dir, ext2_ino_t parent)
{
	ext2_filsys fs = ctx->fs;
	errcode_t	retval;
	struct fix_dotdot_struct fp;
	struct problem_context pctx;

	fp.fs = fs;
	fp.parent = parent;
	fp.done = 0;
	fp.ctx = ctx;

#if 0
	printf("Fixing '..' of inode %lu to be %lu...\n", dir->ino, parent);
#endif
	
	retval = ext2fs_dir_iterate(fs, dir->ino, DIRENT_FLAG_INCLUDE_EMPTY,
				    0, fix_dotdot_proc, &fp);
	if (retval || !fp.done) {
		clear_problem_context(&pctx);
		pctx.ino = dir->ino;
		pctx.errcode = retval;
		fix_problem(ctx, retval ? PR_3_FIX_PARENT_ERR :
			    PR_3_FIX_PARENT_NOFIND, &pctx);
		ext2fs_unmark_valid(fs);
	}
	dir->dotdot = parent;
	
	return;
}

/*
 * These routines are responsible for expanding a /lost+found if it is
 * too small.
 */

struct expand_dir_struct {
	int			done;
	int			newblocks;
	errcode_t		err;
	e2fsck_t		ctx;
};

static int expand_dir_proc(ext2_filsys fs,
			   blk_t	*blocknr,
			   e2_blkcnt_t	blockcnt,
			   blk_t ref_block,
			   int ref_offset, 
			   void	*priv_data)
{
	struct expand_dir_struct *es = (struct expand_dir_struct *) priv_data;
	blk_t	new_blk;
	static blk_t	last_blk = 0;
	char		*block;
	errcode_t	retval;
	e2fsck_t	ctx;

	ctx = es->ctx;
	
	if (*blocknr) {
		last_blk = *blocknr;
		return 0;
	}
	retval = ext2fs_new_block(fs, last_blk, ctx->block_found_map,
				  &new_blk);
	if (retval) {
		es->err = retval;
		return BLOCK_ABORT;
	}
	if (blockcnt > 0) {
		retval = ext2fs_new_dir_block(fs, 0, 0, &block);
		if (retval) {
			es->err = retval;
			return BLOCK_ABORT;
		}
		es->done = 1;
		retval = ext2fs_write_dir_block(fs, new_blk, block);
	} else {
		retval = ext2fs_get_mem(fs->blocksize, (void **) &block);
		if (retval) {
			es->err = retval;
			return BLOCK_ABORT;
		}
		memset(block, 0, fs->blocksize);
		retval = io_channel_write_blk(fs->io, new_blk, 1, block);
	}	
	if (retval) {
		es->err = retval;
		return BLOCK_ABORT;
	}
	ext2fs_free_mem((void **) &block);
	*blocknr = new_blk;
	ext2fs_mark_block_bitmap(ctx->block_found_map, new_blk);
	ext2fs_mark_block_bitmap(fs->block_map, new_blk);
	ext2fs_mark_bb_dirty(fs);
	es->newblocks++;
	
	if (es->done)
		return (BLOCK_CHANGED | BLOCK_ABORT);
	else
		return BLOCK_CHANGED;
}

static errcode_t expand_directory(e2fsck_t ctx, ext2_ino_t dir)
{
	ext2_filsys fs = ctx->fs;
	errcode_t	retval;
	struct expand_dir_struct es;
	struct ext2_inode	inode;
	
	if (!(fs->flags & EXT2_FLAG_RW))
		return EXT2_ET_RO_FILSYS;

	/*
	 * Read the inode and block bitmaps in; we'll be messing with
	 * them.
	 */
	e2fsck_read_bitmaps(ctx);

	retval = ext2fs_check_directory(fs, dir);
	if (retval)
		return retval;
	
	es.done = 0;
	es.err = 0;
	es.newblocks = 0;
	es.ctx = ctx;
	
	retval = ext2fs_block_iterate2(fs, dir, BLOCK_FLAG_APPEND,
				       0, expand_dir_proc, &es);

	if (es.err)
		return es.err;
	if (!es.done)
		return EXT2_ET_EXPAND_DIR_ERR;

	/*
	 * Update the size and block count fields in the inode.
	 */
	retval = ext2fs_read_inode(fs, dir, &inode);
	if (retval)
		return retval;
	
	inode.i_size += fs->blocksize;
	inode.i_blocks += (fs->blocksize / 512) * es.newblocks;

	e2fsck_write_inode(ctx, dir, &inode, "expand_directory");

	return 0;
}
