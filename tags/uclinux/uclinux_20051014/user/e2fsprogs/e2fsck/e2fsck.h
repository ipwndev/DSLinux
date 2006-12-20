/*
 * e2fsck.h
 * 
 * Copyright (C) 1993, 1994 Theodore Ts'o.  This file may be
 * redistributed under the terms of the GNU Public License.
 * 
 */

#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <time.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SETJMP_H
#include <setjmp.h>
#endif

#if EXT2_FLAT_INCLUDES
#include "ext2_fs.h"
#include "ext2fs.h"
#else
#include "ext2fs/ext2_fs.h"
#include "ext2fs/ext2fs.h"
#endif

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(a) (gettext (a))
#ifdef gettext_noop
#define N_(a) gettext_noop (a)
#else
#define N_(a) (a)
#endif
/* FIXME */
#define NLS_CAT_NAME "e2fsprogs"
#define LOCALEDIR "/usr/share/locale"
/* FIXME */
#else
#define _(a) (a)
#define N_(a) a
#endif

/*
 * Exit codes used by fsck-type programs
 */
#define FSCK_OK          0	/* No errors */
#define FSCK_NONDESTRUCT 1	/* File system errors corrected */
#define FSCK_REBOOT      2	/* System should be rebooted */
#define FSCK_UNCORRECTED 4	/* File system errors left uncorrected */
#define FSCK_ERROR       8	/* Operational error */
#define FSCK_USAGE       16	/* Usage or syntax error */
#define FSCK_LIBRARY     128	/* Shared library error */

/*
 * The last ext2fs revision level that this version of e2fsck is able to
 * support
 */
#define E2FSCK_CURRENT_REV	1

/*
 * The directory information structure; stores directory information
 * collected in earlier passes, to avoid disk i/o in fetching the
 * directory information.
 */
struct dir_info {
	ext2_ino_t		ino;	/* Inode number */
	ext2_ino_t		dotdot;	/* Parent according to '..' */
	ext2_ino_t		parent; /* Parent according to treewalk */
};

#ifdef RESOURCE_TRACK
/*
 * This structure is used for keeping track of how much resources have
 * been used for a particular pass of e2fsck.
 */
struct resource_track {
	struct timeval time_start;
	struct timeval user_start;
	struct timeval system_start;
	void	*brk_start;
};
#endif

/*
 * E2fsck options
 */
#define E2F_OPT_READONLY	0x0001
#define E2F_OPT_PREEN		0x0002
#define E2F_OPT_YES		0x0004
#define E2F_OPT_NO		0x0008
#define E2F_OPT_TIME		0x0010
#define E2F_OPT_TIME2		0x0020
#define E2F_OPT_CHECKBLOCKS	0x0040
#define E2F_OPT_DEBUG		0x0080

/*
 * E2fsck flags
 */
#define E2F_FLAG_ABORT		0x0001 /* Abort signaled */
#define E2F_FLAG_CANCEL		0x0002 /* Cancel signaled */
#define E2F_FLAG_SIGNAL_MASK	0x0003
#define E2F_FLAG_RESTART	0x0004 /* Restart signaled */

#define E2F_FLAG_SETJMP_OK	0x0010 /* Setjmp valid for abort */

#define E2F_FLAG_PROG_BAR	0x0020 /* Progress bar on screen */
#define E2F_FLAG_PROG_SUPPRESS	0x0040 /* Progress suspended */
#define E2F_FLAG_JOURNAL_INODE	0x0080 /* Create a new ext3 journal inode */
#define E2F_FLAG_SB_SPECIFIED	0x0100 /* The superblock was explicitly 
					* specified by the user */

/*
 * Defines for indicating the e2fsck pass number
 */
#define E2F_PASS_1	1
#define E2F_PASS_2	2
#define E2F_PASS_3	3
#define E2F_PASS_4	4
#define E2F_PASS_5	5
#define E2F_PASS_1B	6

/*
 * Define the extended attribute refcount structure
 */
typedef struct ea_refcount *ext2_refcount_t;

/*
 * This is the global e2fsck structure.
 */
typedef struct e2fsck_struct *e2fsck_t;

struct e2fsck_struct {
	ext2_filsys fs;
	const char *program_name;
	const char *filesystem_name;
	const char *device_name;
	int	flags;		/* E2fsck internal flags */
	int	options;
	blk_t	use_superblock;	/* sb requested by user */
	blk_t	superblock;	/* sb used to open fs */
	blk_t	num_blocks;	/* Total number of blocks */

#ifdef HAVE_SETJMP_H
	jmp_buf	abort_loc;
#endif
	unsigned long abort_code;

	int (*progress)(e2fsck_t ctx, int pass, unsigned long cur,
			unsigned long max);

	ext2fs_inode_bitmap inode_used_map; /* Inodes which are in use */
	ext2fs_inode_bitmap inode_bad_map; /* Inodes which are bad somehow */
	ext2fs_inode_bitmap inode_dir_map; /* Inodes which are directories */
	ext2fs_inode_bitmap inode_bb_map; /* Inodes which are in bad blocks */
	ext2fs_inode_bitmap inode_imagic_map; /* AFS inodes */
	ext2fs_inode_bitmap inode_reg_map; /* Inodes which are regular files*/

	ext2fs_block_bitmap block_found_map; /* Blocks which are in use */
	ext2fs_block_bitmap block_dup_map; /* Blks referenced more than once */
	ext2fs_block_bitmap block_ea_map; /* Blocks which are used by EA's */

	/*
	 * Inode count arrays
	 */
	ext2_icount_t	inode_count;
	ext2_icount_t inode_link_info;

	ext2_refcount_t	refcount;
	ext2_refcount_t refcount_extra;

	/*
	 * Array of flags indicating whether an inode bitmap, block
	 * bitmap, or inode table is invalid
	 */
	int *invalid_inode_bitmap_flag;
	int *invalid_block_bitmap_flag;
	int *invalid_inode_table_flag;
	int invalid_bitmaps;	/* There are invalid bitmaps/itable */

	/*
	 * Block buffer
	 */
	char *block_buf;

	/*
	 * For pass1_check_directory and pass1_get_blocks
	 */
	ext2_ino_t stashed_ino;
	struct ext2_inode *stashed_inode;

	/*
	 * Directory information
	 */
	int		dir_info_count;
	int		dir_info_size;
	struct dir_info	*dir_info;

	/*
	 * Tuning parameters
	 */
	int process_inode_size;
	int inode_buffer_blocks;

	/*
	 * ext3 journal support
	 */
	io_channel	journal_io;
	const char	*journal_name;

#ifdef RESOURCE_TRACK
	/*
	 * For timing purposes
	 */
	struct resource_track	global_rtrack;
#endif

	/*
	 * How we display the progress update (for unix)
	 */
	int progress_fd;
	int progress_pos;
	int progress_last_percent;
	unsigned int progress_last_time;

	/* File counts */
	int fs_directory_count;
	int fs_regular_count;
	int fs_blockdev_count;
	int fs_chardev_count;
	int fs_links_count;
	int fs_symlinks_count;
	int fs_fast_symlinks_count;
	int fs_fifo_count;
	int fs_total_count;
	int fs_badblocks_count;
	int fs_sockets_count;
	int fs_ind_count;
	int fs_dind_count;
	int fs_tind_count;
	int fs_fragmented;
	int large_files;
	int fs_ext_attr_inodes;
	int fs_ext_attr_blocks;

	/*
	 * For the use of callers of the e2fsck functions; not used by
	 * e2fsck functions themselves.
	 */
	void *priv_data;
};

/* Used by the region allocation code */
typedef __u32 region_addr_t;
typedef struct region_struct *region_t;

/*
 * Procedure declarations
 */

extern void e2fsck_pass1(e2fsck_t ctx);
extern void e2fsck_pass1_dupblocks(e2fsck_t ctx, char *block_buf);
extern void e2fsck_pass2(e2fsck_t ctx);
extern void e2fsck_pass3(e2fsck_t ctx);
extern void e2fsck_pass4(e2fsck_t ctx);
extern void e2fsck_pass5(e2fsck_t ctx);

/* e2fsck.c */
extern errcode_t e2fsck_allocate_context(e2fsck_t *ret);
extern errcode_t e2fsck_reset_context(e2fsck_t ctx);
extern void e2fsck_free_context(e2fsck_t ctx);
extern int e2fsck_run(e2fsck_t ctx);


/* badblock.c */
extern void read_bad_blocks_file(e2fsck_t ctx, const char *bad_blocks_file,
				 int replace_bad_blocks);
extern void test_disk(e2fsck_t ctx);

/* dirinfo.c */
extern void e2fsck_add_dir_info(e2fsck_t ctx, ext2_ino_t ino, ext2_ino_t parent);
extern struct dir_info *e2fsck_get_dir_info(e2fsck_t ctx, ext2_ino_t ino);
extern void e2fsck_free_dir_info(e2fsck_t ctx);
extern int e2fsck_get_num_dirs(e2fsck_t ctx);
extern int e2fsck_get_num_dirinfo(e2fsck_t ctx);
extern struct dir_info *e2fsck_dir_info_iter(e2fsck_t ctx, int *control);

/* ea_refcount.c */
extern errcode_t ea_refcount_create(int size, ext2_refcount_t *ret);
extern void ea_refcount_free(ext2_refcount_t refcount);
extern errcode_t ea_refcount_fetch(ext2_refcount_t refcount, blk_t blk,
				   int *ret);
extern errcode_t ea_refcount_increment(ext2_refcount_t refcount,
				       blk_t blk, int *ret);
extern errcode_t ea_refcount_decrement(ext2_refcount_t refcount,
				       blk_t blk, int *ret);
extern errcode_t ea_refcount_store(ext2_refcount_t refcount,
				   blk_t blk, int count);
extern void ea_refcount_intr_begin(ext2_refcount_t refcount);
extern blk_t ea_refcount_intr_next(ext2_refcount_t refcount, int *ret);

/* ehandler.c */
extern const char *ehandler_operation(const char *op);
extern void ehandler_init(io_channel channel);

/* journal.c */
extern int e2fsck_check_ext3_journal(e2fsck_t ctx);
extern int e2fsck_run_ext3_journal(e2fsck_t ctx);

/* pass1.c */
extern void e2fsck_use_inode_shortcuts(e2fsck_t ctx, int bool);
extern int e2fsck_pass1_check_device_inode(struct ext2_inode *inode);
extern int e2fsck_pass1_check_symlink(ext2_filsys fs, struct ext2_inode *inode);

/* pass2.c */
extern int e2fsck_process_bad_inode(e2fsck_t ctx, ext2_ino_t dir, ext2_ino_t ino);

/* pass3.c */
extern int e2fsck_reconnect_file(e2fsck_t ctx, ext2_ino_t inode);

/* region.c */
extern region_t region_create(region_addr_t min, region_addr_t max);
extern void region_free(region_t region);
extern int region_allocate(region_t region, region_addr_t start, int n);

/* super.c */
void check_super_block(e2fsck_t ctx);
errcode_t e2fsck_get_device_size(e2fsck_t ctx);

/* swapfs.c */
void swap_filesys(e2fsck_t ctx);

/* util.c */
extern void *e2fsck_allocate_memory(e2fsck_t ctx, unsigned int size,
				    const char *description);
extern int ask(e2fsck_t ctx, const char * string, int def);
extern int ask_yn(const char * string, int def);
extern void fatal_error(e2fsck_t ctx, const char * fmt_string);
extern void e2fsck_read_bitmaps(e2fsck_t ctx);
extern void e2fsck_write_bitmaps(e2fsck_t ctx);
extern void preenhalt(e2fsck_t ctx);
#ifdef RESOURCE_TRACK
extern void print_resource_track(const char *desc,
				 struct resource_track *track);
extern void init_resource_track(struct resource_track *track);
#endif
extern int inode_has_valid_blocks(struct ext2_inode *inode);
extern void e2fsck_read_inode(e2fsck_t ctx, unsigned long ino,
			      struct ext2_inode * inode, const char * proc);
extern void e2fsck_write_inode(e2fsck_t ctx, unsigned long ino,
			       struct ext2_inode * inode, const char * proc);
#ifdef MTRACE
extern void mtrace_print(char *mesg);
#endif
extern blk_t get_backup_sb(ext2_filsys fs);
extern int ext2_file_type(unsigned int mode);

/* unix.c */
extern void e2fsck_clear_progbar(e2fsck_t ctx);
