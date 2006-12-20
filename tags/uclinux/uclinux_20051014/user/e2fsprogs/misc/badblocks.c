/*
 * badblocks.c		- Bad blocks checker
 *
 * Copyright (C) 1992, 1993, 1994  Remy Card <card@masi.ibp.fr>
 *                                 Laboratoire MASI, Institut Blaise Pascal
 *                                 Universite Pierre et Marie Curie (Paris VI)
 *
 * Copyright 1995, 1996, 1997, 1998, 1999 by Theodore Ts'o
 * Copyright 1999 by David Beattie
 *
 * This file is based on the minix file system programs fsck and mkfs
 * written and copyrighted by Linus Torvalds <Linus.Torvalds@cs.helsinki.fi>
 * 
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

/*
 * History:
 * 93/05/26	- Creation from e2fsck
 * 94/02/27	- Made a separate bad blocks checker
 * 99/06/30...99/07/26 - Added non-destructive write-testing,
 *                       configurable blocks-at-once parameter,
 * 			 loading of badblocks list to avoid testing
 * 			 blocks known to be bad, multiple passes to 
 * 			 make sure that no new blocks are added to the
 * 			 list.  (Work done by David Beattie)
 */

#include <errno.h>
#include <fcntl.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
extern char *optarg;
extern int optind;
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include "et/com_err.h"
#include "ext2fs/ext2_io.h"
#include "ext2fs/ext2_fs.h"
#include "ext2fs/ext2fs.h"
#include "nls-enable.h"

const char * program_name = "badblocks";
const char * done_string = N_("done                        \n");

static int v_flag = 0;			/* verbose */
static int w_flag = 0;			/* do r/w test: 0=no, 1=yes,
					 * 2=non-destructive */
static int s_flag = 0;			/* show progress of test */
static int force = 0;			/* force check of mounted device */

static void usage(void)
{
	fprintf(stderr, _("Usage: %s [-b block_size] [-i input_file] [-o output_file] [-svwnf]\n [-c blocks_at_once] [-p num_passes] device [last_block [start_count]]\n"),
		 program_name);
	exit (1);
}

static unsigned long currently_testing = 0;
static unsigned long num_blocks = 0;
static ext2_badblocks_list bb_list = NULL;
static FILE *out;
static blk_t next_bad = 0;
static ext2_badblocks_iterate bb_iter = NULL;

/*
 * This routine reports a new bad block.  If the bad block has already
 * been seen before, then it returns 0; otherwise it returns 1.
 */
static int bb_output (unsigned long bad)
{
	errcode_t errcode;

	if (ext2fs_badblocks_list_test(bb_list, bad))
		return 0;

	fprintf (out, "%lu\n", bad);

	errcode = ext2fs_badblocks_list_add (bb_list, bad);
	if (errcode) {
		com_err (program_name, errcode, "adding to in-memory bad block list");
		exit (1);
	}

	/* kludge:
	   increment the iteration through the bb_list if 
	   an element was just added before the current iteration
	   position.  This should not cause next_bad to change. */
	if (bb_iter && bad < next_bad)
		ext2fs_badblocks_list_iterate (bb_iter, &next_bad);
	return 1;
}

static void print_status(void)
{
	fprintf(stderr, "%9ld/%9ld", currently_testing, num_blocks);
	fprintf(stderr, "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	fflush (stderr);
}

static void alarm_intr(int alnum)
{
	signal (SIGALRM, alarm_intr);
	alarm(1);
	if (!num_blocks)
		return;
	fprintf(stderr, "%9ld/%9ld", currently_testing, num_blocks);
	fprintf(stderr, "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
	fflush (stderr);
}

static void *terminate_addr = NULL;

static void terminate_intr(int signo)
{
	if (terminate_addr)
		longjmp(terminate_addr,1);
	exit(1);
}

static void capture_terminate(jmp_buf term_addr)
{
	terminate_addr = term_addr;
	signal (SIGHUP, terminate_intr);
	signal (SIGINT, terminate_intr);
	signal (SIGPIPE, terminate_intr);
	signal (SIGTERM, terminate_intr);
	signal (SIGUSR1, terminate_intr);
	signal (SIGUSR2, terminate_intr);
}

static void uncapture_terminate(void)
{
	terminate_addr = NULL;
	signal (SIGHUP, SIG_DFL);
	signal (SIGINT, SIG_DFL);
	signal (SIGPIPE, SIG_DFL);
	signal (SIGTERM, SIG_DFL);
	signal (SIGUSR1, SIG_DFL);
	signal (SIGUSR2, SIG_DFL);
}

/*
 * Perform a read of a sequence of blocks; return the number of blocks
 *    successfully sequentially read.
 */
static long do_read (int dev, char * buffer, int try, int block_size,
		     unsigned long current_block)
{
	long got;

	if (v_flag > 1)
		print_status();

	/* Seek to the correct loc. */
	if (ext2fs_llseek (dev, (ext2_loff_t) current_block * block_size,
			 SEEK_SET) != (ext2_loff_t) current_block * block_size)
		com_err (program_name, errno, _("during seek"));

	/* Try the read */
	got = read (dev, buffer, try * block_size);
	if (got < 0)
		got = 0;	
	if (got & 511)
		fprintf(stderr, _("Weird value (%ld) in do_read\n"), got);
	got /= block_size;
	return got;
}

/*
 * Perform a write of a sequence of blocks; return the number of blocks
 *    successfully sequentially written.
 */
static long do_write (int dev, char * buffer, int try, int block_size,
		     unsigned long current_block)
{
	long got;

	if (v_flag > 1)
		print_status();

	/* Seek to the correct loc. */
	if (ext2fs_llseek (dev, (ext2_loff_t) current_block * block_size,
			 SEEK_SET) != (ext2_loff_t) current_block * block_size)
		com_err (program_name, errno, _("during seek"));

	/* Try the write */
	got = write (dev, buffer, try * block_size);
	if (got < 0)
		got = 0;	
	if (got & 511)
		fprintf (stderr,
			 "Weird value (%ld) in do_write\n", got);
	got /= block_size;
	return got;
}

static int host_dev;

static void flush_bufs(void)
{
	errcode_t	retval;

	retval = ext2fs_sync_device(host_dev, 1);
	if (retval)
		com_err(program_name, retval, _("during ext2fs_sync_device"));
}

static unsigned int test_ro (int dev, unsigned long last_block,
			     int block_size, unsigned long from_count,
			     unsigned long blocks_at_once)
{
	char * blkbuf;
	int try;
	long got;
	unsigned int bb_count = 0;
	errcode_t errcode;

	errcode = ext2fs_badblocks_list_iterate_begin(bb_list,&bb_iter);
	if (errcode) {
		com_err (program_name, errcode,
			 _("while beginning bad block list iteration"));
		exit (1);
	}
	do {
		ext2fs_badblocks_list_iterate (bb_iter, &next_bad);
	} while (next_bad && next_bad < from_count);

	blkbuf = malloc (blocks_at_once * block_size);
	if (!blkbuf)
	{
		com_err (program_name, ENOMEM, _("while allocating buffers"));
		exit (1);
	}
	flush_bufs();
	if (v_flag) {
	    fprintf(stderr, _("Checking for bad blocks in read-only mode\n"));
	    fprintf (stderr, _("From block %lu to %lu\n"), from_count,
		     last_block);
	}
	try = blocks_at_once;
	currently_testing = from_count;
	num_blocks = last_block;
	if (s_flag || v_flag > 1) {
		fprintf(stderr,
			_("Checking for bad blocks (read-only test): "));
		if (v_flag <= 1)
			alarm_intr(SIGALRM);
	}
	while (currently_testing < last_block)
	{
		if (next_bad) {
			if (currently_testing == next_bad) {
				/* fprintf (out, "%lu\n", nextbad); */
				ext2fs_badblocks_list_iterate (bb_iter, &next_bad);
				currently_testing++;
				continue;
			}
			else if (currently_testing + try > next_bad)
				try = next_bad - currently_testing;
		}
		if (currently_testing + try > last_block)
			try = last_block - currently_testing;
		got = do_read (dev, blkbuf, try, block_size, currently_testing);
		currently_testing += got;
		if (got == try) {
			try = blocks_at_once;
			continue;
		}
		else
			try = 1;
		if (got == 0) {
			bb_count += bb_output(currently_testing++);
		}
	}
	num_blocks = 0;
	alarm(0);
	if (s_flag || v_flag > 1)
		fprintf(stderr, _(done_string));

	fflush (stderr);
	free (blkbuf);

	ext2fs_badblocks_list_iterate_end(bb_iter);

	return bb_count;
}

static unsigned int test_rw (int dev, unsigned long last_block,
			     int block_size, unsigned long from_count,
			     unsigned long blocks_at_once)
{
	int i;
	char * buffer;
	static unsigned char pattern[] = {0xaa, 0x55, 0xff, 0x00};
	unsigned int bb_count = 0;

	buffer = malloc (2 * block_size);
	if (!buffer)
	{
		com_err (program_name, ENOMEM, _("while allocating buffers"));
		exit (1);
	}

	flush_bufs();

	if (v_flag) {
		fprintf(stderr,
			_("Checking for bad blocks in read-write mode\n"));
		fprintf(stderr, _("From block %lu to %lu\n"),
			 from_count, last_block);
	}
	for (i = 0; i < sizeof (pattern); i++) {
		memset (buffer, pattern[i], block_size);
		if (s_flag | v_flag)
			fprintf (stderr, _("Writing pattern 0x%08x: "),
				 *((int *) buffer));
		num_blocks = last_block;
		currently_testing = from_count;
		if (s_flag && v_flag <= 1)
			alarm_intr(SIGALRM);
		for (;
		     currently_testing < last_block;
		     currently_testing++)
		{
			if (ext2fs_llseek (dev, (ext2_loff_t) currently_testing *
					 block_size, SEEK_SET) !=
			    (ext2_loff_t) currently_testing * block_size)
				com_err (program_name, errno,
					 _("during seek on block %d"),
					 currently_testing);
			if (v_flag > 1)
				print_status();
			write (dev, buffer, block_size);
		}
		num_blocks = 0;
		alarm (0);
		if (s_flag | v_flag)
			fprintf(stderr, _(done_string));
		flush_bufs();
		if (s_flag | v_flag)
			fprintf (stderr, _("Reading and comparing: "));
		num_blocks = last_block;
		currently_testing = from_count;
		if (s_flag && v_flag <= 1)
			alarm_intr(SIGALRM);
		for (;
		     currently_testing < last_block;
		     currently_testing++)
		{
			if (ext2fs_llseek (dev, (ext2_loff_t) currently_testing *
					 block_size, SEEK_SET) !=
			    (ext2_loff_t) currently_testing * block_size)
				com_err (program_name, errno,
					 _("during seek on block %d"),
					 currently_testing);
			if (v_flag > 1)
				print_status();
			if ((read (dev, buffer + block_size, block_size) 
			     != block_size) ||
			    memcmp(buffer, buffer + block_size, block_size))
				bb_count += bb_output(currently_testing);
		}
		num_blocks = 0;
		alarm (0);
		if (s_flag | v_flag)
			fprintf(stderr, _(done_string));
		flush_bufs();
	}

	return bb_count;
}

struct saved_blk_record {
	blk_t	block;
	int	num;
};

static unsigned int test_nd (int dev, unsigned long last_block,
			     int block_size, unsigned long from_count,
			     unsigned long blocks_at_once)
{
	char *blkbuf, *save_ptr, *test_ptr, *read_ptr;
	char * ptr;
	int try, i;
	long got, used2, written, save_currently_testing;
	struct saved_blk_record *test_record;
	/* This is static to prevent being clobbered by the longjmp */
	static int num_saved;
	jmp_buf terminate_env;
	errcode_t errcode;
	long buf_used;
	unsigned int bb_count;

	errcode = ext2fs_badblocks_list_iterate_begin(bb_list,&bb_iter);
	if (errcode) {
		com_err (program_name, errcode,
			 _("while beginning bad block list iteration"));
		exit (1);
	}
	do {
		ext2fs_badblocks_list_iterate (bb_iter, &next_bad);
	} while (next_bad && next_bad < from_count);

	blkbuf = malloc (3 * blocks_at_once * block_size);
	test_record = malloc (blocks_at_once*sizeof(struct saved_blk_record));
	if (!blkbuf || !test_record) {
		com_err(program_name, ENOMEM, _("while allocating buffers"));
		exit (1);
	}
	num_saved = 0;

	/* inititalize the test data randomly: */
	if (v_flag) {
		fprintf (stderr, _("Initializing random test data\n"));
	}
	for(ptr = blkbuf + blocks_at_once * block_size;
	    ptr < blkbuf + 2 * blocks_at_once * block_size;
	    ++ptr) {
		(*ptr) = random() % (1 << sizeof(char));
	}

	flush_bufs();
	if (v_flag) {
	    fprintf (stderr,
		     _("Checking for bad blocks in non-destructive read-write mode\n"));
	    fprintf (stderr, _("From block %lu to %lu\n"), from_count, last_block);
	}
	if (s_flag || v_flag > 1) {
		fprintf(stderr, _("Checking for bad blocks (non-destructive read-write test): "));
		if (v_flag <= 1)
			alarm_intr(SIGALRM);
	}
	if (setjmp(terminate_env)) {
		/*
		 * Abnormal termination by a signal is handled here.
		 */
		signal (SIGALRM, SIG_IGN);
		fprintf(stderr, _("\nInterrupt caught, cleaning up\n"));

		save_ptr = blkbuf;
		for (i=0; i < num_saved; i++) {
			do_write(dev, save_ptr, test_record[i].num,
				 block_size, test_record[i].block);
			save_ptr += test_record[i].num * block_size;
		}
		fflush (out);
		exit(1);
	}
	
	/* set up abend handler */
	capture_terminate(terminate_env);

	buf_used = 0;
	bb_count = 0;
	save_ptr = blkbuf;
	test_ptr = blkbuf + (blocks_at_once * block_size);
	currently_testing = from_count;
	num_blocks = last_block;

	while (currently_testing < last_block) {
		try = blocks_at_once - buf_used;
		if (next_bad) {
			if (currently_testing == next_bad) {
				/* fprintf (out, "%lu\n", nextbad); */
				ext2fs_badblocks_list_iterate (bb_iter, &next_bad);
				currently_testing++;
				goto check_for_more;
			}
			else if (currently_testing + try > next_bad)
				try = next_bad - currently_testing;
		}
		if (currently_testing + try > last_block)
			try = last_block - currently_testing;
		got = do_read (dev, save_ptr, try, block_size,
			       currently_testing);
		if (got == 0) {
			/* First block must have been bad. */
			bb_count += bb_output(currently_testing++);
			goto check_for_more;
		}

		/*
		 * Note the fact that we've saved this much data
		 * *before* we overwrite it with test data
		 */
		test_record[num_saved].block = currently_testing;
		test_record[num_saved].num = got;
		num_saved++;

		/* Write the test data */
		written = do_write (dev, test_ptr, got, block_size,
				    currently_testing);
		if (written != got)
			com_err (program_name, errno,
				 _("during test data write, block %lu"),
				 currently_testing + written);

		buf_used += got;
		save_ptr += got * block_size;
		test_ptr += got * block_size;
		currently_testing += got;
		if (got != try)
			bb_count += bb_output(currently_testing++);

	check_for_more:
		/*
		 * If there's room for more blocks to be tested this
		 * around, and we're not done yet testing the disk, go
		 * back and get some more blocks.
		 */
		if ((buf_used != blocks_at_once) &&
		    (currently_testing < last_block))
			continue;

		flush_bufs();
		save_currently_testing = currently_testing;

		/*
		 * for each contiguous block that we read into the
		 * buffer (and wrote test data into afterwards), read
		 * it back (looping if necessary, to get past newly
		 * discovered unreadable blocks, of which there should
		 * be none, but with a hard drive which is unreliable,
		 * it has happened), and compare with the test data
		 * that was written; output to the bad block list if
		 * it doesn't match.
		 */
		used2 = 0;
		save_ptr = blkbuf;
		test_ptr = blkbuf + (blocks_at_once * block_size);
		read_ptr = blkbuf + (2 * blocks_at_once * block_size);
		try = 0;

		while (1) {
			if (try == 0) {
				if (used2 >= num_saved)
					break;
				currently_testing = test_record[used2].block;
				try = test_record[used2].num;
				used2++;
			}
				
			got = do_read (dev, read_ptr, try,
				       block_size, currently_testing);

			/* test the comparison between all the
			   blocks successfully read  */
			for (i = 0; i < got; ++i)
				if (memcmp (test_ptr+i*block_size,
					    read_ptr+i*block_size, block_size))
					bb_count += bb_output(currently_testing + i);
			if (got < try) {
				bb_count += bb_output(currently_testing + got);
				got++;
			}
					
			/* when done, write back original data */
			do_write (dev, save_ptr, got, block_size,
				  currently_testing);

			currently_testing += got;
			save_ptr += got * block_size;
			test_ptr += got * block_size;
			read_ptr += got * block_size;
			try -= got;
		}

		/* empty the buffer so it can be reused */
		num_saved = 0;
		buf_used = 0;
		save_ptr = blkbuf;
		test_ptr = blkbuf + (blocks_at_once * block_size);
		currently_testing = save_currently_testing;
	}
	num_blocks = 0;
	alarm(0);
	uncapture_terminate();
	if (s_flag || v_flag > 1)
		fprintf(stderr, _(done_string));

	fflush(stderr);
	free(blkbuf);
	free(test_record);

	ext2fs_badblocks_list_iterate_end(bb_iter);

	return bb_count;
}

static void check_mount(char *device_name)
{
	errcode_t	retval;
	int		mount_flags;

	retval = ext2fs_check_if_mounted(device_name, &mount_flags);
	if (retval) {
		com_err("ext2fs_check_if_mount", retval,
			_("while determining whether %s is mounted."),
			device_name);
		return;
	}
	if (!(mount_flags & EXT2_MF_MOUNTED))
		return;

	fprintf(stderr, _("%s is mounted; "), device_name);
	if (force) {
		fprintf(stderr, _("badblocks forced anyway.  "
			"Hope /etc/mtab is incorrect.\n"));
		return;
	}
	fprintf(stderr, _("it's not safe to run badblocks!\n"));
	exit(1);
}


int main (int argc, char ** argv)
{
	int c;
	char * tmp;
	char * device_name;
	char * host_device_name = NULL;
	char * input_file = NULL;
	char * output_file = NULL;
	FILE * in = NULL;
	int block_size = 1024;
	unsigned long blocks_at_once = 16;
	blk_t last_block, from_count;
	int num_passes = 0;
	int passes_clean = 0;
	int dev;
	errcode_t errcode;
	unsigned int (*test_func)(int, unsigned long,
				  int, unsigned long,
				  unsigned long);

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
#ifdef ENABLE_NLS
	setlocale(LC_MESSAGES, "");
	bindtextdomain(NLS_CAT_NAME, LOCALEDIR);
	textdomain(NLS_CAT_NAME);
#endif
	test_func = test_ro;

	if (argc && *argv)
		program_name = *argv;
	while ((c = getopt (argc, argv, "b:fi:o:svwnc:p:h:")) != EOF) {
		switch (c) {
		case 'b':
			block_size = strtoul (optarg, &tmp, 0);
			if (*tmp || block_size > 4096) {
				com_err (program_name, 0,
					 _("bad block size - %s"), optarg);
				exit (1);
			}
			break;
		case 'f':
			force++;
			break;
		case 'i':
			input_file = optarg;
			break;
		case 'o':
			output_file = optarg;
			break;
		case 's':
			s_flag = 1;
			break;
		case 'v':
			v_flag++;
			break;
		case 'w':
			if (w_flag)
				usage();
			test_func = test_rw;
			w_flag = 1;
			break;
		case 'n':
			if (w_flag)
				usage();
			test_func = test_nd;
			w_flag = 2;
			break;
		case 'c':
			blocks_at_once = strtoul (optarg, &tmp, 0);
			if (*tmp) {
				com_err (program_name, 0,
					 "bad simultaneous block count - %s", optarg);
				exit (1);
			}
			break;
		case 'p':
			num_passes = strtoul (optarg, &tmp, 0);
			if (*tmp) {
				com_err (program_name, 0,
				    "bad number of clean passes - %s", optarg);
				exit (1);
			}
			break;
		case 'h':
			host_device_name = optarg;
			break;
		default:
			usage();
		}
	}
	if (optind > argc - 1)
		usage();
	device_name = argv[optind++];
	if (optind > argc - 1) {
		errcode = ext2fs_get_device_size(device_name,
						 block_size,
						 &last_block);
		if (errcode == EXT2_ET_UNIMPLEMENTED) {
			com_err(program_name, 0,
				_("Couldn't determine device size; you "
				  "must specify\nthe size manually\n"));
			exit(1);
		}
		if (errcode) {
			com_err(program_name, errcode,
				_("while trying to determine device size"));
			exit(1);
		}
	} else {
		last_block = strtoul (argv[optind], &tmp, 0);
		if (*tmp) {
			com_err (program_name, 0, _("bad blocks count - %s"),
				 argv[optind]);
			exit (1);
		}
		optind++;
	}
	if (optind <= argc-1) {
		from_count = strtoul (argv[optind], &tmp, 0);
		if (*tmp) {
			com_err (program_name, 0, _("bad starting block - %s"),
				 argv[optind]);
			exit (1);
		}
	} else from_count = 0;
	if (from_count >= last_block) {
	    com_err (program_name, 0, _("bad blocks range: %lu-%lu"),
		     from_count, last_block);
	    exit (1);
	}
	if (w_flag)
		check_mount(device_name);
	
	dev = open (device_name, w_flag ? O_RDWR : O_RDONLY);
	if (dev == -1)
	{
		com_err (program_name, errno, _("while trying to open %s"),
			 device_name);
		exit (1);
	}
	if (host_device_name) {
		host_dev = open (host_device_name, O_RDONLY);
		if (host_dev == -1)
		{
			com_err (program_name, errno,
				 _("while trying to open %s"),
				 host_device_name);
			exit (1);
		}
	} else
		host_dev = dev;
	if (input_file)
		if (strcmp (input_file, "-") == 0)
			in = stdin;
		else {
			in = fopen (input_file, "r");
			if (in == NULL)
			{
				com_err (program_name, errno,
					 _("while trying to open %s"),
					 input_file);
				exit (1);
			}
		}
	if (output_file && strcmp (output_file, "-") != 0)
	{
		out = fopen (output_file, "w");
		if (out == NULL)
		{
			com_err (program_name, errno,
				 _("while trying to open %s"),
				 output_file);
			exit (1);
		}
	}
	else
		out = stdout;

	errcode = ext2fs_badblocks_list_create(&bb_list,0);
	if (errcode) {
		com_err (program_name, errcode,
			 _("creating in-memory bad blocks list"));
		exit (1);
	}

	if (in) {
		for(;;) {
			switch(fscanf (in, "%u\n", &next_bad)) {
				case 0:
					com_err (program_name, 0, "input file - bad format");
					exit (1);
				case EOF:
					break;
				default:
					errcode = ext2fs_badblocks_list_add(bb_list,next_bad);
					if (errcode) {
						com_err (program_name, errcode, _("adding to in-memory bad block list"));
						exit (1);
					}
					continue;
			}
			break;
		}

		if (in != stdin)
			fclose (in);
	}

	do {
		unsigned int bb_count;

		bb_count = test_func(dev, last_block, block_size,
				     from_count, blocks_at_once);
		if (bb_count)
			passes_clean = 0;
		else
			++passes_clean;
		
		if (v_flag)
			fprintf(stderr,
				_("Pass completed, %u bad blocks found.\n"), 
				bb_count);

	} while (passes_clean < num_passes);

	close (dev);
	if (out != stdout)
		fclose (out);
	return 0;
}

