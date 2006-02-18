/*
 *  include/asm-ppc64/cputable.h
 *
 *  Copyright (C) 2001 Ben. Herrenschmidt (benh@kernel.crashing.org)
 *
 *  Modifications for ppc64:
 *      Copyright (C) 2003 Dave Engebretsen <engebret@us.ibm.com>
 * 
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

#ifndef __ASM_PPC_CPUTABLE_H
#define __ASM_PPC_CPUTABLE_H

#include <linux/config.h>
#include <asm/page.h> /* for ASM_CONST */

/* Exposed to userland CPU features - Must match ppc32 definitions */
#define PPC_FEATURE_32			0x80000000
#define PPC_FEATURE_64			0x40000000
#define PPC_FEATURE_601_INSTR		0x20000000
#define PPC_FEATURE_HAS_ALTIVEC		0x10000000
#define PPC_FEATURE_HAS_FPU		0x08000000
#define PPC_FEATURE_HAS_MMU		0x04000000
#define PPC_FEATURE_HAS_4xxMAC		0x02000000
#define PPC_FEATURE_UNIFIED_CACHE	0x01000000

#ifdef __KERNEL__

#ifndef __ASSEMBLY__

/* This structure can grow, it's real size is used by head.S code
 * via the mkdefs mechanism.
 */
struct cpu_spec;
struct op_ppc64_model;

typedef	void (*cpu_setup_t)(unsigned long offset, struct cpu_spec* spec);

struct cpu_spec {
	/* CPU is matched via (PVR & pvr_mask) == pvr_value */
	unsigned int	pvr_mask;
	unsigned int	pvr_value;

	char		*cpu_name;
	unsigned long	cpu_features;		/* Kernel features */
	unsigned int	cpu_user_features;	/* Userland features */

	/* cache line sizes */
	unsigned int	icache_bsize;
	unsigned int	dcache_bsize;

	/* number of performance monitor counters */
	unsigned int	num_pmcs;

	/* this is called to initialize various CPU bits like L1 cache,
	 * BHT, SPD, etc... from head.S before branching to identify_machine
	 */
	cpu_setup_t	cpu_setup;

	/* Used by oprofile userspace to select the right counters */
	char		*oprofile_cpu_type;

	/* Processor specific oprofile operations */
	struct op_ppc64_model *oprofile_model;
};

extern struct cpu_spec		cpu_specs[];
extern struct cpu_spec		*cur_cpu_spec;

static inline unsigned long cpu_has_feature(unsigned long feature)
{
	return cur_cpu_spec->cpu_features & feature;
}

#endif /* __ASSEMBLY__ */

/* CPU kernel features */

/* Retain the 32b definitions for the time being - use bottom half of word */
#define CPU_FTR_SPLIT_ID_CACHE		ASM_CONST(0x0000000000000001)
#define CPU_FTR_L2CR			ASM_CONST(0x0000000000000002)
#define CPU_FTR_SPEC7450		ASM_CONST(0x0000000000000004)
#define CPU_FTR_ALTIVEC			ASM_CONST(0x0000000000000008)
#define CPU_FTR_TAU			ASM_CONST(0x0000000000000010)
#define CPU_FTR_CAN_DOZE		ASM_CONST(0x0000000000000020)
#define CPU_FTR_USE_TB			ASM_CONST(0x0000000000000040)
#define CPU_FTR_604_PERF_MON		ASM_CONST(0x0000000000000080)
#define CPU_FTR_601			ASM_CONST(0x0000000000000100)
#define CPU_FTR_HPTE_TABLE		ASM_CONST(0x0000000000000200)
#define CPU_FTR_CAN_NAP			ASM_CONST(0x0000000000000400)
#define CPU_FTR_L3CR			ASM_CONST(0x0000000000000800)
#define CPU_FTR_L3_DISABLE_NAP		ASM_CONST(0x0000000000001000)
#define CPU_FTR_NAP_DISABLE_L2_PR	ASM_CONST(0x0000000000002000)
#define CPU_FTR_DUAL_PLL_750FX		ASM_CONST(0x0000000000004000)

/* Add the 64b processor unique features in the top half of the word */
#define CPU_FTR_SLB           		ASM_CONST(0x0000000100000000)
#define CPU_FTR_16M_PAGE      		ASM_CONST(0x0000000200000000)
#define CPU_FTR_TLBIEL         		ASM_CONST(0x0000000400000000)
#define CPU_FTR_NOEXECUTE     		ASM_CONST(0x0000000800000000)
#define CPU_FTR_NODSISRALIGN  		ASM_CONST(0x0000001000000000)
#define CPU_FTR_IABR  			ASM_CONST(0x0000002000000000)
#define CPU_FTR_MMCRA  			ASM_CONST(0x0000004000000000)
/* unused 				ASM_CONST(0x0000008000000000) */
#define CPU_FTR_SMT  			ASM_CONST(0x0000010000000000)
#define CPU_FTR_COHERENT_ICACHE  	ASM_CONST(0x0000020000000000)
#define CPU_FTR_LOCKLESS_TLBIE		ASM_CONST(0x0000040000000000)
#define CPU_FTR_MMCRA_SIHV		ASM_CONST(0x0000080000000000)
#define CPU_FTR_CTRL			ASM_CONST(0x0000100000000000)

#ifndef __ASSEMBLY__

#define COMMON_USER_PPC64	(PPC_FEATURE_32 | PPC_FEATURE_64 | \
			         PPC_FEATURE_HAS_FPU | PPC_FEATURE_HAS_MMU)

#define CPU_FTR_PPCAS_ARCH_V2_BASE (CPU_FTR_SLB | \
                                 CPU_FTR_TLBIEL | CPU_FTR_NOEXECUTE | \
                                 CPU_FTR_NODSISRALIGN | CPU_FTR_CTRL)

/* iSeries doesn't support large pages */
#ifdef CONFIG_PPC_ISERIES
#define CPU_FTR_PPCAS_ARCH_V2	(CPU_FTR_PPCAS_ARCH_V2_BASE)
#else
#define CPU_FTR_PPCAS_ARCH_V2	(CPU_FTR_PPCAS_ARCH_V2_BASE | CPU_FTR_16M_PAGE)
#endif /* CONFIG_PPC_ISERIES */

#endif /* __ASSEMBLY */

#ifdef __ASSEMBLY__

#define BEGIN_FTR_SECTION		98:

#define END_FTR_SECTION(msk, val)		\
99:						\
	.section __ftr_fixup,"a";		\
	.align 3;				\
	.llong msk;			        \
	.llong val;			        \
	.llong 98b;			        \
	.llong 99b;	 		        \
	.previous

#else

#define BEGIN_FTR_SECTION		"98:\n"
#define END_FTR_SECTION(msk, val)		\
"99:\n"						\
"	.section __ftr_fixup,\"a\";\n"		\
"	.align 3;\n"				\
"	.llong "#msk";\n"			\
"	.llong "#val";\n"			\
"	.llong 98b;\n"			        \
"	.llong 99b;\n"	 		        \
"	.previous\n"

#endif /* __ASSEMBLY__ */

#define END_FTR_SECTION_IFSET(msk)	END_FTR_SECTION((msk), (msk))
#define END_FTR_SECTION_IFCLR(msk)	END_FTR_SECTION((msk), 0)

#endif /* __ASM_PPC_CPUTABLE_H */
#endif /* __KERNEL__ */

