/*
 * linux/include/asm-arm/arch-s3c3410/memory.h
 */

#ifndef __ASM_ARCH_MEMORY_H
#define __ASM_ARCH_MEMORY_H

#include <linux/config.h>

#define TASK_SIZE	(0x0cc00000UL)
#define TASK_SIZE_26	TASK_SIZE

#define PHYS_OFFSET	(CONFIG_DRAM_BASE)
#define PAGE_OFFSET 	(PHYS_OFFSET)
#define END_MEM     	(CONFIG_DRAM_BASE + CONFIG_DRAM_SIZE)



#define __virt_to_phys(vpage) ((unsigned long) (vpage))
#define __phys_to_virt(ppage) ((void *) (ppage))
#define __virt_to_bus(vpage) ((unsigned long) (vpage))
#define __bus_to_virt(ppage) ((void *) (ppage))

#endif /*  __ASM_ARCH_MEMORY_H */
