/*
 * e2image.c --- Program which writes an image file backing up
 * critical metadata for the filesystem.
 *
 * Copyright 2000, 2001 by Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include <fcntl.h>
#include <grp.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
extern char *optarg;
extern int optind;
#endif
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "ext2fs/ext2_fs.h"
#include "ext2fs/ext2fs.h"
#include "et/com_err.h"
#include "uuid/uuid.h"
#include "e2p/e2p.h"
#include "ext2fs/e2image.h"

#include "../version.h"
#include "nls-enable.h"

const char * program_name = "e2image";
char * device_name = NULL;

static void usage(void)
{
	fprintf(stderr, _("Usage: %s [-r] device file\n"), program_name);
	exit (1);
}

static void write_header(int fd, struct ext2_image_hdr *hdr, int blocksize)
{
	char *header_buf;
	int actual;

	header_buf = malloc(blocksize);
	if (!header_buf) {
		fprintf(stderr, _("Couldn't allocate header buffer\n"));
		exit(1);
	}

	if (lseek(fd, 0, SEEK_SET) < 0) {
		perror("lseek while writing header");
		exit(1);
	}
	memset(header_buf, 0, blocksize);
	
	if (hdr)
		memcpy(header_buf, hdr, sizeof(struct ext2_image_hdr));
	
	actual = write(fd, header_buf, blocksize);
	if (actual < 0) {
		perror("write header");
		exit(1);
	}
	if (actual != blocksize) {
		fprintf(stderr, _("short write (only %d bytes) for"
				  "writing image header"), actual);
		exit(1);
	}
	free(header_buf);
}

static void write_image_file(ext2_filsys fs, int fd)
{
	struct ext2_image_hdr	hdr;
	struct stat		st;
	errcode_t		retval;

	write_header(fd, NULL, fs->blocksize);
	memset(&hdr, 0, sizeof(struct ext2_image_hdr));

	hdr.offset_super = lseek(fd, 0, SEEK_CUR);
	retval = ext2fs_image_super_write(fs, fd, 0);
	if (retval) {
		com_err(program_name, retval, _("while writing superblock"));
		exit(1);
	}
	
	hdr.offset_inode = lseek(fd, 0, SEEK_CUR);
	retval = ext2fs_image_inode_write(fs, fd,
				  (fd != 1) ? IMAGER_FLAG_SPARSEWRITE : 0);
	if (retval) {
		com_err(program_name, retval, _("while writing inode table"));
		exit(1);
	}
	
	hdr.offset_blockmap = lseek(fd, 0, SEEK_CUR);
	retval = ext2fs_image_bitmap_write(fs, fd, 0);
	if (retval) {
		com_err(program_name, retval, _("while writing block bitmap"));
		exit(1);
	}

	hdr.offset_inodemap = lseek(fd, 0, SEEK_CUR);
	retval = ext2fs_image_bitmap_write(fs, fd, IMAGER_FLAG_INODEMAP);
	if (retval) {
		com_err(program_name, retval, _("while writing inode bitmap"));
		exit(1);
	}

	hdr.magic_number = EXT2_ET_MAGIC_E2IMAGE;
	strcpy(hdr.magic_descriptor, "Ext2 Image 1.0");
	gethostname(hdr.fs_hostname, sizeof(hdr.fs_hostname));
	strncat(hdr.fs_device_name, device_name, sizeof(hdr.fs_device_name));
	hdr.fs_device_name[sizeof(hdr.fs_device_name) - 1] = 0;
	hdr.fs_blocksize = fs->blocksize;
	
	if (stat(device_name, &st) == 0)
		hdr.fs_device = st.st_rdev;

	if (fstat(fd, &st) == 0) {
		hdr.image_device = st.st_dev;
		hdr.image_inode = st.st_ino;
	}
	memcpy(hdr.fs_uuid, fs->super->s_uuid, sizeof(hdr.fs_uuid));

	hdr.image_time = time(0);
	write_header(fd, &hdr, fs->blocksize);
}

/*
 * These set of functions are used to write a RAW image file.
 */
ext2fs_block_bitmap meta_block_map;

struct process_block_struct {
	ext2_ino_t	ino;
};

/*
 * These subroutines short circuits ext2fs_get_blocks and
 * ext2fs_check_directory; we use them since we already have the inode
 * structure, so there's no point in letting the ext2fs library read
 * the inode again.
 */
static ino_t stashed_ino = 0;
static struct ext2_inode *stashed_inode;

static errcode_t meta_get_blocks(ext2_filsys fs, ext2_ino_t ino,
				  blk_t *blocks)
{
	int	i;
	
	if ((ino != stashed_ino) || !stashed_inode)
		return EXT2_ET_CALLBACK_NOTHANDLED;

	for (i=0; i < EXT2_N_BLOCKS; i++)
		blocks[i] = stashed_inode->i_block[i];
	return 0;
}

static errcode_t meta_read_inode(ext2_filsys fs, ext2_ino_t ino,
				 struct ext2_inode *inode)
{
	if ((ino != stashed_ino) || !stashed_inode)
		return EXT2_ET_CALLBACK_NOTHANDLED;
	*inode = *stashed_inode;
	return 0;
}

static int process_dir_block(ext2_filsys fs, blk_t *block_nr,
			     e2_blkcnt_t blockcnt, blk_t ref_block,
			     int ref_offset, void *priv_data)
{
	ext2fs_mark_block_bitmap(meta_block_map, *block_nr);
	return 0;
}

static int process_file_block(ext2_filsys fs, blk_t *block_nr,
			     e2_blkcnt_t blockcnt, blk_t ref_block,
			     int ref_offset, void *priv_data)
{
	if (blockcnt < 0) {
		ext2fs_mark_block_bitmap(meta_block_map, *block_nr);
	}
	return 0;
}

static void mark_table_blocks(ext2_filsys fs)
{
	blk_t	block, b;
	int	i,j;
	
	block = fs->super->s_first_data_block;
	/*
	 * Mark primary superblock
	 */
	ext2fs_mark_block_bitmap(meta_block_map, block);
			
	/*
	 * Mark the primary superblock descriptors
	 */
	for (j = 0; j < fs->desc_blocks; j++) {
		ext2fs_mark_block_bitmap(meta_block_map,
					 block + j + 1);
	}

	for (i = 0; i < fs->group_desc_count; i++) {
		/*
		 * Mark the blocks used for the inode table
		 */
		if (fs->group_desc[i].bg_inode_table) {
			for (j = 0, b = fs->group_desc[i].bg_inode_table;
			     j < fs->inode_blocks_per_group;
			     j++, b++)
				ext2fs_mark_block_bitmap(meta_block_map, b);
		}
			    
		/*
		 * Mark block used for the block bitmap 
		 */
		if (fs->group_desc[i].bg_block_bitmap) {
			ext2fs_mark_block_bitmap(meta_block_map,
				     fs->group_desc[i].bg_block_bitmap);
		}
		
		/*
		 * Mark block used for the inode bitmap 
		 */
		if (fs->group_desc[i].bg_inode_bitmap) {
			ext2fs_mark_block_bitmap(meta_block_map,
				 fs->group_desc[i].bg_inode_bitmap);
		}
		block += fs->super->s_blocks_per_group;
	}
}

/*
 * This function returns 1 if the specified block is all zeros
 */
static int check_zero_block(char *buf, int blocksize)
{
	char	*cp = buf;
	int	left = blocksize;

	while (left > 0) {
		if (*cp++)
			return 0;
		left--;
	}
	return 1;
}

static void write_block(int fd, char *buf, int blocksize, blk_t block)
{
	int		count;
	errcode_t	err;

	count = write(fd, buf, blocksize);
	if (count != blocksize) {
		if (count == -1)
			err = errno;
		else
			err = 0;
		com_err(program_name, err, "error writing block %d", block);
	}
}

static output_meta_data_blocks(ext2_filsys fs, int fd)
{
	errcode_t	retval;
	blk_t		blk;
	char		buf[8192], zero_buf[8192];

	memset(zero_buf, 0, sizeof(zero_buf));
	for (blk = 0; blk < fs->super->s_blocks_count; blk++) {
		if ((blk >= fs->super->s_first_data_block) &&
		    ext2fs_test_block_bitmap(meta_block_map, blk)) {
			retval = io_channel_read_blk(fs->io, blk, 1, buf);
			if (retval) {
				com_err(program_name, retval,
					"error reading block %d", blk);
			}
			if ((fd != 1) && check_zero_block(buf, fs->blocksize))
				goto sparse_write;
			write_block(fd, buf, fs->blocksize, blk);
		} else {
		sparse_write:
			if (fd == 1) {
				write_block(fd, zero_buf, fs->blocksize, blk);
				continue;
			}
#ifdef HAVE_LSEEK64
			if (lseek64(fd, fs->blocksize, SEEK_CUR) < 0)
				perror("lseek");
#else
			if (lseek(fd, fs->blocksize, SEEK_CUR) < 0)
				perror("lseek");
#endif			
		}
	}
	buf[0] = 0;
	write(fd, buf, 1);
}

static void write_raw_image_file(ext2_filsys fs, int fd)
{
	struct process_block_struct	pb;
	struct ext2_inode		inode;
	ext2_inode_scan			scan;
	ext2_ino_t			ino;
	errcode_t			retval;
	char *				block_buf;
	
	retval = ext2fs_allocate_block_bitmap(fs, "in-use block map",
					      &meta_block_map);
	if (retval) {
		com_err(program_name, retval, "while allocating block bitmap");
		exit(1);
	}
	
	mark_table_blocks(fs);

	retval = ext2fs_open_inode_scan(fs, 0, &scan);
	if (retval) {
		com_err(program_name, retval, _("while opening inode scan"));
		exit(1);
	}

	block_buf = malloc(fs->blocksize * 3);
	if (!block_buf) {
		com_err(program_name, 0, "Can't allocate block buffer");
		exit(1);
	}
	
	stashed_inode = &inode;
	while (1) {
		retval = ext2fs_get_next_inode(scan, &ino, &inode);
		if (retval) {
			com_err(program_name, retval,
				_("while getting next inode"));
			exit(1);
		}
		if (ino == 0)
			break;
		if (!inode.i_links_count ||
		    !ext2fs_inode_has_valid_blocks(&inode))
			continue;
		
		stashed_ino = ino;
		if (LINUX_S_ISDIR(inode.i_mode) ||
		    ino == fs->super->s_journal_inum) {
			retval = ext2fs_block_iterate2(fs, ino, 0, 
				       block_buf, process_dir_block, &pb);
			if (retval) {
				com_err(program_name, retval,
					"while iterating over inode %d", 
					ino);
				exit(1);
			}
		} else {
			if (inode.i_block[EXT2_IND_BLOCK] ||
			    inode.i_block[EXT2_DIND_BLOCK] ||
			    inode.i_block[EXT2_TIND_BLOCK]) {
				retval = ext2fs_block_iterate2(fs,
				       ino, 0, block_buf,
				       process_file_block, &pb);
				if (retval) {
					com_err(program_name, retval,
					"while iterating over %d", ino);
					exit(1);
				}
			}
			if (inode.i_file_acl) {
				ext2fs_mark_block_bitmap(meta_block_map,
							 inode.i_file_acl);
			}
		}
	}
	
	output_meta_data_blocks(fs, fd);
}

int main (int argc, char ** argv)
{
	int c;
	errcode_t retval;
	ext2_filsys fs;
	char *outfn;
	int open_flag = 0;
	int raw_flag = 0;
	int fd = 0;

#ifdef ENABLE_NLS
	setlocale(LC_MESSAGES, "");
	bindtextdomain(NLS_CAT_NAME, LOCALEDIR);
	textdomain(NLS_CAT_NAME);
#endif
	fprintf (stderr, "e2image %s (%s)\n", E2FSPROGS_VERSION,
		 E2FSPROGS_DATE);
	if (argc && *argv)
		program_name = *argv;
	initialize_ext2_error_table();
	while ((c = getopt (argc, argv, "r")) != EOF)
		switch (c) {
		case 'r':
			raw_flag++;
			break;
		default:
			usage();
		}
	if (optind != argc - 2 )
		usage();
	device_name = argv[optind];
	outfn = argv[optind+1];
	retval = ext2fs_open (device_name, open_flag, 0, 0,
			      unix_io_manager, &fs);
        if (retval) {
		com_err (program_name, retval, _("while trying to open %s"),
			 device_name);
		printf(_("Couldn't find valid filesystem superblock.\n"));
		exit(1);
	}

	if (strcmp(outfn, "-") == 0)
		fd = 1;
	else {
#ifdef HAVE_OPEN64
		fd = open64(outfn, O_CREAT|O_TRUNC|O_WRONLY, 0600);
#else
		fd = open(outfn, O_CREAT|O_TRUNC|O_WRONLY, 0600);
#endif
		if (fd < 0) {
			com_err(program_name, errno,
				_("while trying to open %s"), argv[optind+1]);
			exit(1);
		}
	}

	if (raw_flag)
		write_raw_image_file(fs, fd);
	else
		write_image_file(fs, fd);

	ext2fs_close (fs);
	exit (0);
}
