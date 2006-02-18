/*
 * debugfs.c --- a program which allows you to attach an ext2fs
 * filesystem and play with it.
 *
 * Copyright (C) 1993 Theodore Ts'o.  This file may be redistributed
 * under the terms of the GNU Public License.
 * 
 * Modifications by Robert Sanders <gt8134b@prism.gatech.edu>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else 
extern int optind;
extern char *optarg;
#endif
#ifdef HAVE_OPTRESET
extern int optreset;		/* defined by BSD, but not others */
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "et/com_err.h"
#include "ss/ss.h"
#include "debugfs.h"
#include "uuid/uuid.h"
#include "e2p/e2p.h"

#include "../version.h"

extern ss_request_table debug_cmds;

ext2_filsys	current_fs = NULL;
ext2_ino_t	root, cwd;

static void open_filesystem(char *device, int open_flags, blk_t superblock,
			    blk_t blocksize, int catastrophic)
{
	int	retval;

	if (superblock != 0 && blocksize == 0) {
		com_err(device, 0, "if you specify the superblock, you must also specify the block size");
		current_fs = NULL;
		return;
	}

	if (catastrophic && (open_flags & EXT2_FLAG_RW)) {
		com_err(device, 0,
			"opening read-only because of catastrophic mode");
		open_flags &= ~EXT2_FLAG_RW;
	}
	
	retval = ext2fs_open(device, open_flags, superblock, blocksize,
			     unix_io_manager, &current_fs);
	if (retval) {
		com_err(device, retval, "while opening filesystem");
		current_fs = NULL;
		return;
	}

	if (catastrophic)
		com_err(device, 0, "catastrophic mode - not reading inode or group bitmaps");
	else {
		retval = ext2fs_read_inode_bitmap(current_fs);
		if (retval) {
			com_err(device, retval, "while reading inode bitmap");
			goto errout;
		}
		retval = ext2fs_read_block_bitmap(current_fs);
		if (retval) {
			com_err(device, retval, "while reading block bitmap");
			goto errout;
		}
	}
	root = cwd = EXT2_ROOT_INO;
	return;

errout:
	retval = ext2fs_close(current_fs);
	if (retval)
		com_err(device, retval, "while trying to close filesystem");
	current_fs = NULL;
}

void do_open_filesys(int argc, char **argv)
{
	const char *usage = "Usage: open [-s superblock] [-b blocksize] [-c] [-w] <device>";
	int	c;
	int	catastrophic = 0;
	blk_t	superblock = 0;
	blk_t	blocksize = 0;
	char	*tmp;
	int open_flags = 0;
	
	optind = 0;
#ifdef HAVE_OPTRESET
	optreset = 1;		/* Makes BSD getopt happy */
#endif
	while ((c = getopt (argc, argv, "iwfcb:s:")) != EOF) {
		switch (c) {
		case 'i':
			open_flags |= EXT2_FLAG_IMAGE_FILE;
			break;
		case 'w':
			open_flags |= EXT2_FLAG_RW;
			break;
		case 'f':
			open_flags |= EXT2_FLAG_FORCE;
			break;
		case 'c':
			catastrophic = 1;
			break;
		case 'b':
			blocksize = strtoul(optarg, &tmp, 0);
			if (*tmp) {
				com_err(argv[0], 0,
					"Bad block size - %s", optarg);
				return;
			}
			break;
		case 's':
			superblock = strtoul(optarg, &tmp, 0);
			if (*tmp) {
				com_err(argv[0], 0,
					"Bad superblock number - %s", optarg);
				return;
			}
			break;
		default:
			com_err(argv[0], 0, usage);
			return;
		}
	}
	if (optind != argc-1) {
		com_err(argv[0], 0, usage);
		return;
	}
	if (check_fs_not_open(argv[0]))
		return;
	open_filesystem(argv[optind], open_flags,
			superblock, blocksize, catastrophic);
}

void do_lcd(int argc, char **argv)
{
	const char *usage = "Usage: lcd <native dir>";

	if (argc != 2) {
		com_err(argv[0], 0, usage);
		return;
	}

	if (chdir(argv[1]) == -1) {
		com_err(argv[0], errno,
			"while trying to change native directory to %s",
			argv[1]);
		return;
	}
}

static void close_filesystem(NOARGS)
{
	int	retval;
	
	if (current_fs->flags & EXT2_FLAG_IB_DIRTY) {
		retval = ext2fs_write_inode_bitmap(current_fs);
		if (retval)
			com_err("ext2fs_write_inode_bitmap", retval, "");
	}
	if (current_fs->flags & EXT2_FLAG_BB_DIRTY) {
		retval = ext2fs_write_block_bitmap(current_fs);
		if (retval)
			com_err("ext2fs_write_block_bitmap", retval, "");
	}
	retval = ext2fs_close(current_fs);
	if (retval)
		com_err("ext2fs_close", retval, "");
	current_fs = NULL;
	return;
}

void do_close_filesys(int argc, char **argv)
{
	if (argc > 1) {
		com_err(argv[0], 0, "Usage: close_filesys");
		return;
	}
	if (check_fs_open(argv[0]))
		return;
	close_filesystem();
}

void do_init_filesys(int argc, char **argv)
{
	const char	*usage = "Usage: initialize <device> <blocksize>";
	struct ext2_super_block param;
	errcode_t	retval;
	char		*tmp;
	
	if (argc != 3) {
		com_err(argv[0], 0, usage);
		return;
	}
	if (check_fs_not_open(argv[0]))
		return;

	memset(&param, 0, sizeof(struct ext2_super_block));
	param.s_blocks_count = strtoul(argv[2], &tmp, 0);
	if (*tmp) {
		com_err(argv[0], 0, "Bad blocks count - %s", argv[2]);
		return;
	}
	retval = ext2fs_initialize(argv[1], 0, &param,
				   unix_io_manager, &current_fs);
	if (retval) {
		com_err(argv[1], retval, "while initializing filesystem");
		current_fs = NULL;
		return;
	}
	root = cwd = EXT2_ROOT_INO;
	return;
}

static void print_features(struct ext2_super_block * s, FILE *f)
{
	int	i, j, printed=0;
	__u32	*mask = &s->s_feature_compat, m;

	fputs("Filesystem features:", f);
	for (i=0; i <3; i++,mask++) {
		for (j=0,m=1; j < 32; j++, m<<=1) {
			if (*mask & m) {
				fprintf(f, " %s", e2p_feature2string(i, m));
				printed++;
			}
		}
	}
	if (printed == 0)
		fputs("(none)", f);
	fputs("\n", f);
}

void do_show_super_stats(int argc, char *argv[])
{
	int	i;
	FILE 	*out;
	struct ext2_group_desc *gdp;
	int	c, header_only = 0;
	const char *usage = "Usage: show_super [-h]";

	optind = 0;
#ifdef HAVE_OPTRESET
	optreset = 1;		/* Makes BSD getopt happy */
#endif
	while ((c = getopt (argc, argv, "h")) != EOF) {
		switch (c) {
		case 'h':
			header_only++;
			break;
		default:
			com_err(argv[0], 0, usage);
			return;
		}
	}
	if (optind != argc) {
		com_err(argv[0], 0, usage);
		return;
	}
	if (check_fs_open(argv[0]))
		return;
	out = open_pager();

	list_super2(current_fs->super, out);

	if (header_only) {
		close_pager(out);
		return;
	}
	
	gdp = &current_fs->group_desc[0];
	for (i = 0; i < current_fs->group_desc_count; i++, gdp++)
		fprintf(out, " Group %2d: block bitmap at %d, "
		        "inode bitmap at %d, "
		        "inode table at %d\n"
		        "           %d free %s, "
		        "%d free %s, "
		        "%d used %s\n",
		        i, gdp->bg_block_bitmap,
		        gdp->bg_inode_bitmap, gdp->bg_inode_table,
		        gdp->bg_free_blocks_count,
		        gdp->bg_free_blocks_count != 1 ? "blocks" : "block",
		        gdp->bg_free_inodes_count,
		        gdp->bg_free_inodes_count != 1 ? "inodes" : "inode",
		        gdp->bg_used_dirs_count,
		        gdp->bg_used_dirs_count != 1 ? "directories"
				: "directory");
	close_pager(out);
}

void do_dirty_filesys(int argc, char **argv)
{
	if (check_fs_open(argv[0]))
		return;
	if (check_fs_read_write(argv[0]))
		return;

	if (argv[1] && !strcmp(argv[1], "-clean"))
		current_fs->super->s_state |= EXT2_VALID_FS;
	else
		current_fs->super->s_state &= ~EXT2_VALID_FS;
	ext2fs_mark_super_dirty(current_fs);
}

struct list_blocks_struct {
	FILE	*f;
	int	total;
	blk_t	first_block, last_block;
	int	first_bcnt, last_bcnt;
	int	first;
};

static void finish_range(struct list_blocks_struct *lb)
{
	if (lb->first_block == 0)
		return;
	if (lb->first)
		lb->first = 0;
	else
		fprintf(lb->f, ", ");
	if (lb->first_block == lb->last_block)
		fprintf(lb->f, "(%d):%d", lb->first_bcnt, lb->first_block);
	else
		fprintf(lb->f, "(%d-%d):%d-%d", lb->first_bcnt,
			lb->last_bcnt, lb->first_block, lb->last_block);
	lb->first_block = 0;
}

static int list_blocks_proc(ext2_filsys fs, blk_t *blocknr, int blockcnt,
			    void *private)
{
	struct list_blocks_struct *lb = (struct list_blocks_struct *) private;

	lb->total++;
	if (blockcnt >= 0) {
		/*
		 * See if we can add on to the existing range (if it exists)
		 */
		if (lb->first_block &&
		    (lb->last_block+1 == *blocknr) &&
		    (lb->last_bcnt+1 == blockcnt)) {
			lb->last_block = *blocknr;
			lb->last_bcnt = blockcnt;
			return 0;
		}
		/*
		 * Start a new range.
		 */
		finish_range(lb);
		lb->first_block = lb->last_block = *blocknr;
		lb->first_bcnt = lb->last_bcnt = blockcnt;
		return 0;
	}
	/*
	 * Not a normal block.  Always force a new range.
	 */
	finish_range(lb);
	if (lb->first)
		lb->first = 0;
	else
		fprintf(lb->f, ", ");
	if (blockcnt == -1)
		fprintf(lb->f, "(IND):%d", *blocknr);
	else if (blockcnt == -2)
		fprintf(lb->f, "(DIND):%d", *blocknr);
	else if (blockcnt == -3)
		fprintf(lb->f, "(TIND):%d", *blocknr);
	return 0;
}


static void dump_blocks(FILE *f, const char *prefix, ext2_ino_t inode)
{
	struct list_blocks_struct lb;

	fprintf(f, "%sBLOCKS:\n%s", prefix, prefix);
	lb.total = 0;
	lb.first_block = 0;
	lb.f = f;
	lb.first = 1;
	ext2fs_block_iterate(current_fs, inode, 0, NULL,
			     list_blocks_proc, (void *)&lb);
	finish_range(&lb);
	if (lb.total)
		fprintf(f, "\n%sTOTAL: %d\n", prefix, lb.total);
	fprintf(f,"\n");
}


void internal_dump_inode(FILE *out, const char *prefix,
			 ext2_ino_t inode_num, struct ext2_inode *inode,
			 int do_dump_blocks)
{
	const char *i_type;
	char frag, fsize;
	int os = current_fs->super->s_creator_os;
	
	if (LINUX_S_ISDIR(inode->i_mode)) i_type = "directory";
	else if (LINUX_S_ISREG(inode->i_mode)) i_type = "regular";
	else if (LINUX_S_ISLNK(inode->i_mode)) i_type = "symlink";
	else if (LINUX_S_ISBLK(inode->i_mode)) i_type = "block special";
	else if (LINUX_S_ISCHR(inode->i_mode)) i_type = "character special";
	else if (LINUX_S_ISFIFO(inode->i_mode)) i_type = "FIFO";
	else if (LINUX_S_ISSOCK(inode->i_mode)) i_type = "socket";
	else i_type = "bad type";
	fprintf(out, "%sInode: %u   Type: %s    ", prefix, inode_num, i_type);
	fprintf(out, "%sMode:  %04o   Flags: 0x%x   Generation: %u\n",
		prefix, 
		inode->i_mode & 0777, inode->i_flags, inode->i_generation);
	fprintf(out, "%sUser: %5d   Group: %5d   Size: ",
		prefix, inode->i_uid, inode->i_gid);
	if (LINUX_S_ISDIR(inode->i_mode))
		fprintf(out, "%d\n", inode->i_size);
	else {
		__u64 i_size = (inode->i_size |
				((unsigned long long)inode->i_size_high << 32));
		
		fprintf(out, "%lld\n", i_size);
	}
	if (current_fs->super->s_creator_os == EXT2_OS_HURD)
		fprintf(out,
			"%sFile ACL: %d    Directory ACL: %d Translator: %d\n",
			prefix,
			inode->i_file_acl, LINUX_S_ISDIR(inode->i_mode) ? inode->i_dir_acl : 0,
			inode->osd1.hurd1.h_i_translator);
	else
		fprintf(out, "%sFile ACL: %d    Directory ACL: %d\n",
			prefix,
			inode->i_file_acl, LINUX_S_ISDIR(inode->i_mode) ? inode->i_dir_acl : 0);
	fprintf(out, "%sLinks: %d   Blockcount: %d\n", 
		prefix, inode->i_links_count, inode->i_blocks);
	switch (os) {
	    case EXT2_OS_LINUX:
		frag = inode->osd2.linux2.l_i_frag;
		fsize = inode->osd2.linux2.l_i_fsize;
		break;
	    case EXT2_OS_HURD:
		frag = inode->osd2.hurd2.h_i_frag;
		fsize = inode->osd2.hurd2.h_i_fsize;
		break;
	    case EXT2_OS_MASIX:
		frag = inode->osd2.masix2.m_i_frag;
		fsize = inode->osd2.masix2.m_i_fsize;
		break;
	    default:
		frag = fsize = 0;
	}
	fprintf(out, "%sFragment:  Address: %d    Number: %d    Size: %d\n",
		prefix, inode->i_faddr, frag, fsize);
	fprintf(out, "%sctime: 0x%08x -- %s", prefix, inode->i_ctime,
		time_to_string(inode->i_ctime));
	fprintf(out, "%satime: 0x%08x -- %s", prefix, inode->i_atime,
		time_to_string(inode->i_atime));
	fprintf(out, "%smtime: 0x%08x -- %s", prefix, inode->i_mtime,
		time_to_string(inode->i_mtime));
	if (inode->i_dtime) 
	  fprintf(out, "%sdtime: 0x%08x -- %s", prefix, inode->i_dtime,
		  time_to_string(inode->i_dtime));
	if (LINUX_S_ISLNK(inode->i_mode) && inode->i_blocks == 0)
		fprintf(out, "%sFast_link_dest: %.*s\n", prefix,
			(int) inode->i_size, (char *)inode->i_block);
	else if (do_dump_blocks)
		dump_blocks(out, prefix, inode_num);
}

static void dump_inode(ext2_ino_t inode_num, struct ext2_inode inode)
{
	FILE	*out;
	
	out = open_pager();
	internal_dump_inode(out, "", inode_num, &inode, 1);
	close_pager(out);
}


void do_stat(int argc, char *argv[])
{
	ext2_ino_t	inode;
	struct ext2_inode inode_buf;
	int 		retval;

	if (argc != 2) {
		com_err(argv[0], 0, "Usage: stat <file>");
		return;
	}
	if (check_fs_open(argv[0]))
		return;
	inode = string_to_inode(argv[1]);
	if (!inode) 
		return;

	retval = ext2fs_read_inode(current_fs, inode, &inode_buf);
	if (retval) 
	  {
	    com_err(argv[0], retval, "while trying to read inode %d", inode);
	    return;
	  }

	dump_inode(inode,inode_buf);
	return;
}

void do_chroot(int argc, char *argv[])
{
	ext2_ino_t inode;
	int retval;

	if (argc != 2) {
		com_err(argv[0], 0, "Usage: chroot <file>");
		return;
	}
	if (check_fs_open(argv[0]))
		return;
	inode = string_to_inode(argv[1]);
	if (!inode) 
		return;

	retval = ext2fs_check_directory(current_fs, inode);
	if (retval)  {
		com_err(argv[1], retval, "");
		return;
	}
	root = inode;
}

void do_clri(int argc, char *argv[])
{
	ext2_ino_t inode;
	int retval;
	struct ext2_inode inode_buf;

	if (argc != 2) {
		com_err(argv[0], 0, "Usage: clri <file>");
		return;
	}
	if (check_fs_open(argv[0]))
		return;
	if (check_fs_read_write(argv[0]))
		return;
	inode = string_to_inode(argv[1]);
	if (!inode) 
		return;

	retval = ext2fs_read_inode(current_fs, inode, &inode_buf);
	if (retval) {
		com_err(argv[0], retval, "while trying to read inode %d", 
			inode);
		return;
	}
	memset(&inode_buf, 0, sizeof(inode_buf));
	retval = ext2fs_write_inode(current_fs, inode, &inode_buf);
	if (retval) {
		com_err(argv[0], retval, "while trying to write inode %d",
			inode);
		return;
	}
}

void do_freei(int argc, char *argv[])
{
	ext2_ino_t inode;

	if (argc != 2) {
		com_err(argv[0], 0, "Usage: freei <file>");
		return;
	}
	if (check_fs_open(argv[0]))
		return;
	if (check_fs_read_write(argv[0]))
		return;
	inode = string_to_inode(argv[1]);
	if (!inode) 
		return;

	if (!ext2fs_test_inode_bitmap(current_fs->inode_map,inode))
		com_err(argv[0], 0, "Warning: inode already clear");
	ext2fs_unmark_inode_bitmap(current_fs->inode_map,inode);
	ext2fs_mark_ib_dirty(current_fs);
}

void do_seti(int argc, char *argv[])
{
	ext2_ino_t inode;

	if (argc != 2) {
		com_err(argv[0], 0, "Usage: seti <file>");
		return;
	}
	if (check_fs_open(argv[0]))
		return;
	if (check_fs_read_write(argv[0]))
		return;
	inode = string_to_inode(argv[1]);
	if (!inode) 
		return;

	if (ext2fs_test_inode_bitmap(current_fs->inode_map,inode))
		com_err(argv[0], 0, "Warning: inode already set");
	ext2fs_mark_inode_bitmap(current_fs->inode_map,inode);
	ext2fs_mark_ib_dirty(current_fs);
}

void do_testi(int argc, char *argv[])
{
	ext2_ino_t inode;

	if (argc != 2) {
		com_err(argv[0], 0, "Usage: testi <file>");
		return;
	}
	if (check_fs_open(argv[0]))
		return;
	if (check_fs_bitmaps(argv[0]))
		return;
	inode = string_to_inode(argv[1]);
	if (!inode) 
		return;

	if (ext2fs_test_inode_bitmap(current_fs->inode_map,inode))
		printf("Inode %u is marked in use\n", inode);
	else
		printf("Inode %u is not in use\n", inode);
}


void do_freeb(int argc, char *argv[])
{
	blk_t block;
	char *tmp;

	if (argc != 2) {
		com_err(argv[0], 0, "Usage: freeb <block>");
		return;
	}
	if (check_fs_open(argv[0]))
		return;
	if (check_fs_read_write(argv[0]))
		return;
	block = strtoul(argv[1], &tmp, 0);
	if (!block || *tmp) {
		com_err(argv[0], 0, "No block 0");
		return;
	} 
	if (!ext2fs_test_block_bitmap(current_fs->block_map,block))
		com_err(argv[0], 0, "Warning: block already clear");
	ext2fs_unmark_block_bitmap(current_fs->block_map,block);
	ext2fs_mark_bb_dirty(current_fs);
}

void do_setb(int argc, char *argv[])
{
	blk_t block;
	char *tmp;

	if (argc != 2) {
		com_err(argv[0], 0, "Usage: setb <block>");
		return;
	}
	if (check_fs_open(argv[0]))
		return;
	if (check_fs_read_write(argv[0]))
		return;
	block = strtoul(argv[1], &tmp, 0);
	if (!block || *tmp) {
		com_err(argv[0], 0, "No block 0");
		return;
	} 
	if (ext2fs_test_block_bitmap(current_fs->block_map,block))
		com_err(argv[0], 0, "Warning: block already set");
	ext2fs_mark_block_bitmap(current_fs->block_map,block);
	ext2fs_mark_bb_dirty(current_fs);
}

void do_testb(int argc, char *argv[])
{
	blk_t block;
	char *tmp;

	if (argc != 2) {
		com_err(argv[0], 0, "Usage: testb <block>");
		return;
	}
	if (check_fs_open(argv[0]))
		return;
	if (check_fs_bitmaps(argv[0]))
		return;
	block = strtoul(argv[1], &tmp, 0);
	if (!block || *tmp) {
		com_err(argv[0], 0, "No block 0");
		return;
	} 
	if (ext2fs_test_block_bitmap(current_fs->block_map,block))
		printf("Block %d marked in use\n", block);
	else printf("Block %d not in use\n", block);
}

static void modify_u8(char *com, const char *prompt,
		      const char *format, __u8 *val)
{
	char buf[200];
	unsigned long v;
	char *tmp;

	sprintf(buf, format, *val);
	printf("%30s    [%s] ", prompt, buf);
	fgets(buf, sizeof(buf), stdin);
	if (buf[strlen (buf) - 1] == '\n')
		buf[strlen (buf) - 1] = '\0';
	if (!buf[0])
		return;
	v = strtoul(buf, &tmp, 0);
	if (*tmp)
		com_err(com, 0, "Bad value - %s", buf);
	else
		*val = v;
}

static void modify_u16(char *com, const char *prompt,
		       const char *format, __u16 *val)
{
	char buf[200];
	unsigned long v;
	char *tmp;

	sprintf(buf, format, *val);
	printf("%30s    [%s] ", prompt, buf);
	fgets(buf, sizeof(buf), stdin);
	if (buf[strlen (buf) - 1] == '\n')
		buf[strlen (buf) - 1] = '\0';
	if (!buf[0])
		return;
	v = strtoul(buf, &tmp, 0);
	if (*tmp)
		com_err(com, 0, "Bad value - %s", buf);
	else
		*val = v;
}

static void modify_u32(char *com, const char *prompt,
		       const char *format, __u32 *val)
{
	char buf[200];
	unsigned long v;
	char *tmp;

	sprintf(buf, format, *val);
	printf("%30s    [%s] ", prompt, buf);
	fgets(buf, sizeof(buf), stdin);
	if (buf[strlen (buf) - 1] == '\n')
		buf[strlen (buf) - 1] = '\0';
	if (!buf[0])
		return;
	v = strtoul(buf, &tmp, 0);
	if (*tmp)
		com_err(com, 0, "Bad value - %s", buf);
	else
		*val = v;
}


void do_modify_inode(int argc, char *argv[])
{
	struct ext2_inode inode;
	ext2_ino_t	inode_num;
	int 		i;
	errcode_t	retval;
	unsigned char	*frag, *fsize;
	char		buf[80];
	int 		os = current_fs->super->s_creator_os;
	const char	*hex_format = "0x%x";
	const char	*octal_format = "0%o";
	const char	*decimal_format = "%d";
	
	if (argc != 2) {
		com_err(argv[0], 0, "Usage: modify_inode <file>");
		return;
	}
	if (check_fs_open(argv[0]))
		return;
	if (check_fs_read_write(argv[0]))
		return;

	inode_num = string_to_inode(argv[1]);
	if (!inode_num) 
		return;

	retval = ext2fs_read_inode(current_fs, inode_num, &inode);
	if (retval) {
		com_err(argv[1], retval, "while trying to read inode %d",
			inode_num);
		return;
	}
	
	modify_u16(argv[0], "Mode", octal_format, &inode.i_mode);
	modify_u16(argv[0], "User ID", decimal_format, &inode.i_uid);
	modify_u16(argv[0], "Group ID", decimal_format, &inode.i_gid);
	modify_u32(argv[0], "Size", decimal_format, &inode.i_size);
	modify_u32(argv[0], "Creation time", decimal_format, &inode.i_ctime);
	modify_u32(argv[0], "Modification time", decimal_format, &inode.i_mtime);
	modify_u32(argv[0], "Access time", decimal_format, &inode.i_atime);
	modify_u32(argv[0], "Deletion time", decimal_format, &inode.i_dtime);
	modify_u16(argv[0], "Link count", decimal_format, &inode.i_links_count);
	modify_u32(argv[0], "Block count", decimal_format, &inode.i_blocks);
	modify_u32(argv[0], "File flags", hex_format, &inode.i_flags);
	modify_u32(argv[0], "Generation", hex_format, &inode.i_generation);
#if 0
	modify_u32(argv[0], "Reserved1", decimal_format, &inode.i_reserved1);
#endif
	modify_u32(argv[0], "File acl", decimal_format, &inode.i_file_acl);
	if (LINUX_S_ISDIR(inode.i_mode))
		modify_u32(argv[0], "Directory acl", decimal_format, &inode.i_dir_acl);
	else
		modify_u32(argv[0], "High 32bits of size", decimal_format, &inode.i_size_high);

	if (current_fs->super->s_creator_os == EXT2_OS_HURD)
		modify_u32(argv[0], "Translator Block",
			    decimal_format, &inode.osd1.hurd1.h_i_translator);
	
	modify_u32(argv[0], "Fragment address", decimal_format, &inode.i_faddr);
	switch (os) {
	    case EXT2_OS_LINUX:
		frag = &inode.osd2.linux2.l_i_frag;
		fsize = &inode.osd2.linux2.l_i_fsize;
		break;
	    case EXT2_OS_HURD:
		frag = &inode.osd2.hurd2.h_i_frag;
		fsize = &inode.osd2.hurd2.h_i_fsize;
		break;
	    case EXT2_OS_MASIX:
		frag = &inode.osd2.masix2.m_i_frag;
		fsize = &inode.osd2.masix2.m_i_fsize;
		break;
	    default:
		frag = fsize = 0;
	}
	if (frag)
		modify_u8(argv[0], "Fragment number", decimal_format, frag);
	if (fsize)
		modify_u8(argv[0], "Fragment size", decimal_format, fsize);

	for (i=0;  i < EXT2_NDIR_BLOCKS; i++) {
		sprintf(buf, "Direct Block #%d", i);
		modify_u32(argv[0], buf, decimal_format, &inode.i_block[i]);
	}
	modify_u32(argv[0], "Indirect Block", decimal_format,
		    &inode.i_block[EXT2_IND_BLOCK]);    
	modify_u32(argv[0], "Double Indirect Block", decimal_format,
		    &inode.i_block[EXT2_DIND_BLOCK]);
	modify_u32(argv[0], "Triple Indirect Block", decimal_format,
		    &inode.i_block[EXT2_TIND_BLOCK]);
	retval = ext2fs_write_inode(current_fs, inode_num, &inode);
	if (retval) {
		com_err(argv[1], retval, "while trying to write inode %d",
			inode_num);
		return;
	}
}

void do_change_working_dir(int argc, char *argv[])
{
	ext2_ino_t	inode;
	int		retval;
	
	if (argc != 2) {
		com_err(argv[0], 0, "Usage: cd <file>");
		return;
	}
	if (check_fs_open(argv[0]))
		return;

	inode = string_to_inode(argv[1]);
	if (!inode) 
		return;

	retval = ext2fs_check_directory(current_fs, inode);
	if (retval) {
		com_err(argv[1], retval, "");
		return;
	}
	cwd = inode;
	return;
}

void do_print_working_directory(int argc, char *argv[])
{
	int	retval;
	char	*pathname = NULL;
	
	if (argc > 1) {
		com_err(argv[0], 0, "Usage: print_working_directory");
		return;
	}
	if (check_fs_open(argv[0]))
		return;

	retval = ext2fs_get_pathname(current_fs, cwd, 0, &pathname);
	if (retval) {
		com_err(argv[0], retval,
			"while trying to get pathname of cwd");
	}
	printf("[pwd]   INODE: %6u  PATH: %s\n", cwd, pathname);
	free(pathname);
	retval = ext2fs_get_pathname(current_fs, root, 0, &pathname);
	if (retval) {
		com_err(argv[0], retval,
			"while trying to get pathname of root");
	}
	printf("[root]  INODE: %6u  PATH: %s\n", root, pathname);
	free(pathname);
	return;
}

static void make_link(char *sourcename, char *destname)
{
	ext2_ino_t	inode;
	int		retval;
	ext2_ino_t	dir;
	char		*dest, *cp, *basename;

	/*
	 * Get the source inode
	 */
	inode = string_to_inode(sourcename);
	if (!inode)
		return;
	basename = strrchr(sourcename, '/');
	if (basename)
		basename++;
	else
		basename = sourcename;
	/*
	 * Figure out the destination.  First see if it exists and is
	 * a directory.  
	 */
	if (! (retval=ext2fs_namei(current_fs, root, cwd, destname, &dir)))
		dest = basename;
	else {
		/*
		 * OK, it doesn't exist.  See if it is
		 * '<dir>/basename' or 'basename'
		 */
		cp = strrchr(destname, '/');
		if (cp) {
			*cp = 0;
			dir = string_to_inode(destname);
			if (!dir)
				return;
			dest = cp+1;
		} else {
			dir = cwd;
			dest = destname;
		}
	}
	
	retval = ext2fs_link(current_fs, dir, dest, inode, 0);
	if (retval)
		com_err("make_link", retval, "");
	return;
}


void do_link(int argc, char *argv[])
{
	if (argc != 3) {
		com_err(argv[0], 0, "Usage: link <source_file> <dest_name>");
		return;
	}
	if (check_fs_open(argv[0]))
		return;

	make_link(argv[1], argv[2]);
}

static void unlink_file_by_name(char *filename)
{
	int		retval;
	ext2_ino_t	dir;
	char		*basename;
	
	basename = strrchr(filename, '/');
	if (basename) {
		*basename++ = '\0';
		dir = string_to_inode(filename);
		if (!dir)
			return;
	} else {
		dir = cwd;
		basename = filename;
	}
	retval = ext2fs_unlink(current_fs, dir, basename, 0, 0);
	if (retval)
		com_err("unlink_file_by_name", retval, "");
	return;
}

void do_unlink(int argc, char *argv[])
{
	if (argc != 2) {
		com_err(argv[0], 0, "Usage: unlink <pathname>");
		return;
	}
	if (check_fs_open(argv[0]))
		return;

	unlink_file_by_name(argv[1]);
}

void do_find_free_block(int argc, char *argv[])
{
	blk_t	free_blk, goal;
	errcode_t	retval;
	char		*tmp;
	
	if ((argc > 2) || (argc==2 && *argv[1] == '?')) {
		com_err(argv[0], 0, "Usage: find_free_block [goal]");
		return;
	}
	if (check_fs_open(argv[0]))
		return;

	if (argc > 1) {
		goal = strtol(argv[1], &tmp, 0);
		if (*tmp) {
			com_err(argv[0], 0, "Bad goal - %s", argv[1]);
			return;
		}
	}
	else
		goal = current_fs->super->s_first_data_block;

	retval = ext2fs_new_block(current_fs, goal, 0, &free_blk);
	if (retval)
		com_err("ext2fs_new_block", retval, "");
	else
		printf("Free block found: %d\n", free_blk);

}

void do_find_free_inode(int argc, char *argv[])
{
	ext2_ino_t	free_inode, dir;
	int		mode;
	int		retval;
	char		*tmp;
	
	if (argc > 3 || (argc>1 && *argv[1] == '?')) {
		com_err(argv[0], 0, "Usage: find_free_inode [dir] [mode]");
		return;
	}
	if (check_fs_open(argv[0]))
		return;

	if (argc > 1) {
		dir = strtol(argv[1], &tmp, 0);
		if (*tmp) {
			com_err(argv[0], 0, "Bad dir - %s", argv[1]);
			return;
		}
	}
	else
		dir = root;
	if (argc > 2) {
		mode = strtol(argv[2], &tmp, 0);
		if (*tmp) {
			com_err(argv[0], 0, "Bad mode - %s", argv[2]);
			return;
		}
	} else
		mode = 010755;

	retval = ext2fs_new_inode(current_fs, dir, mode, 0, &free_inode);
	if (retval)
		com_err("ext2fs_new_inode", retval, "");
	else
		printf("Free inode found: %u\n", free_inode);
}

static errcode_t copy_file(int fd, ext2_ino_t newfile)
{
	ext2_file_t	e2_file;
	errcode_t	retval;
	int		got;
	unsigned int	written;
	char		buf[8192];
	char		*ptr;

	retval = ext2fs_file_open(current_fs, newfile,
				  EXT2_FILE_WRITE, &e2_file);
	if (retval)
		return retval;

	while (1) {
		got = read(fd, buf, sizeof(buf));
		if (got == 0)
			break;
		if (got < 0) {
			retval = errno;
			goto fail;
		}
		ptr = buf;
		while (got > 0) {
			retval = ext2fs_file_write(e2_file, ptr,
						   got, &written);
			if (retval)
				goto fail;

			got -= written;
			ptr += written;
		}
	}
	retval = ext2fs_file_close(e2_file);

	return retval;

fail:
	(void) ext2fs_file_close(e2_file);
	return retval;
}


void do_write(int argc, char *argv[])
{
	int		fd;
	struct stat	statbuf;
	ext2_ino_t	newfile;
	errcode_t	retval;
	struct ext2_inode inode;
	dgrp_t		group;

	if (check_fs_open(argv[0]))
		return;
	if (argc != 3) {
		com_err(argv[0], 0, "Usage: write <nativefile> <newfile>");
		return;
	}
	if (check_fs_read_write(argv[0]))
		return;
	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		com_err(argv[1], fd, "");
		return;
	}
	if (fstat(fd, &statbuf) < 0) {
		com_err(argv[1], errno, "");
		close(fd);
		return;
	}

	retval = ext2fs_new_inode(current_fs, cwd, 010755, 0, &newfile);
	if (retval) {
		com_err(argv[0], retval, "");
		close(fd);
		return;
	}
	group = ext2fs_group_of_ino(current_fs, newfile);
	current_fs->group_desc[group].bg_free_inodes_count--;
	current_fs->super->s_free_inodes_count--;
	ext2fs_mark_super_dirty(current_fs);
	printf("Allocated inode: %u\n", newfile);
	retval = ext2fs_link(current_fs, cwd, argv[2], newfile,
			     EXT2_FT_REG_FILE);
	if (retval) {
		com_err(argv[2], retval, "");
		close(fd);
		return;
	}
        if (ext2fs_test_inode_bitmap(current_fs->inode_map,newfile))
		com_err(argv[0], 0, "Warning: inode already set");
	ext2fs_mark_inode_bitmap(current_fs->inode_map,newfile);
	ext2fs_mark_ib_dirty(current_fs);
	memset(&inode, 0, sizeof(inode));
	inode.i_mode = statbuf.st_mode;
	inode.i_atime = inode.i_ctime = inode.i_mtime = time(NULL);
	inode.i_links_count = 1;
	inode.i_size = statbuf.st_size;
	retval = ext2fs_write_inode(current_fs, newfile, &inode);
	if (retval) {
		com_err(argv[0], retval, "while trying to write inode %d", 
			inode);
		close(fd);
		return;
	}
	if (LINUX_S_ISREG(inode.i_mode)) {
		retval = copy_file(fd, newfile);
		if (retval)
			com_err("copy_file", retval, "");
	}
	close(fd);
}

void do_mknod(int argc, char *argv[])
{
	unsigned long	mode, major, minor, nr;
	ext2_ino_t	newfile;
	errcode_t 	retval;
	struct ext2_inode inode;
	int		filetype;

	if (check_fs_open(argv[0]))
		return;
	if (argc < 3 || argv[2][1]) {
		com_err(argv[0], 0, "Usage: mknod <name> [p| [c|b] <major> <minor>]");
		return;
	}
	mode = minor = major = 0;
	switch (argv[2][0]) {
		case 'p':
			mode = LINUX_S_IFIFO;
			filetype = EXT2_FT_FIFO;
			nr = 3;
			break;
		case 'c':
			mode = LINUX_S_IFCHR;
			filetype = EXT2_FT_CHRDEV;
			nr = 5;
			break;
		case 'b':
			mode = LINUX_S_IFBLK;
			filetype = EXT2_FT_BLKDEV;
			nr = 5;
			break;
		default:
			filetype = 0;
			nr = 0;
	}
	if (nr == 5) {
		major = strtoul(argv[3], argv+3, 0);
		minor = strtoul(argv[4], argv+4, 0);
		if (major > 255 || minor > 255 || argv[3][0] || argv[4][0])
			nr = 0;
	}
	if (argc != nr) {
		com_err(argv[0], 0, "Usage: mknod <name> [p| [c|b] <major> <minor>]");
		return;
	}
	if (check_fs_read_write(argv[0]))
		return;
	retval = ext2fs_new_inode(current_fs, cwd, 010755, 0, &newfile);
	if (retval) {
		com_err(argv[0], retval, "");
		return;
	}
	printf("Allocated inode: %u\n", newfile);
	retval = ext2fs_link(current_fs, cwd, argv[1], newfile, filetype);
	if (retval) {
		if (retval == EXT2_ET_DIR_NO_SPACE) {
			retval = ext2fs_expand_dir(current_fs, cwd);
			if (!retval)
				retval = ext2fs_link(current_fs, cwd,
						     argv[1], newfile,
						     filetype);
		}
		if (retval) {
			com_err(argv[1], retval, "");
			return;
		}
	}
        if (ext2fs_test_inode_bitmap(current_fs->inode_map,newfile))
		com_err(argv[0], 0, "Warning: inode already set");
	ext2fs_mark_inode_bitmap(current_fs->inode_map, newfile);
	ext2fs_mark_ib_dirty(current_fs);
	memset(&inode, 0, sizeof(inode));
	inode.i_mode = mode;
	inode.i_atime = inode.i_ctime = inode.i_mtime = time(NULL);
	inode.i_block[0] = major*256+minor;
	inode.i_links_count = 1;
	retval = ext2fs_write_inode(current_fs, newfile, &inode);
	if (retval) {
		com_err(argv[0], retval, "while trying to write inode %d", inode);
		return;
	}
}

void do_mkdir(int argc, char *argv[])
{
	char	*cp;
	ext2_ino_t	parent;
	char	*name;
	errcode_t retval;

	if (check_fs_open(argv[0]))
		return;

	if (argc != 2) {
		com_err(argv[0], 0, "Usage: mkdir <file>");
		return;
	}

	if (check_fs_read_write(argv[0]))
		return;

	cp = strrchr(argv[1], '/');
	if (cp) {
		*cp = 0;
		parent = string_to_inode(argv[1]);
		if (!parent) {
			com_err(argv[1], ENOENT, "");
			return;
		}
		name = cp+1;
	} else {
		parent = cwd;
		name = argv[1];
	}


	retval = ext2fs_mkdir(current_fs, parent, 0, name);
	if (retval) {
		com_err("ext2fs_mkdir", retval, "");
		return;
	}

}

void do_rmdir(int argc, char *argv[])
{
	printf("Unimplemented\n");
}


static int release_blocks_proc(ext2_filsys fs, blk_t *blocknr,
			       int blockcnt, void *private)
{
	printf("%d ", *blocknr);
	ext2fs_unmark_block_bitmap(fs->block_map,*blocknr);
	return 0;
}

static void kill_file_by_inode(ext2_ino_t inode)
{
	struct ext2_inode inode_buf;

	ext2fs_read_inode(current_fs, inode, &inode_buf);
	inode_buf.i_dtime = time(NULL);
	ext2fs_write_inode(current_fs, inode, &inode_buf);

	printf("Kill file by inode %u\n", inode);
	ext2fs_block_iterate(current_fs, inode, 0, NULL,
			     release_blocks_proc, NULL);
	printf("\n");
	ext2fs_unmark_inode_bitmap(current_fs->inode_map, inode);

	ext2fs_mark_bb_dirty(current_fs);
	ext2fs_mark_ib_dirty(current_fs);
}


void do_kill_file(int argc, char *argv[])
{
	ext2_ino_t inode_num;

	if (argc != 2) {
		com_err(argv[0], 0, "Usage: kill_file <file>");
		return;
	}
	if (check_fs_open(argv[0]))
		return;

	if (check_fs_read_write(argv[0]))
		return;

	inode_num = string_to_inode(argv[1]);
	if (!inode_num) {
		com_err(argv[0], 0, "Cannot find file");
		return;
	}
	kill_file_by_inode(inode_num);
}

void do_rm(int argc, char *argv[])
{
	int retval;
	ext2_ino_t inode_num;
	struct ext2_inode inode;

	if (argc != 2) {
		com_err(argv[0], 0, "Usage: rm <filename>");
		return;
	}
	if (check_fs_open(argv[0]))
		return;

	if (check_fs_read_write(argv[0]))
		return;

	retval = ext2fs_namei(current_fs, root, cwd, argv[1], &inode_num);
	if (retval) {
		com_err(argv[0], retval, "while trying to resolve filename");
		return;
	}

	retval = ext2fs_read_inode(current_fs,inode_num,&inode);
	if (retval) {
		com_err(argv[0], retval, "while reading file's inode");
		return;
	}

	if (LINUX_S_ISDIR(inode.i_mode)) {
		com_err(argv[0], 0, "file is a directory");
		return;
	}

	--inode.i_links_count;
	retval = ext2fs_write_inode(current_fs,inode_num,&inode);
	if (retval) {
		com_err(argv[0], retval, "while writing inode");
		return;
	}

	unlink_file_by_name(argv[1]);
	if (inode.i_links_count == 0)
		kill_file_by_inode(inode_num);
}

void do_show_debugfs_params(int argc, char *argv[])
{
	FILE *out = stdout;

	if (current_fs)
		fprintf(out, "Open mode: read-%s\n",
			current_fs->flags & EXT2_FLAG_RW ? "write" : "only");
	fprintf(out, "Filesystem in use: %s\n",
		current_fs ? current_fs->device_name : "--none--");
}

void do_expand_dir(int argc, char *argv[])
{
	ext2_ino_t inode;
	int retval;

	if (argc != 2) {
		com_err(argv[0], 0, "Usage: expand_dir <file>");
		return;
	}
	if (check_fs_open(argv[0]))
		return;
	inode = string_to_inode(argv[1]);
	if (!inode)
		return;

	retval = ext2fs_expand_dir(current_fs, inode);
	if (retval)
		com_err("ext2fs_expand_dir", retval, "");
	return;
}

void do_features(int argc, char *argv[])
{
	int	i;
	
	if (check_fs_open(argv[0]))
		return;

	if ((argc != 1) && check_fs_read_write(argv[0]))
		return;
	for (i=1; i < argc; i++) {
		if (e2p_edit_feature(argv[i],
				     &current_fs->super->s_feature_compat, 0))
			com_err(argv[0], 0, "Unknown feature: %s\n",
				argv[i]);
		else
			ext2fs_mark_super_dirty(current_fs);
	}
	print_features(current_fs->super, stdout);
}

static int source_file(const char *cmd_file, int sci_idx)
{
	FILE		*f;
	char		buf[256];
	char		*cp;
	int		exit_status = 0;
	int		retval;

	if (strcmp(cmd_file, "-") == 0)
		f = stdin;
	else {
		f = fopen(cmd_file, "r");
		if (!f) {
			perror(cmd_file);
			exit(1);
		}
	}
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	while (!feof(f)) {
		if (fgets(buf, sizeof(buf), f) == NULL)
			break;
		cp = strchr(buf, '\n');
		if (cp)
			*cp = 0;
		cp = strchr(buf, '\r');
		if (cp)
			*cp = 0;
		printf("debugfs: %s\n", buf);
		retval = ss_execute_line(sci_idx, buf);
		if (retval) {
			ss_perror(sci_idx, retval, buf);
			exit_status++;
		}
	}
	return exit_status;
}

int main(int argc, char **argv)
{
	int		retval;
	int		sci_idx;
	const char	*usage = "Usage: debugfs [-b blocksize] [-s superblock] [-f cmd_file] [-R request] [-V] [[-w] [-c] device]";
	int		c;
	int		open_flags = 0;
	char		*request = 0;
	int		exit_status = 0;
	char		*cmd_file = 0;
	blk_t		superblock = 0;
	blk_t		blocksize = 0;
	int		catastrophic = 0;
	char		*tmp;
	
	initialize_ext2_error_table();
	fprintf (stderr, "debugfs %s (%s)\n", E2FSPROGS_VERSION,
		 E2FSPROGS_DATE);

	while ((c = getopt (argc, argv, "iwcR:f:b:s:V")) != EOF) {
		switch (c) {
		case 'R':
			request = optarg;
			break;
		case 'f':
			cmd_file = optarg;
			break;
		case 'i':
			open_flags |= EXT2_FLAG_IMAGE_FILE;
			break;
		case 'w':
			open_flags |= EXT2_FLAG_RW;
			break;
		case 'b':
			blocksize = strtoul(optarg, &tmp, 0);
			if (*tmp) {
				com_err(argv[0], 0,
					"Bad block size - %s", optarg);
				return 1;
			}
			break;
		case 's':
			superblock = strtoul(optarg, &tmp, 0);
			if (*tmp) {
				com_err(argv[0], 0,
					"Bad superblock number - %s", optarg);
				return 1;
			}
			break;
		case 'c':
			catastrophic = 1;
			break;
		case 'V':
			/* Print version number and exit */
			fprintf(stderr, "\tUsing %s\n",
				error_message(EXT2_ET_BASE));
			exit(0);
		default:
			com_err(argv[0], 0, usage);
			return 1;
		}
	}
	if (optind < argc)
		open_filesystem(argv[optind], open_flags,
				superblock, blocksize, catastrophic);
	
	sci_idx = ss_create_invocation("debugfs", "0.0", (char *) NULL,
				       &debug_cmds, &retval);
	if (retval) {
		ss_perror(sci_idx, retval, "creating invocation");
		exit(1);
	}

	(void) ss_add_request_table (sci_idx, &ss_std_requests, 1, &retval);
	if (retval) {
		ss_perror(sci_idx, retval, "adding standard requests");
		exit (1);
	}
	if (request) {
		retval = 0;
		retval = ss_execute_line(sci_idx, request);
		if (retval) {
			ss_perror(sci_idx, retval, request);
			exit_status++;
		}
	} else if (cmd_file) {
		exit_status = source_file(cmd_file, sci_idx);
	} else {
		ss_listen(sci_idx);
	}

	if (current_fs)
		close_filesystem();
	
	return exit_status;
}
