/* vi: set sw=4 ts=4: */
/*
 * sleep implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

/* BB_AUDIT SUSv3 compliant */
/* BB_AUDIT GNU issues -- fancy version matches except args must be ints. */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/sleep.html */

/* Mar 16, 2003      Manuel Novoa III   (mjn3@codepoet.org)
 *
 * Rewritten to do proper arg and error checking.
 * Also, added a 'fancy' configuration to accept multiple args with
 * time suffixes for seconds, minutes, hours, and days.
 */

#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include "busybox.h"

#ifdef CONFIG_FEATURE_FANCY_SLEEP
static const struct suffix_mult sleep_suffixes[] = {
	{ "s", 1 },
	{ "m", 60 },
	{ "h", 60*60 },
	{ "d", 24*60*60 },
	{ NULL, 0 }
};
#endif

extern int sleep_main(int argc, char **argv)
{
	unsigned int duration;

#ifdef CONFIG_FEATURE_FANCY_SLEEP

	if (argc < 2) {
		bb_show_usage();
	}

	++argv;
	duration = 0;
	do {
		duration += bb_xgetularg_bnd_sfx(*argv, 10,
										 0, UINT_MAX-duration,
										 sleep_suffixes);
	} while (*++argv);

#else  /* CONFIG_FEATURE_FANCY_SLEEP */

	if (argc != 2) {
		bb_show_usage();
	}

#if UINT_MAX == ULONG_MAX
	duration = bb_xgetularg10(argv[1]);
#else
	duration = bb_xgetularg10_bnd(argv[1], 0, UINT_MAX);
#endif

#endif /* CONFIG_FEATURE_FANCY_SLEEP */

	if (sleep(duration)) {
		bb_perror_nomsg_and_die();
	}

	return EXIT_SUCCESS;
}
