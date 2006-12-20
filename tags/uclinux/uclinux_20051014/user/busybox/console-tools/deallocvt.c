/* vi: set sw=4 ts=4: */
/*
 * Disallocate virtual terminal(s)
 *
 * Copyright (C) 2003 by Tito Ragusa <farmatito@tiscali.it>
 * Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* no options, no getopt */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "busybox.h"

/* From <linux/vt.h> */
static const int VT_DISALLOCATE = 0x5608;  /* free memory associated to vt */

int deallocvt_main(int argc, char *argv[])
{
	/* num = 0 deallocate all unused consoles */
	int num = 0;

	switch(argc)
	{
		case 2:
			if((num = bb_xgetlarg(argv[1], 10, 0, INT_MAX)) == 0)
				bb_error_msg_and_die("0: illegal VT number");
		/* Falltrough */
		case 1:
			break;
		default:
			bb_show_usage();
	}

	if (ioctl( get_console_fd(), VT_DISALLOCATE, num )) {
		bb_perror_msg_and_die("VT_DISALLOCATE");
	}
	return EXIT_SUCCESS;
}
