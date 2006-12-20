/*
 * scantest.c - test the speed of the inode scan routine
 */

#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <time.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#include <unistd.h>
#ifdef HAVE_MNTENT_H
#include <mntent.h>
#endif
#include <sys/ioctl.h>
#include <malloc.h>
#include <sys/resource.h>

#include "et/com_err.h"
#include "../version.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include "ext2fs/ext2_fs.h"
#include "ext2fs/ext2fs.h"


extern int isatty(int);

const char * device_name = NULL;

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

struct resource_track	global_rtrack;

void init_resource_track(struct resource_track *track)
{
	struct rusage r;
	
#ifdef HAVE_SBRK
	track->brk_start = sbrk(0);
#endif
	gettimeofday(&track->time_start, 0);
	getrusage(RUSAGE_SELF, &r);
	track->user_start = r.ru_utime;
	track->system_start = r.ru_stime;
}

#ifdef EMBED
static int timeval_subtract(struct timeval *tv1,
					 struct timeval *tv2)
{
	return ((tv1->tv_sec - tv2->tv_sec) +
		((int) (tv1->tv_usec - tv2->tv_usec)) / 1000000);
}

static int timeval_subtract_100(struct timeval *tv1,
					 struct timeval *tv2)
{
	return (((tv1->tv_sec - tv2->tv_sec) +
		((int) (tv1->tv_usec - tv2->tv_usec)) / 10000) % 100);
}
#else
static __inline__ float timeval_subtract(struct timeval *tv1,
					 struct timeval *tv2)
{
	return ((tv1->tv_sec - tv2->tv_sec) +
		((float) (tv1->tv_usec - tv2->tv_usec)) / 1000000);
}
#endif

static void print_resource_track(struct resource_track *track)
{
	struct rusage r;
	struct timeval time_end;
	int memused = 0;

#ifdef HAVE_SBRK
	memused = (int) (((char *) sbrk(0)) - ((char *) track->brk_start));
#endif
	gettimeofday(&time_end, 0);
	getrusage(RUSAGE_SELF, &r);

#ifdef EMBED
	printf(_("Memory used: %d, elapsed time: %6d.%02d/%6d.%02d/%6d.%02d\n"),
	       memused,
	       timeval_subtract(&time_end, &track->time_start),
	       timeval_subtract_100(&time_end, &track->time_start),
	       timeval_subtract(&r.ru_utime, &track->user_start),
	       timeval_subtract_100(&r.ru_utime, &track->user_start),
	       timeval_subtract(&r.ru_stime, &track->system_start),
	       timeval_subtract_100(&r.ru_stime, &track->system_start));
#else
	printf(_("Memory used: %d, elapsed time: %6.3f/%6.3f/%6.3f\n"),
	       memused,
	       timeval_subtract(&time_end, &track->time_start),
	       timeval_subtract(&r.ru_utime, &track->user_start),
	       timeval_subtract(&r.ru_stime, &track->system_start));
#endif
}



int main (int argc, char *argv[])
{
	errcode_t	retval = 0;
	int		exit_value = 0;
	int		i;
	ext2_filsys	fs;
	ext2_inode_scan	scan;
	ext2_ino_t	ino;
	struct ext2_inode inode;

	printf(_("size of inode=%d\n"), sizeof(inode));

	device_name = "/dev/hda3";
	
	init_resource_track(&global_rtrack);

	retval = ext2fs_open(device_name, 0,
			     0, 0, unix_io_manager, &fs);
	if (retval) {
		com_err(argv[0], retval, _("while trying to open %s"),
			device_name);
		exit(1);
	}

	retval = ext2fs_open_inode_scan(fs, 0, &scan);
	if (retval) {
		com_err(argv[0], retval, _("while opening inode scan"));
		exit(1);
	}
	retval = ext2fs_get_next_inode(scan, &ino, &inode);
	if (retval) {
		com_err(argv[0], retval, _("while starting inode scan"));
		exit(1);
	}
	while (ino) {
		if (!inode.i_links_count)
			goto next;
		printf("%lu\n", inode.i_blocks);
	next:
		retval = ext2fs_get_next_inode(scan, &ino, &inode);
		if (retval) {
			com_err(argv[0], retval,
				_("while doing inode scan"));
			exit(1);
		}
	}
	

	ext2fs_close(fs);
	
	print_resource_track(&global_rtrack);
	
	return exit_value;
}
