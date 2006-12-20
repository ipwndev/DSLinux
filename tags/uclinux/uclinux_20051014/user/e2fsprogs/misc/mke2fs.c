/*
 * mke2fs.c - Make a ext2fs filesystem.
 * 
 * Copyright (C) 1994, 1995, 1996, 1997 Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

/* Usage: mke2fs [options] device
 * 
 * The device may be a block device or a image of one, but this isn't
 * enforced (but it's not much fun on a character device :-). 
 */

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#ifdef linux
#include <sys/utsname.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
extern char *optarg;
extern int optind;
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_MNTENT_H
#include <mntent.h>
#endif
#include <sys/ioctl.h>
#include <sys/types.h>

#include "ext2fs/ext2_fs.h"
#include "et/com_err.h"
#include "uuid/uuid.h"
#include "e2p/e2p.h"
#include "ext2fs/ext2fs.h"
#include "util.h"
#include "../version.h"
#include "nls-enable.h"

#define STRIDE_LENGTH 8

#ifndef __sparc__
#define ZAP_BOOTBLOCK
#endif

extern int isatty(int);
extern FILE *fpopen(const char *cmd, const char *mode);

const char * program_name = "mke2fs";
const char * device_name /* = NULL */;

/* Command line options */
int	cflag;
int	verbose;
int	quiet;
int	super_only;
int	force;
int	noaction;
int	journal_size;
int	journal_flags;
char	*bad_blocks_filename;
__u32	fs_stride;

struct ext2_super_block param;
char *creator_os;
char *volume_label;
char *mount_dir;
char *journal_device;
int sync_kludge;	/* Set using the MKE2FS_SYNC env. option */

static void usage(void)
{
	fprintf(stderr, _("Usage: %s [-c|-t|-l filename] [-b block-size] "
	"[-f fragment-size]\n\t[-i bytes-per-inode] [-j] [-J journal-options]"
	" [-N number-of-inodes]\n\t[-m reserved-blocks-percentage] "
	"[-o creator-os] [-g blocks-per-group]\n\t[-L volume-label] "
	"[-M last-mounted-directory] [-O feature[,...]]\n\t"
	"[-r fs-revision] [-R raid_opts] [-qvSV] device [blocks-count]\n"),
		program_name);
	exit(1);
}

static int int_log2(int arg)
{
	int	l = 0;

	arg >>= 1;
	while (arg) {
		l++;
		arg >>= 1;
	}
	return l;
}

static int int_log10(unsigned int arg)
{
	int	l;

	for (l=0; arg ; l++)
		arg = arg / 10;
	return l;
}

/*
 * This function sets the default parameters for a filesystem
 *
 * The type is specified by the user.  The size is the maximum size
 * (in megabytes) for which a set of parameters applies, with a size
 * of zero meaning that it is the default parameter for the type.
 * Note that order is important in the table below.
 */
static char default_str[] = "default";
struct mke2fs_defaults {
	const char	*type;
	int		size;
	int		blocksize;
	int		inode_ratio;
} settings[] = {
	{ default_str, 0, 4096, 8192 },
	{ default_str, 512, 1024, 4096 },
	{ default_str, 3, 1024, 8192 },
	{ "journal", 0, 4096, 8192 },
	{ "news", 0, 4096, 4096 },
	{ "largefile", 0, 4096, 1024 * 1024 },
	{ "largefile4", 0, 4096, 4096 * 1024 },
	{ 0, 0, 0, 0},
};

static void set_fs_defaults(char *fs_type, struct ext2_super_block *super,
			    int blocksize, int *inode_ratio)
{
	int	megs;
	int	ratio = 0;
	struct mke2fs_defaults *p;

	megs = (super->s_blocks_count * (EXT2_BLOCK_SIZE(super) / 1024) /
		1024);
	if (inode_ratio)
		ratio = *inode_ratio;
	if (!fs_type)
		fs_type = default_str;
	for (p = settings; p->type; p++) {
		if ((strcmp(p->type, fs_type) != 0) &&
		    (strcmp(p->type, default_str) != 0))
			continue;
		if ((p->size != 0) &&
		    (megs > p->size))
			continue;
		if (ratio == 0)
			*inode_ratio = p->inode_ratio;
		if (blocksize == 0) {
			super->s_log_frag_size = super->s_log_block_size =
				int_log2(p->blocksize >> EXT2_MIN_BLOCK_LOG_SIZE);
		}
	}
	if (blocksize == 0)
		super->s_blocks_count /= EXT2_BLOCK_SIZE(super) / 1024;
}

/*
 * Helper function for read_bb_file and test_disk
 */
static void invalid_block(ext2_filsys fs, blk_t blk)
{
	printf(_("Bad block %u out of range; ignored.\n"), blk);
	return;
}

/*
 * Reads the bad blocks list from a file
 */
static void read_bb_file(ext2_filsys fs, badblocks_list *bb_list,
			 const char *bad_blocks_file)
{
	FILE		*f;
	errcode_t	retval;

	f = fopen(bad_blocks_file, "r");
	if (!f) {
		com_err("read_bad_blocks_file", errno,
			_("while trying to open %s"), bad_blocks_file);
		exit(1);
	}
	retval = ext2fs_read_bb_FILE(fs, f, bb_list, invalid_block);
	fclose (f);
	if (retval) {
		com_err("ext2fs_read_bb_FILE", retval,
			_("while reading in list of bad blocks from file"));
		exit(1);
	}
}

/*
 * Runs the badblocks program to test the disk
 */
static void test_disk(ext2_filsys fs, badblocks_list *bb_list)
{
	FILE		*f;
	errcode_t	retval;
	char		buf[1024];

	sprintf(buf, "badblocks -b %d %s%s %d", fs->blocksize,
		quiet ? "" : "-s ", fs->device_name,
		fs->super->s_blocks_count);
	if (verbose)
		printf(_("Running command: %s\n"), buf);
	f = popen(buf, "r");
	if (!f) {
		com_err("popen", errno,
			_("while trying run '%s'"), buf);
		exit(1);
	}
	retval = ext2fs_read_bb_FILE(fs, f, bb_list, invalid_block);
	pclose(f);
	if (retval) {
		com_err("ext2fs_read_bb_FILE", retval,
			_("while processing list of bad blocks from program"));
		exit(1);
	}
}

static void handle_bad_blocks(ext2_filsys fs, badblocks_list bb_list)
{
	int			i, j;
	int			must_be_good;
	blk_t			blk;
	badblocks_iterate	bb_iter;
	errcode_t		retval;
	blk_t			group_block;
	int			group;
	int			group_bad;

	if (!bb_list)
		return;
	
	/*
	 * The primary superblock and group descriptors *must* be
	 * good; if not, abort.
	 */
	must_be_good = fs->super->s_first_data_block + 1 + fs->desc_blocks;
	for (i = fs->super->s_first_data_block; i <= must_be_good; i++) {
		if (ext2fs_badblocks_list_test(bb_list, i)) {
			fprintf(stderr, _("Block %d in primary "
				"superblock/group descriptor area bad.\n"), i);
			fprintf(stderr, _("Blocks %d through %d must be good "
				"in order to build a filesystem.\n"),
				fs->super->s_first_data_block, must_be_good);
			fprintf(stderr, _("Aborting....\n"));
			exit(1);
		}
	}

	/*
	 * See if any of the bad blocks are showing up in the backup
	 * superblocks and/or group descriptors.  If so, issue a
	 * warning and adjust the block counts appropriately.
	 */
	group_block = fs->super->s_first_data_block +
		fs->super->s_blocks_per_group;
	
	for (i = 1; i < fs->group_desc_count; i++) {
		group_bad = 0;
		for (j=0; j < fs->desc_blocks+1; j++) {
			if (ext2fs_badblocks_list_test(bb_list,
						       group_block + j)) {
				if (!group_bad) 
					fprintf(stderr,
_("Warning: the backup superblock/group descriptors at block %d contain\n"
"	bad blocks.\n\n"),
						group_block);
				group_bad++;
				group = ext2fs_group_of_blk(fs, group_block+j);
				fs->group_desc[group].bg_free_blocks_count++;
				fs->super->s_free_blocks_count++;
			}
		}
		group_block += fs->super->s_blocks_per_group;
	}
	
	/*
	 * Mark all the bad blocks as used...
	 */
	retval = ext2fs_badblocks_list_iterate_begin(bb_list, &bb_iter);
	if (retval) {
		com_err("ext2fs_badblocks_list_iterate_begin", retval,
			_("while marking bad blocks as used"));
		exit(1);
	}
	while (ext2fs_badblocks_list_iterate(bb_iter, &blk)) 
		ext2fs_mark_block_bitmap(fs->block_map, blk);
	ext2fs_badblocks_list_iterate_end(bb_iter);
}

/*
 * These functions implement a generalized progress meter.
 */
struct progress_struct {
	char		format[20];
	char		backup[80];
	__u32		max;
};

static void progress_init(struct progress_struct *progress,
			  char *label,__u32 max)
{
	int	i;

	memset(progress, 0, sizeof(struct progress_struct));
	if (quiet)
		return;

	/*
	 * Figure out how many digits we need
	 */
	i = int_log10(max);
	sprintf(progress->format, "%%%dd/%%%dld", i, i);
	memset(progress->backup, '\b', sizeof(progress->backup)-1);
	progress->backup[sizeof(progress->backup)-1] = 0;
	if ((2*i)+1 < sizeof(progress->backup))
		progress->backup[(2*i)+1] = 0;
	progress->max = max;

	fputs(label, stdout);
	fflush(stdout);
}

static void progress_update(struct progress_struct *progress, __u32 val)
{
	if (progress->format[0] == 0)
		return;
	printf(progress->format, val, progress->max);
	fputs(progress->backup, stdout);
}

static void progress_close(struct progress_struct *progress)
{
	if (progress->format[0] == 0)
		return;
	fputs(_("done                            \n"), stdout);
}


/*
 * Helper function which zeros out _num_ blocks starting at _blk_.  In
 * case of an error, the details of the error is returned via _ret_blk_
 * and _ret_count_ if they are non-NULL pointers.  Returns 0 on
 * success, and an error code on an error.
 *
 * As a special case, if the first argument is NULL, then it will
 * attempt to free the static zeroizing buffer.  (This is to keep
 * programs that check for memory leaks happy.)
 */
static errcode_t zero_blocks(ext2_filsys fs, blk_t blk, int num,
			     struct progress_struct *progress,
			     blk_t *ret_blk, int *ret_count)
{
	int		j, count, next_update, next_update_incr;
	static char	*buf;
	errcode_t	retval;

	/* If fs is null, clean up the static buffer and return */
	if (!fs) {
		if (buf) {
			free(buf);
			buf = 0;
		}
		return 0;
	}
	/* Allocate the zeroizing buffer if necessary */
	if (!buf) {
		buf = malloc(fs->blocksize * STRIDE_LENGTH);
		if (!buf) {
			com_err("malloc", ENOMEM,
				_("while allocating zeroizing buffer"));
			exit(1);
		}
		memset(buf, 0, fs->blocksize * STRIDE_LENGTH);
	}
	/* OK, do the write loop */
	next_update = 0;
	next_update_incr = num / 100;
	if (next_update_incr < 1)
		next_update_incr = 1;
	for (j=0; j < num; j += STRIDE_LENGTH, blk += STRIDE_LENGTH) {
		if (num-j > STRIDE_LENGTH)
			count = STRIDE_LENGTH;
		else
			count = num - j;
		retval = io_channel_write_blk(fs->io, blk, count, buf);
		if (retval) {
			if (ret_count)
				*ret_count = count;
			if (ret_blk)
				*ret_blk = blk;
			return retval;
		}
		if (progress && j > next_update) {
			next_update += num / 100;
			progress_update(progress, blk);
		}
	}
	return 0;
}	

static void write_inode_tables(ext2_filsys fs)
{
	errcode_t	retval;
	blk_t		blk;
	int		i, num;
	struct progress_struct progress;

	if (quiet)
		memset(&progress, 0, sizeof(progress));
	else
		progress_init(&progress, _("Writing inode tables: "),
			      fs->group_desc_count);

	for (i = 0; i < fs->group_desc_count; i++) {
		progress_update(&progress, i);
		
		blk = fs->group_desc[i].bg_inode_table;
		num = fs->inode_blocks_per_group;

		retval = zero_blocks(fs, blk, num, 0, &blk, &num);
		if (retval) {
			printf(_("\nCould not write %d blocks "
				 "in inode table starting at %d: %s\n"),
			       num, blk, error_message(retval));
			exit(1);
		}
		if (sync_kludge) {
			if (sync_kludge == 1)
				sync();
			else if ((i % sync_kludge) == 0)
				sync();
		}
	}
	zero_blocks(0, 0, 0, 0, 0, 0);
	progress_close(&progress);
}

static void create_root_dir(ext2_filsys fs)
{
	errcode_t	retval;
	struct ext2_inode	inode;

	retval = ext2fs_mkdir(fs, EXT2_ROOT_INO, EXT2_ROOT_INO, 0);
	if (retval) {
		com_err("ext2fs_mkdir", retval, _("while creating root dir"));
		exit(1);
	}
	if (geteuid()) {
		retval = ext2fs_read_inode(fs, EXT2_ROOT_INO, &inode);
		if (retval) {
			com_err("ext2fs_read_inode", retval,
				_("while reading root inode"));
			exit(1);
		}
		inode.i_uid = getuid();
		if (inode.i_uid)
			inode.i_gid = getgid();
		retval = ext2fs_write_inode(fs, EXT2_ROOT_INO, &inode);
		if (retval) {
			com_err("ext2fs_write_inode", retval,
				_("while setting root inode ownership"));
			exit(1);
		}
	}
}

static void create_lost_and_found(ext2_filsys fs)
{
	errcode_t		retval;
	ext2_ino_t		ino;
	const char		*name = "lost+found";
	int			i;
	int			lpf_size = 0;

	retval = ext2fs_mkdir(fs, EXT2_ROOT_INO, 0, name);
	if (retval) {
		com_err("ext2fs_mkdir", retval,
			_("while creating /lost+found"));
		exit(1);
	}

	retval = ext2fs_lookup(fs, EXT2_ROOT_INO, name, strlen(name), 0, &ino);
	if (retval) {
		com_err("ext2_lookup", retval,
			_("while looking up /lost+found"));
		exit(1);
	}
	
	for (i=1; i < EXT2_NDIR_BLOCKS; i++) {
		if ((lpf_size += fs->blocksize) >= 16*1024)
			break;
		retval = ext2fs_expand_dir(fs, ino);
		if (retval) {
			com_err("ext2fs_expand_dir", retval,
				_("while expanding /lost+found"));
			exit(1);
		}
	}		
}

static void create_bad_block_inode(ext2_filsys fs, badblocks_list bb_list)
{
	errcode_t	retval;
	
	ext2fs_mark_inode_bitmap(fs->inode_map, EXT2_BAD_INO);
	fs->group_desc[0].bg_free_inodes_count--;
	fs->super->s_free_inodes_count--;
	retval = ext2fs_update_bb_inode(fs, bb_list);
	if (retval) {
		com_err("ext2fs_update_bb_inode", retval,
			_("while setting bad block inode"));
		exit(1);
	}

}

static void reserve_inodes(ext2_filsys fs)
{
	ext2_ino_t	i;
	int		group;

	for (i = EXT2_ROOT_INO + 1; i < EXT2_FIRST_INODE(fs->super); i++) {
		ext2fs_mark_inode_bitmap(fs->inode_map, i);
		group = ext2fs_group_of_ino(fs, i);
		fs->group_desc[group].bg_free_inodes_count--;
		fs->super->s_free_inodes_count--;
	}
	ext2fs_mark_ib_dirty(fs);
}

static void zap_sector(ext2_filsys fs, int sect, int nsect)
{
	char *buf;
	int retval;

	buf = malloc(512*nsect);
	memset(buf, 0, 512*nsect);
	
	io_channel_set_blksize(fs->io, 512);
	retval = io_channel_write_blk(fs->io, sect, -512*nsect, buf);
	io_channel_set_blksize(fs->io, fs->blocksize);
	free(buf);
	if (retval)
		printf(_("Warning: could not erase sector %d: %s\n"), sect,
		       error_message(retval));
}

static void create_journal_dev(ext2_filsys fs)
{
	struct progress_struct progress;
	errcode_t		retval;
	char			*buf;
	blk_t			blk;
	int			count;

	retval = ext2fs_create_journal_superblock(fs,
				  fs->super->s_blocks_count, 0, &buf);
	if (retval) {
		com_err("create_journal_dev", retval,
			_("while initializing journal superblock"));
		exit(1);
	}
	if (quiet)
		memset(&progress, 0, sizeof(progress));
	else
		progress_init(&progress, _("Zeroing journal device: "),
			      fs->super->s_blocks_count);

	retval = zero_blocks(fs, 0, fs->super->s_blocks_count,
			     &progress, &blk, &count);
	if (retval) {
		com_err("create_journal_dev", retval,
			_("while zeroing journal device (block %u, count %d)"),
			blk, count);
		exit(1);
	}
	zero_blocks(0, 0, 0, 0, 0, 0);

	retval = io_channel_write_blk(fs->io,
				      fs->super->s_first_data_block+1,
				      1, buf);
	if (retval) {
		com_err("create_journal_dev", retval,
			_("while writing journal superblock"));
		exit(1);
	}
	progress_close(&progress);
}

static void show_stats(ext2_filsys fs)
{
	struct ext2_super_block *s = fs->super;
	char 			buf[80];
	blk_t			group_block;
	int			i, need, col_left;
	
	if (param.s_blocks_count != s->s_blocks_count)
		printf(_("warning: %d blocks unused.\n\n"),
		       param.s_blocks_count - s->s_blocks_count);

	memset(buf, 0, sizeof(buf));
	strncpy(buf, s->s_volume_name, sizeof(s->s_volume_name));
	printf(_("Filesystem label=%s\n"), buf);
	printf(_("OS type: "));
	switch (fs->super->s_creator_os) {
	    case EXT2_OS_LINUX: printf ("Linux"); break;
	    case EXT2_OS_HURD:  printf ("GNU/Hurd");   break;
	    case EXT2_OS_MASIX: printf ("Masix"); break;
	    default:		printf (_("(unknown os)"));
        }
	printf("\n");
	printf(_("Block size=%u (log=%u)\n"), fs->blocksize,
		s->s_log_block_size);
	printf(_("Fragment size=%u (log=%u)\n"), fs->fragsize,
		s->s_log_frag_size);
	printf(_("%u inodes, %u blocks\n"), s->s_inodes_count,
	       s->s_blocks_count);
#ifdef EMBED
	printf(_("%u blocks (%d%%) reserved for the super user\n"),
		s->s_r_blocks_count,
	       100 * s->s_r_blocks_count / s->s_blocks_count);
#else
	printf(_("%u blocks (%2.2f%%) reserved for the super user\n"),
		s->s_r_blocks_count,
	       100.0 * s->s_r_blocks_count / s->s_blocks_count);
#endif
	printf(_("First data block=%u\n"), s->s_first_data_block);
	if (fs->group_desc_count > 1)
		printf(_("%u block groups\n"), fs->group_desc_count);
	else
		printf(_("%u block group\n"), fs->group_desc_count);
	printf(_("%u blocks per group, %u fragments per group\n"),
	       s->s_blocks_per_group, s->s_frags_per_group);
	printf(_("%u inodes per group\n"), s->s_inodes_per_group);

	if (fs->group_desc_count == 1) {
		printf("\n");
		return;
	}
	
	printf(_("Superblock backups stored on blocks: "));
	group_block = s->s_first_data_block;
	col_left = 0;
	for (i = 1; i < fs->group_desc_count; i++) {
		group_block += s->s_blocks_per_group;
		if (!ext2fs_bg_has_super(fs, i))
			continue;
		if (i != 1)
			printf(", ");
		need = int_log10(group_block) + 2;
		if (need > col_left) {
			printf("\n\t");
			col_left = 72;
		}
		col_left -= need;
		printf("%u", group_block);
	}
	printf("\n\n");
}

/*
 * Set the S_CREATOR_OS field.  Return true if OS is known,
 * otherwise, 0.
 */
static int set_os(struct ext2_super_block *sb, char *os)
{
	if (isdigit (*os))
		sb->s_creator_os = atoi (os);
	else if (strcasecmp(os, "linux") == 0)
		sb->s_creator_os = EXT2_OS_LINUX;
	else if (strcasecmp(os, "GNU") == 0 || strcasecmp(os, "hurd") == 0)
		sb->s_creator_os = EXT2_OS_HURD;
	else if (strcasecmp(os, "masix") == 0)
		sb->s_creator_os = EXT2_OS_MASIX;
	else
		return 0;
	return 1;
}

#define PATH_SET "PATH=/sbin"

static void parse_raid_opts(const char *opts)
{
	char	*buf, *token, *next, *p, *arg;
	int	len;
	int	raid_usage = 0;

	len = strlen(opts);
	buf = malloc(len+1);
	if (!buf) {
		fprintf(stderr, _("Couldn't allocate memory to parse "
			"raid options!\n"));
		exit(1);
	}
	strcpy(buf, opts);
	for (token = buf; token && *token; token = next) {
		p = strchr(token, ',');
		next = 0;
		if (p) {
			*p = 0;
			next = p+1;
		} 
		arg = strchr(token, '=');
		if (arg) {
			*arg = 0;
			arg++;
		}
		if (strcmp(token, "stride") == 0) {
			if (!arg) {
				raid_usage++;
				continue;
			}
			fs_stride = strtoul(arg, &p, 0);
			if (*p || (fs_stride == 0)) {
				fprintf(stderr,
					_("Invalid stride parameter.\n"));
				raid_usage++;
				continue;
			}
		} else
			raid_usage++;
	}
	if (raid_usage) {
		fprintf(stderr, _("\nBad raid options specified.\n\n"
			"Raid options are separated by commas, "
			"and may take an argument which\n"
			"\tis set off by an equals ('=') sign.\n\n"
			"Valid raid options are:\n"
			"\tstride=<stride length in blocks>\n\n"));
		exit(1);
	}
}	

static __u32 ok_features[3] = {
	EXT3_FEATURE_COMPAT_HAS_JOURNAL,	/* Compat */
	EXT2_FEATURE_INCOMPAT_FILETYPE|		/* Incompat */
		EXT3_FEATURE_INCOMPAT_JOURNAL_DEV,
	EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER	/* R/O compat */
};


static void PRS(int argc, char *argv[])
{
	int		c;
	int		size;
	char *		tmp;
	blk_t		group_blk_max = 8192;
	int		blocksize = 0;
	int		inode_ratio = 0;
	int		reserved_ratio = 5;
	ext2_ino_t	num_inodes = 0;
	errcode_t	retval;
	char *		oldpath = getenv("PATH");
	char *		raid_opts = 0;
	char *		fs_type = 0;
	int		default_features = 1;
	blk_t		dev_size;
#ifdef linux
	struct 		utsname ut;
#endif

	/* Update our PATH to include /sbin  */
	if (oldpath) {
		char *newpath;
		
		newpath = malloc(sizeof (PATH_SET) + 1 + strlen (oldpath));
		strcpy (newpath, PATH_SET);
		strcat (newpath, ":");
		strcat (newpath, oldpath);
		putenv (newpath);
	} else
		putenv (PATH_SET);

	tmp = getenv("MKE2FS_SYNC");
	if (tmp)
		sync_kludge = atoi(tmp);
	
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	initialize_ext2_error_table();
	memset(&param, 0, sizeof(struct ext2_super_block));
	param.s_rev_level = 1;  /* Create revision 1 filesystems now */
	param.s_feature_incompat |= EXT2_FEATURE_INCOMPAT_FILETYPE;
	param.s_feature_ro_compat |= EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER;
		
#ifdef linux
	if (uname(&ut)) {
		perror("uname");
		exit(1);
	}
	if ((ut.release[0] == '1') ||
	    (ut.release[0] == '2' && ut.release[1] == '.' &&
	     ut.release[2] < '2' && ut.release[3] == '.')) {
		param.s_rev_level = 0;
		param.s_feature_incompat = 0;
		param.s_feature_compat = 0;
		param.s_feature_ro_compat = 0;
	}
#endif
	fprintf (stderr, "mke2fs %s (%s)\n",
		 E2FSPROGS_VERSION, E2FSPROGS_DATE);
	if (argc && *argv)
		program_name = *argv;
	while ((c = getopt (argc, argv,
		    "b:cf:g:i:jl:m:no:qr:R:s:tvI:J:ST:FL:M:N:O:V")) != EOF)
		switch (c) {
		case 'b':
			blocksize = strtoul(optarg, &tmp, 0);
			if (blocksize < 1024 || blocksize > 4096 || *tmp) {
				com_err(program_name, 0,
					_("bad block size - %s"), optarg);
				exit(1);
			}
			param.s_log_block_size =
				int_log2(blocksize >> EXT2_MIN_BLOCK_LOG_SIZE);
			group_blk_max = blocksize * 8;
			break;
		case 'c':	/* Check for bad blocks */
		case 't':	/* deprecated */
			cflag = 1;
			break;
		case 'f':
			size = strtoul(optarg, &tmp, 0);
			if (size < 1024 || size > 4096 || *tmp) {
				com_err(program_name, 0,
					_("bad fragment size - %s"),
					optarg);
				exit(1);
			}
			param.s_log_frag_size =
				int_log2(size >> EXT2_MIN_BLOCK_LOG_SIZE);
			printf(_("Warning: fragments not supported.  "
			       "Ignoring -f option\n"));
			break;
		case 'g':
			param.s_blocks_per_group = strtoul(optarg, &tmp, 0);
			if (*tmp) {
				com_err(program_name, 0,
					_("Illegal number for blocks per group"));
				exit(1);
			}
			if ((param.s_blocks_per_group % 8) != 0) {
				com_err(program_name, 0,
				_("blocks per group must be multiple of 8"));
				exit(1);
			}
			break;
		case 'i':
			inode_ratio = strtoul(optarg, &tmp, 0);
			if (inode_ratio < 1024 || inode_ratio > 4096 * 1024 ||
			    *tmp) {
				com_err(program_name, 0,
					_("bad inode ratio - %s"), optarg);
				exit(1);
			}
			break;
		case 'J':
			parse_journal_opts(optarg);
			break;
		case 'j':
			param.s_feature_compat |=
				EXT3_FEATURE_COMPAT_HAS_JOURNAL;
			if (!journal_size)
				journal_size = -1;
			break;
		case 'l':
			bad_blocks_filename = malloc(strlen(optarg)+1);
			if (!bad_blocks_filename) {
				com_err(program_name, ENOMEM,
					_("in malloc for bad_blocks_filename"));
				exit(1);
			}
			strcpy(bad_blocks_filename, optarg);
			break;
		case 'm':
			reserved_ratio = strtoul(optarg, &tmp, 0);
			if (reserved_ratio > 50 || *tmp) {
				com_err(program_name, 0,
					_("bad reserved blocks percent - %s"),
					optarg);
				exit(1);
			}
			break;
		case 'n':
			noaction++;
			break;
		case 'o':
			creator_os = optarg;
			break;
		case 'r':
			param.s_rev_level = atoi(optarg);
			if (param.s_rev_level == EXT2_GOOD_OLD_REV) {
				param.s_feature_incompat = 0;
				param.s_feature_compat = 0;
				param.s_feature_ro_compat = 0;
			}
			break;
		case 's':	/* deprecated */
			if (atoi(optarg))
				param.s_feature_ro_compat |=
					EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER;
			else 
				param.s_feature_ro_compat &=
					~EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER;
			break;
#ifdef EXT2_DYNAMIC_REV
		case 'I':
			param.s_inode_size = atoi(optarg);
			break;
#endif
		case 'N':
			num_inodes = atoi(optarg);
			break;
		case 'v':
			verbose = 1;
			break;
		case 'q':
			quiet = 1;
			break;
		case 'F':
			force = 1;
			break;
		case 'L':
			volume_label = optarg;
			break;
		case 'M':
			mount_dir = optarg;
			break;
		case 'O':
			if (!strcmp(optarg, "none") || default_features) {
				param.s_feature_compat = 0;
				param.s_feature_incompat = 0;
				param.s_feature_ro_compat = 0;
				default_features = 0;
			}
			if (!strcmp(optarg, "none"))
				break;
			if (e2p_edit_feature(optarg,
					    &param.s_feature_compat,
					    ok_features)) {
				fprintf(stderr,
					_("Invalid filesystem option set: %s\n"), optarg);
				exit(1);
			}
			break;
		case 'R':
			raid_opts = optarg;
			break;
		case 'S':
			super_only = 1;
			break;
		case 'T':
			fs_type = optarg;
			break;
		case 'V':
			/* Print version number and exit */
			fprintf(stderr, _("\tUsing %s\n"),
				error_message(EXT2_ET_BASE));
			exit(0);
		default:
			usage();
		}
	if (optind == argc)
		usage();
	device_name = argv[optind];
	optind++;
	if (optind < argc) {
		unsigned long tmp2  = strtoul(argv[optind++], &tmp, 0);

		if ((*tmp) || (tmp2 > 0xfffffffful)) {
			com_err(program_name, 0, _("bad blocks count - %s"),
				argv[optind - 1]);
			exit(1);
		}
		param.s_blocks_count = tmp2;
	}
	if (optind < argc)
		usage();

	if (raid_opts)
		parse_raid_opts(raid_opts);

	/*
	 * If there's no blocksize specified and there is a journal
	 * device, use it to figure out the blocksize
	 */
	if (blocksize == 0 && journal_device) {
		ext2_filsys	jfs;

		retval = ext2fs_open(journal_device,
				     EXT2_FLAG_JOURNAL_DEV_OK, 0,
				     0, unix_io_manager, &jfs);
		if (retval) {
			com_err(program_name, retval,
				_("while trying to open journal device %s\n"),
				journal_device);
			exit(1);
		}
		blocksize = jfs->blocksize;
		param.s_log_block_size =
			int_log2(blocksize >> EXT2_MIN_BLOCK_LOG_SIZE);
		ext2fs_close(jfs);
	}

	if (param.s_feature_incompat & EXT3_FEATURE_INCOMPAT_JOURNAL_DEV) {
		if (!fs_type)
			fs_type = "journal";
		reserved_ratio = 0;
		param.s_feature_incompat = EXT3_FEATURE_INCOMPAT_JOURNAL_DEV;
		param.s_feature_compat = 0;
		param.s_feature_ro_compat = 0;
 	}
	if (param.s_rev_level == EXT2_GOOD_OLD_REV &&
	    (param.s_feature_compat || param.s_feature_ro_compat ||
	     param.s_feature_incompat))
		param.s_rev_level = 1;  /* Create a revision 1 filesystem */

	if (!force)
		check_plausibility(device_name);
	check_mount(device_name, force, _("filesystem"));

	param.s_log_frag_size = param.s_log_block_size;

	if (noaction && param.s_blocks_count) {
		dev_size = param.s_blocks_count;
		retval = 0;
	} else
		retval = ext2fs_get_device_size(device_name,
						EXT2_BLOCK_SIZE(&param),
						&dev_size);
	if (retval && (retval != EXT2_ET_UNIMPLEMENTED)) {
		com_err(program_name, retval,
			_("while trying to determine filesystem size"));
		exit(1);
	}
	if (!param.s_blocks_count) {
		if (retval == EXT2_ET_UNIMPLEMENTED) {
			com_err(program_name, 0,
				_("Couldn't determine device size; you "
				"must specify\nthe size of the "
				"filesystem\n"));
			exit(1);
		} else {
			if (dev_size == 0) {
				com_err(program_name, 0,
				_("Device size reported to be zero.  "
				  "Invalid partition specified, or\n\t"
				  "partition table wasn't reread "
				  "after running fdisk, due to\n\t"
				  "a modified partition being busy "
				  "and in use.  You may need to reboot\n\t"
				  "to re-read your partition table.\n"
				  ));
				exit(1);
			}
			param.s_blocks_count = dev_size;
		}
		
	} else if (!force && (param.s_blocks_count > dev_size)) {
		com_err(program_name, 0,
			_("Filesystem larger than apparent filesystem size."));
		proceed_question();
	}

	/*
	 * If the user asked for HAS_JOURNAL, then make sure a journal
	 * gets created.
	 */
	if ((param.s_feature_compat & EXT3_FEATURE_COMPAT_HAS_JOURNAL) &&
	    !journal_size)
		journal_size = -1;

	set_fs_defaults(fs_type, &param, blocksize, &inode_ratio);

	if (param.s_blocks_per_group) {
		if (param.s_blocks_per_group < 256 ||
		    param.s_blocks_per_group > group_blk_max || *tmp) {
			com_err(program_name, 0,
				_("blocks per group count out of range"));
			exit(1);
		}
	}

	/*
	 * Calculate number of inodes based on the inode ratio
	 */
	param.s_inodes_count = num_inodes ? num_inodes : 
		((__u64) param.s_blocks_count * EXT2_BLOCK_SIZE(&param))
			/ inode_ratio;

	/*
	 * Calculate number of blocks to reserve
	 */
	param.s_r_blocks_count = (param.s_blocks_count * reserved_ratio) / 100;

}
					
int main (int argc, char *argv[])
{
	errcode_t	retval = 0;
	ext2_filsys	fs;
	badblocks_list	bb_list = 0;
	int		journal_blocks;
	int		i, val;

#ifdef ENABLE_NLS
	setlocale(LC_MESSAGES, "");
	bindtextdomain(NLS_CAT_NAME, LOCALEDIR);
	textdomain(NLS_CAT_NAME);
#endif
	PRS(argc, argv);

	/*
	 * Initialize the superblock....
	 */
	retval = ext2fs_initialize(device_name, 0, &param,
				   unix_io_manager, &fs);
	if (retval) {
		com_err(device_name, retval, _("while setting up superblock"));
		exit(1);
	}

	/*
	 * Wipe out the old on-disk superblock
	 */
	if (!noaction)
		zap_sector(fs, 2, 6);

	/*
	 * Generate a UUID for it...
	 */
	uuid_generate(fs->super->s_uuid);

	/*
	 * Add "jitter" to the superblock's check interval so that we
	 * don't check all the filesystems at the same time.  We use a
	 * kludgy hack of using the UUID to derive a random jitter value.
	 */
	for (i = 0, val = 0 ; i < sizeof(fs->super->s_uuid); i++)
		val += fs->super->s_uuid[i];
	fs->super->s_max_mnt_count += val % EXT2_DFL_MAX_MNT_COUNT;

	/*
	 * Override the creator OS, if applicable
	 */
	if (creator_os && !set_os(fs->super, creator_os)) {
		com_err (program_name, 0, _("unknown os - %s"), creator_os);
		exit(1);
	}

	/*
	 * For the Hurd, we will turn off filetype since it doesn't
	 * support it.
	 */
	if (fs->super->s_creator_os == EXT2_OS_HURD)
		fs->super->s_feature_incompat &=
			~EXT2_FEATURE_INCOMPAT_FILETYPE;

	/*
	 * Set the volume label...
	 */
	if (volume_label) {
		memset(fs->super->s_volume_name, 0,
		       sizeof(fs->super->s_volume_name));
		strncpy(fs->super->s_volume_name, volume_label,
			sizeof(fs->super->s_volume_name));
	}

	/*
	 * Set the last mount directory
	 */
	if (mount_dir) {
		memset(fs->super->s_last_mounted, 0,
		       sizeof(fs->super->s_last_mounted));
		strncpy(fs->super->s_last_mounted, mount_dir,
			sizeof(fs->super->s_last_mounted));
	}
	
	if (!quiet || noaction)
		show_stats(fs);

	if (noaction)
		exit(0);

	if (fs->super->s_feature_incompat &
	    EXT3_FEATURE_INCOMPAT_JOURNAL_DEV) {
		create_journal_dev(fs);
		ext2fs_close(fs);
		exit(0);
	}

	if (bad_blocks_filename)
		read_bb_file(fs, &bb_list, bad_blocks_filename);
	if (cflag)
		test_disk(fs, &bb_list);

	handle_bad_blocks(fs, bb_list);
	fs->stride = fs_stride;
	retval = ext2fs_allocate_tables(fs);
	if (retval) {
		com_err(program_name, retval,
			_("while trying to allocate filesystem tables"));
		exit(1);
	}
	if (super_only) {
		fs->super->s_state |= EXT2_ERROR_FS;
		fs->flags &= ~(EXT2_FLAG_IB_DIRTY|EXT2_FLAG_BB_DIRTY);
	} else {
		/* rsv must be a power of two (64kB is MD RAID sb alignment) */
		int rsv = 65536 / fs->blocksize;
		unsigned long blocks = fs->super->s_blocks_count;
		unsigned long start;
		blk_t ret_blk;

#ifdef ZAP_BOOTBLOCK
		zap_sector(fs, 0, 2);
#endif

		/*
		 * Wipe out any old MD RAID (or other) metadata at the end
		 * of the device.  This will also verify that the device is
		 * as large as we think.  Be careful with very small devices.
		 */
		start = (blocks & ~(rsv - 1));
		if (start > rsv)
			start -= rsv;
		if (start > 0)
			retval = zero_blocks(fs, start, blocks - start,
					     NULL, &ret_blk, NULL);

		if (retval) {
			com_err(program_name, retval,
				_("zeroing block %u at end of filesystem"),
				ret_blk);
			exit(1);
		}
		write_inode_tables(fs);
		create_root_dir(fs);
		create_lost_and_found(fs);
		reserve_inodes(fs);
		create_bad_block_inode(fs, bb_list);
	}

	if (journal_device) {
		ext2_filsys	jfs;
		
		if (!force)
			check_plausibility(journal_device); 
		check_mount(journal_device, force, _("journal"));

		retval = ext2fs_open(journal_device, EXT2_FLAG_RW|
				     EXT2_FLAG_JOURNAL_DEV_OK, 0,
				     fs->blocksize, unix_io_manager, &jfs);
		if (retval) {
			com_err(program_name, retval,
				_("while trying to open journal device %s\n"),
				journal_device);
			exit(1);
		}
		if (!quiet) {
			printf(_("Adding journal to device %s: "), 
			       journal_device);
			fflush(stdout);
		}
		retval = ext2fs_add_journal_device(fs, jfs);
		if(retval) {
			com_err (program_name, retval, 
				 _("\n\twhile trying to add journal to device %s"), 
				 journal_device);
			exit(1);
		}
		if (!quiet)
			printf(_("done\n"));
		ext2fs_close(jfs);
		free(journal_device);
	} else if (journal_size) {
		journal_blocks = figure_journal_size(journal_size, fs);

		if (!journal_blocks) {
			fs->super->s_feature_compat &=
				~EXT3_FEATURE_COMPAT_HAS_JOURNAL;
			goto no_journal;
		}
		if (!quiet) {
			printf(_("Creating journal (%d blocks): "),
			       journal_blocks);
			fflush(stdout);
		}
		retval = ext2fs_add_journal_inode(fs, journal_blocks,
						  journal_flags);
		if (retval) {
			com_err (program_name, retval,
				 _("\n\twhile trying to create journal"));
			exit(1);
		}
		if (!quiet)
			printf(_("done\n"));
	}
no_journal:

	if (!quiet)
		printf(_("Writing superblocks and "
		       "filesystem accounting information: "));
	retval = ext2fs_flush(fs);
	if (retval) {
		printf(_("\nWarning, had trouble writing out superblocks."));
	}
	if (!quiet) {
		printf(_("done\n\n"));
		print_check_message(fs);
	}
	ext2fs_close(fs);
	return 0;
}
