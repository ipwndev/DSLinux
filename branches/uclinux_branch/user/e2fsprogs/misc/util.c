/*
 * util.c --- helper functions used by tune2fs and mke2fs
 * 
 * Copyright 1995, 1996, 1997, 1998, 1999, 2000 by Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

#include <stdio.h>
#include <string.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_LINUX_MAJOR_H
#include <linux/major.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "et/com_err.h"
#include "e2p/e2p.h"
#include "ext2fs/ext2_fs.h"
#include "ext2fs/ext2fs.h"
#include "nls-enable.h"
#include "get_device_by_label.h"
#include "util.h"

#ifndef HAVE_STRCASECMP
int strcasecmp (char *s1, char *s2)
{
	while (*s1 && *s2) {
		int ch1 = *s1++, ch2 = *s2++;
		if (isupper (ch1))
			ch1 = tolower (ch1);
		if (isupper (ch2))
			ch2 = tolower (ch2);
		if (ch1 != ch2)
			return ch1 - ch2;
	}
	return *s1 ? 1 : *s2 ? -1 : 0;
}
#endif

void proceed_question(void)
{
	char buf[256];
	const char *short_yes = _("yY");

	fflush(stdout);
	fflush(stderr);
	printf(_("Proceed anyway? (y,n) "));
	buf[0] = 0;
	fgets(buf, sizeof(buf), stdin);
	if (strchr(short_yes, buf[0]) == 0)
		exit(1);
}

void check_plausibility(const char *device)
{
	int val;
	struct stat s;
	
	val = stat(device, &s);
	
	if(val == -1) {
		fprintf(stderr, _("Could not stat %s --- %s\n"),
			device, error_message(errno));
		if (errno == ENOENT)
			fprintf(stderr, _("\nThe device apparently does "
			       "not exist; did you specify it correctly?\n"));
		exit(1);
	}
	if (!S_ISBLK(s.st_mode)) {
		printf(_("%s is not a block special device.\n"), device);
		proceed_question();
		return;
	}

#ifdef HAVE_LINUX_MAJOR_H
#ifndef MAJOR
#define MAJOR(dev)	((dev)>>8)
#define MINOR(dev)	((dev) & 0xff)
#endif
#ifndef SCSI_DISK_MAJOR
#define	SCSI_DISK_MAJOR(M) \
		(((M) == SCSI_DISK0_MAJOR) || \
		(((M) >= SCSI_DISK1_MAJOR) && ((M) <= SCSI_DISK7_MAJOR)) || \
		(((M) >= SCSI_DISK8_MAJOR) && ((M) <= SCSI_DISK15_MAJOR)))
#endif
#ifndef SCSI_BLK_MAJOR
#define SCSI_BLK_MAJOR(M)  (SCSI_DISK_MAJOR(M) || (M) == SCSI_CDROM_MAJOR)
#endif
	if (((MAJOR(s.st_rdev) == HD_MAJOR &&
	      MINOR(s.st_rdev)%64 == 0) ||
	     (SCSI_BLK_MAJOR(MAJOR(s.st_rdev)) &&
	      MINOR(s.st_rdev)%16 == 0))) {
		printf(_("%s is entire device, not just one partition!\n"),
		       device);
		proceed_question();
	}
#endif
}

void check_mount(const char *device, int force, const char *type)
{
	errcode_t	retval;
	int		mount_flags;

	retval = ext2fs_check_if_mounted(device, &mount_flags);
	if (retval) {
		com_err("ext2fs_check_if_mount", retval,
			_("while determining whether %s is mounted."),
			device);
		return;
	}
	if (!(mount_flags & EXT2_MF_MOUNTED))
		return;

	fprintf(stderr, _("%s is mounted; "), device);
	if (force) {
		fprintf(stderr, _("mke2fs forced anyway.  "
			"Hope /etc/mtab is incorrect.\n"));
	} else {
		fprintf(stderr, _("will not make a %s here!\n"), type);
		exit(1);
	}
}

void parse_journal_opts(const char *opts)
{
	char	*buf, *token, *next, *p, *arg;
	int	len;
	int	journal_usage = 0;

	len = strlen(opts);
	buf = malloc(len+1);
	if (!buf) {
		fprintf(stderr, _("Couldn't allocate memory to parse "
			"journal options!\n"));
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
#if 0
		printf("Journal option=%s, argument=%s\n", token,
		       arg ? arg : "NONE");
#endif
		if (strcmp(token, "device") == 0) {
			journal_device = interpret_spec(arg);
			if (!journal_device) {
				journal_usage++;
				continue;
			}
		} else if (strcmp(token, "size") == 0) {
			if (!arg) {
				journal_usage++;
				continue;
			}
			journal_size = strtoul(arg, &p, 0);
			if (*p)
				journal_usage++;
		} else if (strcmp(token, "v1_superblock") == 0) {
			journal_flags |= EXT2_MKJOURNAL_V1_SUPER;
			continue;
		} else
			journal_usage++;
	}
	if (journal_usage) {
		fprintf(stderr, _("\nBad journal options specified.\n\n"
			"Journal options are separated by commas, "
			"and may take an argument which\n"
			"\tis set off by an equals ('=') sign.\n\n"
			"Valid raid options are:\n"
			"\tsize=<journal size in megabytes>\n"
			"\tdevice=<journal device>\n\n"
			"The journal size must be between "
			"1024 and 102400 filesystem blocks.\n\n" ));
		exit(1);
	}
}	

/*
 * Determine the number of journal blocks to use, either via
 * user-specified # of megabytes, or via some intelligently selected
 * defaults.
 * 
 * Find a reasonable journal file size (in blocks) given the number of blocks
 * in the filesystem.  For very small filesystems, it is not reasonable to
 * have a journal that fills more than half of the filesystem.
 */
int figure_journal_size(int journal_size, ext2_filsys fs)
{
	blk_t j_blocks;

	if (fs->super->s_blocks_count < 2048) {
		fprintf(stderr, _("\nFilesystem too small for a journal\n"));
		return 0;
	}
	
	if (journal_size >= 0) {
		j_blocks = journal_size * 1024 /
			(fs->blocksize	/ 1024);
		if (j_blocks < 1024 || j_blocks > 102400) {
			fprintf(stderr, _("\nThe requested journal "
				"size is %d blocks; it must be\n"
				"between 1024 and 102400 blocks.  "
				"Aborting.\n"),
				j_blocks);
			exit(1);
		}
		if (j_blocks > fs->super->s_free_blocks_count) {
			fprintf(stderr, _("\nJournal size too big "
					  "for filesystem.\n"));
			exit(1);
		}
		return j_blocks;
	}

	if (fs->super->s_blocks_count < 32768)
		j_blocks = 1024;
	else if (fs->super->s_blocks_count < 262144)
		j_blocks = 4096;
	else
		j_blocks = 8192;

	return j_blocks;
}

void print_check_message(ext2_filsys fs)
{
#ifdef EMBED
	printf(_("This filesystem will be automatically "
		 "checked every %d mounts or\n"
		 "%d.%02d days, whichever comes first.  "
		 "Use tune2fs -c or -i to override.\n"),
	       fs->super->s_max_mnt_count,
	       fs->super->s_checkinterval / (3600 * 24),
	       (fs->super->s_checkinterval * 100 / (3600 * 24)) % 100);
#else
	printf(_("This filesystem will be automatically "
		 "checked every %d mounts or\n"
		 "%g days, whichever comes first.  "
		 "Use tune2fs -c or -i to override.\n"),
	       fs->super->s_max_mnt_count,
	       (double)fs->super->s_checkinterval / (3600 * 24));
#endif
}
