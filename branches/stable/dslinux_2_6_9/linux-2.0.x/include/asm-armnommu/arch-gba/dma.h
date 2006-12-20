/****************************************************************************/

/*
 * linux/include/asm-armnommu/arch-gba/dma.h
 */

/****************************************************************************/
#ifndef __ASM_ARCH_DMA_H
#define __ASM_ARCH_DMA_H
/****************************************************************************/

#define MAX_DMA_ADDRESS		0x03000000

typedef enum {
	DMA_MODE_READ,
	DMA_MODE_WRITE
} dmamode_t;

#define MAX_DMA_CHANNELS	1

/****************************************************************************/
#endif	/* __ASM_ARCH_DMA_H */
