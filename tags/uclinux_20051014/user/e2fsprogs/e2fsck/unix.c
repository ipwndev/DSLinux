/*
 * unix.c - The unix-specific code for e2fsck
 * 
 * Copyright (C) 1993, 1994, 1995, 1996, 1997 Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
extern char *optarg;
extern int optind;
#endif
#include <unistd.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_MNTENT_H
#include <mntent.h>
#endif
#include <sys/ioctl.h>
#include <malloc.h>

#include "et/com_err.h"
#include "e2fsck.h"
#include "problem.h"
#include "../version.h"

/* Command line options */
static int blocksize = 0;
static int swapfs = 0;
static int normalize_swapfs = 0;
static int cflag = 0;		/* check disk */
static int show_version_only = 0;
static int force = 0;
static int verbose = 0;

static int replace_bad_blocks = 0;
static char *bad_blocks_file = 0;

static int possible_block_sizes[] = { 1024, 2048, 4096, 8192, 0};

static int root_filesystem = 0;
static int read_only_root = 0;

e2fsck_t e2fsck_global_ctx;	/* Try your very best not to use this! */

static void usage(e2fsck_t ctx)
{
	fprintf(stderr,
		_("Usage: %s [-panyrcdfvstFSV] [-b superblock] [-B blocksize]\n"
		"\t\t[-I inode_buffer_blocks] [-P process_inode_size]\n"
		"\t\t[-l|-L bad_blocks_file] [-C fd] [-j ext-journal] device\n"),
		ctx->program_name);

	fprintf(stderr, _("\nEmergency help:\n"
		" -p                   Automatic repair (no questions)\n"
		" -n                   Make no changes to the filesystem\n"
		" -y                   Assume \"yes\" to all questions\n"
		" -c                   Check for bad blocks\n"
		" -f                   Force checking even if filesystem is marked clean\n"));
	fprintf(stderr, _(""
		" -v                   Be verbose\n"
		" -b superblock        Use alternative superblock\n"
		" -B blocksize         Force blocksize when looking for superblock\n"
		" -j external-journal  Set location of the external journal\n"
		" -l bad_blocks_file   Add to badblocks list\n"
		" -L bad_blocks_file   Set badblocks list\n"
		));

	exit(FSCK_USAGE);
}

static void show_stats(e2fsck_t	ctx)
{
	ext2_filsys fs = ctx->fs;
	int inodes, inodes_used, blocks, blocks_used;
	int dir_links;
	int num_files, num_links;
	int frag_percent;

	dir_links = 2 * ctx->fs_directory_count - 1;
	num_files = ctx->fs_total_count - dir_links;
	num_links = ctx->fs_links_count - dir_links;
	inodes = fs->super->s_inodes_count;
	inodes_used = (fs->super->s_inodes_count -
		       fs->super->s_free_inodes_count);
	blocks = fs->super->s_blocks_count;
	blocks_used = (fs->super->s_blocks_count -
		       fs->super->s_free_blocks_count);

	frag_percent = (10000 * ctx->fs_fragmented) / inodes_used;
	frag_percent = (frag_percent + 5) / 10;
	
	if (!verbose) {
		printf(_("%s: %d/%d files (%0d.%d%% non-contiguous), %d/%d blocks\n"),
		       ctx->device_name, inodes_used, inodes,
		       frag_percent / 10, frag_percent % 10,
		       blocks_used, blocks);
		return;
	}
	/*
	 * This is a bit ugly. But I think there will nearly always be more
	 * than one "thing" to report about, so I won't try writing complex
	 * code to handle one/two/many forms of all words.
	 * Some languages (Italian, at least) never uses the plural form
	 * of foreign words, so in real life this could not be a problem.
	 * md@linux.it - 2000-1-15
	 */
#ifdef ENABLE_NLS
	printf (_("\n%8d inodes used (%d%%)\n"), inodes_used,
		(inodes_used != 1), 100 * inodes_used / inodes);
	printf (_("%8d non-contiguous inodes (%0d.%d%%)\n"),
		ctx->fs_fragmented, frag_percent / 10, frag_percent % 10);
	printf (_("         # of inodes with ind/dind/tind blocks: %d/%d/%d\n"),
		ctx->fs_ind_count, ctx->fs_dind_count, ctx->fs_tind_count);
	printf (_("%8d blocks used (%d%%)\n"
		"%8d bad blocks\n"), blocks_used,
		(int) ((long long) 100 * blocks_used / blocks),
		ctx->fs_badblocks_count);
	printf(_("%8d large files\n"), ctx->large_files);
	printf (_("\n%8d regular files\n"
		"%8d directories\n"
		"%8d character device files\n"
		"%8d block device files\n"
		"%8d fifos\n"
		"%8d links\n"
		"%8d symbolic links (%d fast symbolic links)\n"
		"%8d sockets\n"
		"--------\n"
		"%8d files\n"),
		ctx->fs_regular_count,
		ctx->fs_directory_count,
		ctx->fs_chardev_count,
		ctx->fs_blockdev_count,
		ctx->fs_fifo_count,
		ctx->fs_links_count - dir_links,
		ctx->fs_symlinks_count,
		ctx->fs_fast_symlinks_count,
		ctx->fs_sockets_count,
		ctx->fs_total_count - dir_links);
#else
	printf ("\n%8d inode%s used (%d%%)\n", inodes_used,
		(inodes_used != 1) ? "s" : "",
		100 * inodes_used / inodes);
	printf ("%8d non-contiguous inodes (%0d.%d%%)\n",
		ctx->fs_fragmented, frag_percent / 10, frag_percent % 10);
	printf ("         # of inodes with ind/dind/tind blocks: %d/%d/%d\n",
		ctx->fs_ind_count, ctx->fs_dind_count, ctx->fs_tind_count);
	printf ("%8d block%s used (%d%%)\n"
		"%8d bad block%s\n", blocks_used,
		(blocks_used != 1) ? "s" : "",
		100 * blocks_used / blocks, ctx->fs_badblocks_count,
		ctx->fs_badblocks_count != 1 ? "s" : "");
	printf(_("%8d large file%s\n"), ctx->large_files,
	       (ctx->large_files != 1) ? "s" : "");
	printf ("\n%8d regular file%s\n"
		"%8d director%s\n"
		"%8d character device file%s\n"
		"%8d block device file%s\n"
		"%8d fifo%s\n"
		"%8d link%s\n"
		"%8d symbolic link%s (%d fast symbolic link%s)\n"
		"%8d socket%s\n"
		"--------\n"
		"%8d file%s\n",
		ctx->fs_regular_count,
		(ctx->fs_regular_count != 1) ? "s" : "",
		ctx->fs_directory_count,
		(ctx->fs_directory_count != 1) ? "ies" : "y",
		ctx->fs_chardev_count,
		(ctx->fs_chardev_count != 1) ? "s" : "",
		ctx->fs_blockdev_count,
		(ctx->fs_blockdev_count != 1) ? "s" : "",
		ctx->fs_fifo_count,
		(ctx->fs_fifo_count != 1) ? "s" : "",
		ctx->fs_links_count - dir_links,
		((ctx->fs_links_count - dir_links) != 1) ? "s" : "",
		ctx->fs_symlinks_count,
		(ctx->fs_symlinks_count != 1) ? "s" : "",
		ctx->fs_fast_symlinks_count,
		(ctx->fs_fast_symlinks_count != 1) ? "s" : "",
		ctx->fs_sockets_count, (ctx->fs_sockets_count != 1) ? "s" : "",
		ctx->fs_total_count - dir_links,
		((ctx->fs_total_count - dir_links) != 1) ? "s" : "");
#endif
}

static void check_mount(e2fsck_t ctx)
{
	errcode_t	retval;
	int		mount_flags, cont, fd;

	retval = ext2fs_check_if_mounted(ctx->filesystem_name, &mount_flags);
	if (retval) {
		com_err("ext2fs_check_if_mount", retval,
			_("while determining whether %s is mounted."),
			ctx->filesystem_name);
		return;
	}

	/*
	 * If the filesystem isn't mounted, or it's the root filesystem
	 * and it's mounted read-only, then everything's fine.
	 */
	if ((!(mount_flags & EXT2_MF_MOUNTED)) ||
	    ((mount_flags & EXT2_MF_ISROOT) &&
	     (mount_flags & EXT2_MF_READONLY)))
		return;

	if (ctx->options & E2F_OPT_READONLY) {
		printf(_("Warning!  %s is mounted.\n"), ctx->filesystem_name);
		return;
	}

	printf(_("%s is mounted.  "), ctx->filesystem_name);
	if (!isatty(0) || !isatty(1))
		fatal_error(ctx, _("Cannot continue, aborting.\n\n"));
	printf(_("\n\n\007\007\007\007WARNING!!!  "
	       "Running e2fsck on a mounted filesystem may cause\n"
	       "SEVERE filesystem damage.\007\007\007\n\n"));
	cont = ask_yn(_("Do you really want to continue"), -1);
	if (!cont) {
		printf (_("check aborted.\n"));
		exit (0);
	}
	return;
}

/*
 * This routine checks to see if a filesystem can be skipped; if so,
 * it will exit with E2FSCK_OK.  Under some conditions it will print a
 * message explaining why a check is being forced.
 */
static void check_if_skip(e2fsck_t ctx)
{
	ext2_filsys fs = ctx->fs;
	const char *reason = NULL;
	unsigned int reason_arg = 0;
	
	if (force || bad_blocks_file || cflag || swapfs)
		return;
	
	if (fs->super->s_state & EXT2_ERROR_FS)
		reason = _(" contains a file system with errors");
	else if ((fs->super->s_state & EXT2_VALID_FS) == 0)
		reason = _(" was not cleanly unmounted");
	else if ((fs->super->s_max_mnt_count > 0) &&
		 (fs->super->s_mnt_count >=
		  (unsigned) fs->super->s_max_mnt_count)) {
		reason = _(" has been mounted %u times without being checked");
		reason_arg = fs->super->s_mnt_count;
	} else if (fs->super->s_checkinterval &&
		 time(0) >= (fs->super->s_lastcheck +
			     fs->super->s_checkinterval)) {
		reason = _(" has gone %u days without being checked");
		reason_arg = (time(0) - fs->super->s_lastcheck)/(3600*24);
	}
	if (reason) {
		fputs(ctx->device_name, stdout);
		printf(reason, reason_arg);
		fputs(_(", check forced.\n"), stdout);
		return;
	}
	printf(_("%s: clean, %d/%d files, %d/%d blocks\n"), ctx->device_name,
	       fs->super->s_inodes_count - fs->super->s_free_inodes_count,
	       fs->super->s_inodes_count,
	       fs->super->s_blocks_count - fs->super->s_free_blocks_count,
	       fs->super->s_blocks_count);
	ext2fs_close(fs);
	ctx->fs = NULL;
	e2fsck_free_context(ctx);
	exit(FSCK_OK);
}

/*
 * For completion notice
 */
struct percent_tbl {
	int	max_pass;
	int	table[32];
};
struct percent_tbl e2fsck_tbl = {
	5, { 0, 70, 90, 92,  95, 100 }
};
static char bar[] =
	"==============================================================="
	"===============================================================";
static char spaces[] =
	"                                                               "
	"                                                               ";

static float calc_percent(struct percent_tbl *tbl, int pass, int curr,
			  int max)
{
	float	percent;
	
	if (pass <= 0)
		return 0.0;
	if (pass > tbl->max_pass || max == 0)
		return 100.0;
	percent = ((float) curr) / ((float) max);
	return ((percent * (tbl->table[pass] - tbl->table[pass-1]))
		+ tbl->table[pass-1]);
}

extern void e2fsck_clear_progbar(e2fsck_t ctx)
{
	if (!(ctx->flags & E2F_FLAG_PROG_BAR))
		return;
	
	printf("%s\r", spaces + (sizeof(spaces) - 80));
	ctx->flags &= ~E2F_FLAG_PROG_BAR;
}

static int e2fsck_update_progress(e2fsck_t ctx, int pass,
				  unsigned long cur, unsigned long max)
{
	static const char spinner[] = "\\|/-";
	char buf[80];
	int	i;
	float percent;
	int	tick;
	struct timeval	tv;
	static int dpywidth = 0;

	if (pass == 0)
		return 0;
	
	if (ctx->progress_fd) {
		sprintf(buf, "%d %lu %lu\n", pass, cur, max);
		write(ctx->progress_fd, buf, strlen(buf));
	} else {
		if (ctx->flags & E2F_FLAG_PROG_SUPPRESS)
			return 0;
		if (dpywidth == 0) {
			dpywidth = 66 - strlen(ctx->device_name);
			dpywidth = 8 * (dpywidth / 8);
		}
		/*
		 * Calculate the new progress position.  If the
		 * percentage hasn't changed, then we skip out right
		 * away. 
		 */
		percent = calc_percent(&e2fsck_tbl, pass, cur, max);
		if (ctx->progress_last_percent == (int) 10 * percent)
			return 0;
		ctx->progress_last_percent = (int) 10 * percent;

		/*
		 * If we've already updated the spinner once within
		 * the last 1/8th of a second, no point doing it
		 * again.
		 */
		gettimeofday(&tv, NULL);
		tick = (tv.tv_sec << 3) + (tv.tv_usec / (1000000 / 8));
		if ((tick == ctx->progress_last_time) &&
		    (cur != max) && (cur != 0))
			return 0;
		ctx->progress_last_time = tick;

		/*
		 * Advance the spinner, and note that the progress bar
		 * will be on the screen
		 */
		ctx->progress_pos = (ctx->progress_pos+1) & 3;
		ctx->flags |= E2F_FLAG_PROG_BAR;
		
		i = ((percent * dpywidth) + 50) / 100;
		printf("%s: |%s%s", ctx->device_name,
		       bar + (sizeof(bar) - (i+1)),
		       spaces + (sizeof(spaces) - (dpywidth - i + 1)));
		if (percent == 100.0)
			fputc('|', stdout);
		else
			fputc(spinner[ctx->progress_pos & 3], stdout);
		printf(" %4.1f%%   \r", percent);
		if (percent == 100.0)
			e2fsck_clear_progbar(ctx);
		fflush(stdout);
	}
	return 0;
}

#define PATH_SET "PATH=/sbin"

static void reserve_stdio_fds(void)
{
	int	fd;

	while (1) {
		fd = open("/dev/null", O_RDWR);
		if (fd > 2)
			break;
		if (fd < 0) {
			fprintf(stderr, _("ERROR: Couldn't open "
				"/dev/null (%s)\n"),
				strerror(errno));
			break;
		}
	}
	close(fd);
}

#ifdef HAVE_SIGNAL_H
static void signal_progress_on(int sig)
{
	e2fsck_t ctx = e2fsck_global_ctx;

	if (!ctx)
		return;

	ctx->progress = e2fsck_update_progress;
	ctx->progress_fd = 0;
}

static void signal_progress_off(int sig)
{
	e2fsck_t ctx = e2fsck_global_ctx;

	if (!ctx)
		return;

	e2fsck_clear_progbar(ctx);
	ctx->progress = 0;
}
#endif

static errcode_t PRS(int argc, char *argv[], e2fsck_t *ret_ctx)
{
	int		flush = 0;
	int		c, fd;
#ifdef MTRACE
	extern void	*mallwatch;
#endif
	e2fsck_t	ctx;
	errcode_t	retval;
#ifdef HAVE_SIGNAL_H
	struct sigaction	sa;
#endif

	retval = e2fsck_allocate_context(&ctx);
	if (retval)
		return retval;

	*ret_ctx = ctx;

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	initialize_ext2_error_table();
	
	if (argc && *argv)
		ctx->program_name = *argv;
	else
		ctx->program_name = "e2fsck";
	while ((c = getopt (argc, argv, "panyrcC:B:dfvtFVM:b:I:j:P:l:L:N:Ss")) != EOF)
		switch (c) {
		case 'C':
			ctx->progress = e2fsck_update_progress;
			ctx->progress_fd = atoi(optarg);
			if (!ctx->progress_fd)
				break;
			/* Validate the file descriptor to avoid disasters */
			fd = dup(ctx->progress_fd);
			if (fd < 0) {
				fprintf(stderr,
				_("Error validating file descriptor %d: %s\n"),
					ctx->progress_fd,
					error_message(errno));
				fatal_error(ctx,
			_("Invalid completion information file descriptor"));
			} else
				close(fd);
			break;
		case 'p':
		case 'a':
			ctx->options |= E2F_OPT_PREEN;
			ctx->options &= ~(E2F_OPT_YES|E2F_OPT_NO);
			break;
		case 'n':
			ctx->options |= E2F_OPT_NO;
			ctx->options &= ~(E2F_OPT_YES|E2F_OPT_PREEN);
			break;
		case 'y':
			ctx->options |= E2F_OPT_YES;
			ctx->options &= ~(E2F_OPT_PREEN|E2F_OPT_NO);
			break;
		case 't':
#ifdef RESOURCE_TRACK
			if (ctx->options & E2F_OPT_TIME)
				ctx->options |= E2F_OPT_TIME2;
			else
				ctx->options |= E2F_OPT_TIME;
#else
			fprintf(stderr, _("The -t option is not "
				"supported on this version of e2fsck.\n"));
#endif
			break;
		case 'c':
			cflag++;
			ctx->options |= E2F_OPT_CHECKBLOCKS;
			break;
		case 'r':
			/* What we do by default, anyway! */
			break;
		case 'b':
			ctx->use_superblock = atoi(optarg);
			ctx->flags |= E2F_FLAG_SB_SPECIFIED;
			break;
		case 'B':
			blocksize = atoi(optarg);
			break;
		case 'I':
			ctx->inode_buffer_blocks = atoi(optarg);
			break;
		case 'j':
			ctx->journal_name = optarg;
			break;
		case 'P':
			ctx->process_inode_size = atoi(optarg);
			break;
		case 'L':
			replace_bad_blocks++;
		case 'l':
			bad_blocks_file = (char *) malloc(strlen(optarg)+1);
			if (!bad_blocks_file)
				fatal_error(ctx,
					    "Couldn't malloc bad_blocks_file");
			strcpy(bad_blocks_file, optarg);
			break;
		case 'd':
			ctx->options |= E2F_OPT_DEBUG;
			break;
		case 'f':
			force = 1;
			break;
		case 'F':
			flush = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'V':
			show_version_only = 1;
			break;
#ifdef MTRACE
		case 'M':
			mallwatch = (void *) strtol(optarg, NULL, 0);
			break;
#endif
		case 'N':
			ctx->device_name = optarg;
			break;
#ifdef ENABLE_SWAPFS
		case 's':
			normalize_swapfs = 1;
		case 'S':
			swapfs = 1;
			break;
#else
		case 's':
		case 'S':
			fprintf(stderr, _("Byte-swapping filesystems "
					  "not compiled in this version "
					  "of e2fsck\n"));
			exit(1);
#endif
		default:
			usage(ctx);
		}
	if (show_version_only)
		return 0;
	if (optind != argc - 1)
		usage(ctx);
	if ((ctx->options & E2F_OPT_NO) && !bad_blocks_file &&
	    !cflag && !swapfs)
		ctx->options |= E2F_OPT_READONLY;
	ctx->filesystem_name = argv[optind];
	if (flush) {
		int	fd = open(ctx->filesystem_name, O_RDONLY, 0);

		if (fd < 0) {
			com_err("open", errno,
				_("while opening %s for flushing"),
				ctx->filesystem_name);
			fatal_error(ctx, 0);
		}
		if ((retval = ext2fs_sync_device(fd, 1))) {
			com_err("ext2fs_sync_device", retval,
				_("while trying to flush %s"),
				ctx->filesystem_name);
			fatal_error(ctx, 0);
		}
		close(fd);
	}
#ifdef ENABLE_SWAPFS
	if (swapfs) {
		if (cflag || bad_blocks_file) {
			fprintf(stderr, _("Incompatible options not "
				"allowed when byte-swapping.\n"));
			exit(FSCK_USAGE);
		}
	}
#endif
#ifdef HAVE_SIGNAL_H
	/*
	 * Set up signal action
	 */
	memset(&sa, 0, sizeof(struct sigaction));
#ifdef SA_RESTART
	sa.sa_flags = SA_RESTART;
#endif
	e2fsck_global_ctx = ctx;
	sa.sa_handler = signal_progress_on;
	sigaction(SIGUSR1, &sa, 0);
	sa.sa_handler = signal_progress_off;
	sigaction(SIGUSR2, &sa, 0);
#endif

	/* Update our PATH to include /sbin if we need to run badblocks  */
	if (cflag) {
		char *oldpath = getenv("PATH");
		if (oldpath) {
			char *newpath;

			newpath = (char *) malloc(sizeof (PATH_SET) + 1 +
						  strlen (oldpath));
			if (!newpath)
				fatal_error(ctx, "Couldn't malloc() newpath");
			strcpy (newpath, PATH_SET);
			strcat (newpath, ":");
			strcat (newpath, oldpath);
			putenv (newpath);
		} else
			putenv (PATH_SET);
	}
	return 0;
}

static const char *my_ver_string = E2FSPROGS_VERSION;
static const char *my_ver_date = E2FSPROGS_DATE;
					
int main (int argc, char *argv[])
{
	errcode_t	retval = 0;
	int		exit_value = FSCK_OK;
	int		i;
	ext2_filsys	fs = 0;
	io_manager	io_ptr;
	struct ext2_super_block *sb;
	const char	*lib_ver_date;
	int		my_ver, lib_ver;
	e2fsck_t	ctx;
	struct problem_context pctx;
	int flags, run_result;
	
	clear_problem_context(&pctx);
#ifdef MTRACE
	mtrace();
#endif
#ifdef MCHECK
	mcheck(0);
#endif
#ifdef ENABLE_NLS
	setlocale(LC_MESSAGES, "");
	bindtextdomain(NLS_CAT_NAME, LOCALEDIR);
	textdomain(NLS_CAT_NAME);
#endif
	my_ver = ext2fs_parse_version_string(my_ver_string);
	lib_ver = ext2fs_get_library_version(0, &lib_ver_date);
	if (my_ver > lib_ver) {
		fprintf( stderr, _("Error: ext2fs library version "
			"out of date!\n"));
		show_version_only++;
	}
	
	retval = PRS(argc, argv, &ctx);
	if (retval) {
		com_err("e2fsck", retval,
			_("while trying to initialize program"));
		exit(FSCK_ERROR);
	}
	reserve_stdio_fds();
	
#ifdef RESOURCE_TRACK
	init_resource_track(&ctx->global_rtrack);
#endif

	if (!(ctx->options & E2F_OPT_PREEN) || show_version_only)
		fprintf (stderr, "e2fsck %s (%s)\n", my_ver_string,
			 my_ver_date);

	if (show_version_only) {
		fprintf(stderr, _("\tUsing %s, %s\n"),
			error_message(EXT2_ET_BASE), lib_ver_date);
		exit(FSCK_OK);
	}
	
	check_mount(ctx);
	
	if (!(ctx->options & E2F_OPT_PREEN) &&
	    !(ctx->options & E2F_OPT_NO) &&
	    !(ctx->options & E2F_OPT_YES)) {
		if (!isatty (0) || !isatty (1))
			fatal_error(ctx,
				    _("need terminal for interactive repairs"));
	}
	ctx->superblock = ctx->use_superblock;
restart:
#if 1
	io_ptr = unix_io_manager;
#else
	io_ptr = test_io_manager;
	test_io_backing_manager = unix_io_manager;
#endif
	flags = 0;
	if ((ctx->options & E2F_OPT_READONLY) == 0)
		flags |= EXT2_FLAG_RW;

	if (ctx->superblock && blocksize) {
		retval = ext2fs_open(ctx->filesystem_name, flags,
				     ctx->superblock, blocksize, io_ptr, &fs);
	} else if (ctx->superblock) {
		for (i=0; possible_block_sizes[i]; i++) {
			retval = ext2fs_open(ctx->filesystem_name, flags,
					     ctx->superblock,
					     possible_block_sizes[i],
					     io_ptr, &fs);
			if (!retval)
				break;
		}
	} else 
		retval = ext2fs_open(ctx->filesystem_name, flags, 
				     0, 0, io_ptr, &fs);
	if (!ctx->superblock && !(ctx->options & E2F_OPT_PREEN) &&
	    !(ctx->flags & E2F_FLAG_SB_SPECIFIED) &&
	    ((retval == EXT2_ET_BAD_MAGIC) ||
	     ((retval == 0) && ext2fs_check_desc(fs)))) {
		if (!fs || (fs->group_desc_count > 1)) {
			printf(_("%s trying backup blocks...\n"),
			       retval ? _("Couldn't find ext2 superblock,") :
			       _("Group descriptors look bad..."));
			ctx->superblock = get_backup_sb(fs);
			if (fs)
				ext2fs_close(fs);
			goto restart;
		}
	}
	if (retval) {
		com_err(ctx->program_name, retval, _("while trying to open %s"),
			ctx->filesystem_name);
		if (retval == EXT2_ET_REV_TOO_HIGH) {
			printf(_("The filesystem revision is apparently "
			       "too high for this version of e2fsck.\n"
			       "(Or the filesystem superblock "
			       "is corrupt)\n\n"));
			fix_problem(ctx, PR_0_SB_CORRUPT, &pctx);
		} else if (retval == EXT2_ET_SHORT_READ)
			printf(_("Could this be a zero-length partition?\n"));
		else if ((retval == EPERM) || (retval == EACCES))
			printf(_("You must have %s access to the "
			       "filesystem or be root\n"),
			       (ctx->options & E2F_OPT_READONLY) ?
			       "r/o" : "r/w");
		else if (retval == ENXIO)
			printf(_("Possibly non-existent or swap device?\n"));
#ifdef EROFS
		else if (retval == EROFS)
			printf(_("Disk write-protected; use the -n option "
			       "to do a read-only\n"
			       "check of the device.\n"));
#endif
		else
			fix_problem(ctx, PR_0_SB_CORRUPT, &pctx);
		fatal_error(ctx, 0);
	}
	ctx->fs = fs;
	fs->priv_data = ctx;
	sb = fs->super;
	if (sb->s_rev_level > E2FSCK_CURRENT_REV) {
		com_err(ctx->program_name, EXT2_ET_REV_TOO_HIGH,
			_("while trying to open %s"),
			ctx->filesystem_name);
	get_newer:
		fatal_error(ctx, _("Get a newer version of e2fsck!"));
	}

	/*
	 * Set the device name, which is used whenever we print error
	 * or informational messages to the user.
	 */
	if (ctx->device_name == 0 &&
	    (sb->s_volume_name[0] != 0)) {
		char *cp = malloc(sizeof(sb->s_volume_name)+1);
		if (cp) {
			strncpy(cp, sb->s_volume_name,
				sizeof(sb->s_volume_name));
			cp[sizeof(sb->s_volume_name)] = 0;
			ctx->device_name = cp;
		}
	}
	if (ctx->device_name == 0)
		ctx->device_name = ctx->filesystem_name;

	/*
	 * Make sure the ext3 superblock fields are consistent.
	 */
	retval = e2fsck_check_ext3_journal(ctx);
	if (retval) {
		com_err(ctx->program_name, retval,
			_("while checking ext3 journal for %s"),
			ctx->device_name);
		ext2fs_close(ctx->fs);
		fatal_error(ctx, 0);
	}

	/*
	 * Check to see if we need to do ext3-style recovery.  If so,
	 * do it, and then restart the fsck.
	 */
	if (sb->s_feature_incompat & EXT3_FEATURE_INCOMPAT_RECOVER) {
		if (ctx->options & E2F_OPT_READONLY) {
			printf(_("Warning: skipping journal recovery "
				 "because doing a read-only filesystem "
				 "check.\n"));
			io_channel_flush(ctx->fs->io);
		} else {
			retval = e2fsck_run_ext3_journal(ctx);
			if (retval) {
				com_err(ctx->program_name, retval,
				_("while recovering ext3 journal of %s"),
					ctx->device_name);
				fatal_error(ctx, 0);
			}
			ext2fs_close(ctx->fs);
			ctx->fs = 0;
			goto restart;
		}
	}

	/*
	 * Check for compatibility with the feature sets.  We need to
	 * be more stringent than ext2fs_open().
	 */
	if ((sb->s_feature_compat & ~EXT2_LIB_FEATURE_COMPAT_SUPP) ||
	    (sb->s_feature_incompat & ~EXT2_LIB_FEATURE_INCOMPAT_SUPP)) {
		com_err(ctx->program_name, EXT2_ET_UNSUPP_FEATURE,
			"(%s)", ctx->device_name);
		goto get_newer;
	}
	if (sb->s_feature_ro_compat & ~EXT2_LIB_FEATURE_RO_COMPAT_SUPP) {
		com_err(ctx->program_name, EXT2_ET_RO_UNSUPP_FEATURE,
			"(%s)", ctx->device_name);
		goto get_newer;
	}
#ifdef ENABLE_COMPRESSION
	if (sb->s_feature_incompat & EXT2_FEATURE_INCOMPAT_COMPRESSION)
		com_err(ctx->program_name, 0,
			_("Warning: compression support is experimental.\n"));
#endif
	
	/*
	 * If the user specified a specific superblock, presumably the
	 * master superblock has been trashed.  So we mark the
	 * superblock as dirty, so it can be written out.
	 */
	if (ctx->superblock &&
	    !(ctx->options & E2F_OPT_READONLY))
		ext2fs_mark_super_dirty(fs);

	/*
	 * Don't overwrite the backup superblock and block
	 * descriptors, until we're sure the filesystem is OK....
	 */
	fs->flags |= EXT2_FLAG_MASTER_SB_ONLY;

	ehandler_init(fs->io);

	if (ctx->superblock)
		set_latch_flags(PR_LATCH_RELOC, PRL_LATCHED, 0);
	check_super_block(ctx);
	if (ctx->flags & E2F_FLAG_SIGNAL_MASK)
		fatal_error(ctx, 0);
	check_if_skip(ctx);
	if (bad_blocks_file)
		read_bad_blocks_file(ctx, bad_blocks_file, replace_bad_blocks);
	else if (cflag)
		test_disk(ctx);
	if (ctx->flags & E2F_FLAG_SIGNAL_MASK)
		fatal_error(ctx, 0);
#ifdef ENABLE_SWAPFS
	if (normalize_swapfs) {
		if ((fs->flags & EXT2_FLAG_SWAP_BYTES) ==
		    ext2fs_native_flag()) {
			fprintf(stderr, _("%s: Filesystem byte order "
				"already normalized.\n"), ctx->device_name);
			fatal_error(ctx, 0);
		}
	}
	if (swapfs) {
		swap_filesys(ctx);
		if (ctx->flags & E2F_FLAG_SIGNAL_MASK)
			fatal_error(ctx, 0);
	}
#endif

	/*
	 * Mark the system as valid, 'til proven otherwise
	 */
	ext2fs_mark_valid(fs);

	retval = ext2fs_read_bb_inode(fs, &fs->badblocks);
	if (retval) {
		com_err(ctx->program_name, retval,
			_("while reading bad blocks inode"));
		preenhalt(ctx);
		printf(_("This doesn't bode well,"
			 " but we'll try to go on...\n"));
	}

	run_result = e2fsck_run(ctx);
	e2fsck_clear_progbar(ctx);
	if (run_result == E2F_FLAG_RESTART) {
		printf(_("Restarting e2fsck from the beginning...\n"));
		retval = e2fsck_reset_context(ctx);
		if (retval) {
			com_err(ctx->program_name, retval,
				_("while resetting context"));
			fatal_error(ctx, 0);
		}
		ext2fs_close(fs);
		goto restart;
	}
	if (run_result & E2F_FLAG_SIGNAL_MASK)
		fatal_error(ctx, 0);
	if (run_result & E2F_FLAG_CANCEL)
		ext2fs_unmark_valid(fs);

#ifdef MTRACE
	mtrace_print("Cleanup");
#endif
	if (ext2fs_test_changed(fs)) {
		exit_value = FSCK_NONDESTRUCT;
		if (!(ctx->options & E2F_OPT_PREEN))
		    printf(_("\n%s: ***** FILE SYSTEM WAS MODIFIED *****\n"),
			       ctx->device_name);
		if (root_filesystem && !read_only_root) {
			printf(_("%s: ***** REBOOT LINUX *****\n"),
			       ctx->device_name);
			exit_value = FSCK_REBOOT;
		}
	}
	if (ext2fs_test_valid(fs))
		fs->flags &= ~EXT2_FLAG_MASTER_SB_ONLY;
	else {
		printf(_("\n%s: ********** WARNING: Filesystem still has "
			 "errors **********\n\n"), ctx->device_name);
		exit_value = FSCK_UNCORRECTED;
	}
	if (!(ctx->options & E2F_OPT_READONLY)) {
		if (ext2fs_test_valid(fs)) {
			if (!(sb->s_state & EXT2_VALID_FS))
				exit_value = FSCK_NONDESTRUCT;
			sb->s_state = EXT2_VALID_FS;
		} else
			sb->s_state &= ~EXT2_VALID_FS;
		sb->s_mnt_count = 0;
		sb->s_lastcheck = time(NULL);
		ext2fs_mark_super_dirty(fs);
	}
	show_stats(ctx);

	e2fsck_write_bitmaps(ctx);
	
	ext2fs_close(fs);
	ctx->fs = NULL;
	e2fsck_free_context(ctx);
	
#ifdef RESOURCE_TRACK
	if (ctx->options & E2F_OPT_TIME)
		print_resource_track(NULL, &ctx->global_rtrack);
#endif

	return exit_value;
}
