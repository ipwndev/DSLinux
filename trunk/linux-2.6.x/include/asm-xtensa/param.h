/*
 * include/asm-xtensa/param.h
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2001 - 2005 Tensilica Inc.
 */

#ifndef _XTENSA_PARAM_H
#define _XTENSA_PARAM_H

#include <xtensa/config/core.h>

#ifdef __KERNEL__
# define HZ		100		/* internal timer frequency */
# define USER_HZ	100		/* for user interfaces in "ticks" */
# define CLOCKS_PER_SEC (USER_HZ)	/* frequnzy at which times() counts */
#endif

#define EXEC_PAGESIZE	(1 << XCHAL_MMU_MIN_PTE_PAGE_SIZE)

#ifndef NGROUPS
#define NGROUPS		32
#endif

#ifndef NOGROUP
#define NOGROUP		(-1)
#endif

#define MAXHOSTNAMELEN	64	/* max length of hostname */

#endif /* _XTENSA_PARAM_H */
