/*
 *  linux/include/asm-armnommu/proc-fns.h
 *
 *  Copyright (C) 1997-1999 Russell King
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
 *  Copyright (C) 2003 Hyok S. Choi, Samsung Electronics Co.,Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ASM_PROCFNS_H
#define __ASM_PROCFNS_H

#ifdef __KERNEL__

#include <linux/config.h>

/*
 * Work out if we need multiple CPU support
 */
#undef MULTI_CPU
#undef CPU_NAME

/*
 * CPU_NAME - the prefix for CPU related functions
 */

#ifdef CONFIG_CPU_32
# ifdef CONFIG_CPU_ARM610
#  ifdef CPU_NAME
#   undef  MULTI_CPU
#   define MULTI_CPU
#  else
#   define CPU_NAME cpu_arm6
#  endif
# endif
# ifdef CONFIG_CPU_ARM7V3
#  ifdef CPU_NAME
#   undef  MULTI_CPU
#   define MULTI_CPU
#  else
#   define CPU_NAME cpu_arm7
#  endif
# endif
# ifdef CONFIG_CPU_ARM710
#  ifdef CPU_NAME
#   undef  MULTI_CPU
#   define MULTI_CPU
#  else
#   define CPU_NAME cpu_arm7
#  endif
# endif
# ifdef CONFIG_CPU_S3C4510B
#  ifdef CPU_NAME
#   undef  MULTI_CPU
#   define MULTI_CPU
#  else
#   define CPU_NAME cpu_arm7
#  endif
# endif
# ifdef CONFIG_CPU_ARM720T
#  ifdef CPU_NAME
#   undef  MULTI_CPU
#   define MULTI_CPU
#  else
#   define CPU_NAME cpu_arm720
#  endif
# endif
# ifdef CONFIG_CPU_ARM920T
#  ifdef CPU_NAME
#   undef  MULTI_CPU
#   define MULTI_CPU
#  else
#   define CPU_NAME cpu_arm920
#  endif
# endif
# ifdef CONFIG_CPU_ARM922T
#  ifdef CPU_NAME
#   undef  MULTI_CPU
#   define MULTI_CPU
#  else
#   define CPU_NAME cpu_arm922
#  endif
# endif
# ifdef CONFIG_CPU_ARM926T
#  ifdef CPU_NAME
#   undef  MULTI_CPU
#   define MULTI_CPU
#  else
#   define CPU_NAME cpu_arm926
#  endif
# endif
# ifdef CONFIG_CPU_ARM940T
#  ifdef CPU_NAME
#   undef  MULTI_CPU
#   define MULTI_CPU
#  else
#   define CPU_NAME cpu_arm940
#  endif
# endif
# ifdef CONFIG_CPU_ARM946E
#  ifdef CPU_NAME
#   undef  MULTI_CPU
#   define MULTI_CPU
#  else
#   define CPU_NAME cpu_arm946
#  endif
# endif
# ifdef CONFIG_CPU_ARM1020
#  ifdef CPU_NAME
#   undef  MULTI_CPU
#   define MULTI_CPU
#  else
#   define CPU_NAME cpu_arm1020
#  endif
# endif
# ifdef CONFIG_CPU_ARM1020E
#  ifdef CPU_NAME
#   undef  MULTI_CPU
#   define MULTI_CPU
#  else
#   define CPU_NAME cpu_arm1020e
#  endif
# endif
# ifdef CONFIG_CPU_ARM1022
#  ifdef CPU_NAME
#   undef  MULTI_CPU
#   define MULTI_CPU
#  else
#   define CPU_NAME cpu_arm1022
#  endif
# endif
# ifdef CONFIG_CPU_ARM1026
#  ifdef CPU_NAME
#   undef  MULTI_CPU
#   define MULTI_CPU
#  else
#   define CPU_NAME cpu_arm1026
#  endif
# endif
#endif

#ifndef __ASSEMBLY__

#ifndef MULTI_CPU
#include "asm/cpu-single.h"
#else
#include "asm/cpu-multi32.h"
#endif

#include <asm/memory.h>

#define cpu_switch_mm(pgd,mm) cpu_do_switch_mm(virt_to_phys(pgd),mm)

#endif /* __ASSEMBLY__ */
#endif /* __KERNEL__ */
#endif /* __ASM_PROCFNS_H */
