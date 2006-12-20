/****************************************************************************/

/*
 *	linux/include/asm-armnommu/arch-gba/hardware.h
 */

/****************************************************************************/
#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H
/****************************************************************************/

#include <linux/config.h>

#ifndef __ASSEMBLER__
/* ARM asynchronous clock */
#define ARM_CLK	((unsigned long)(CONFIG_ARM_CLK))
#else
#define ARM_CLK	CONFIG_ARM_CLK
#endif

#ifndef __ASSEMBLER__
/*
 * RAM definitions
 */
#define MAPTOPHYS(a)      ((unsigned long)a)
#define KERNTOPHYS(a)     ((unsigned long)(&a))
#define GET_MEMORY_END(p) ((p->u1.s.page_size) * (p->u1.s.nr_pages))

#define PARAMS_BASE       0x7000

#define HARD_RESET_NOW()  { arch_hard_reset(); }

#endif

/****************************************************************************/
#endif	/* __ASM_ARCH_HARDWARE_H */
