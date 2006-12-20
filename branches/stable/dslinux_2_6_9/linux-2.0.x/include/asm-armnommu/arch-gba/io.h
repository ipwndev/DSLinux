/****************************************************************************/

/*
 * linux/include/asm-armnommu/arch-gba/io.h
 */

/****************************************************************************/
#ifndef __ASM_ARM_ARCH_IO_H
#define __ASM_ARM_ARCH_IO_H
/****************************************************************************/

#undef ARCH_IO_DELAY

#define outb_t(v,p) (*(volatile unsigned char *)(p) = (v))
#define outl_t(v,p) (*(volatile unsigned long *)(p) = (v))
#define inb_t(p)    (*(volatile unsigned char *)(p))
#define inl_t(p)    (*(volatile unsigned long *)(p))

#define outw_t(v,p) (*(volatile unsigned short *)(p) = (v))
#define inw_t(p)    (*(volatile unsigned short *)(p))

extern __inline__ void __outb (unsigned int value, unsigned int port) { outb_t(value,port); }
extern __inline__ void __outw (unsigned int value, unsigned int port) { outw_t(value,port); }
extern __inline__ void __outl (unsigned int value, unsigned int port) { outl_t(value,port); }

#define DECLARE_DYN_IN(sz,fnsuffix,instr)     \
extern __inline__ unsigned sz __in##fnsuffix (unsigned int port) { return in##fnsuffix##_t(port); }

DECLARE_DYN_IN(char,b,"b")
DECLARE_DYN_IN(short,w,"")
DECLARE_DYN_IN(long,l,"")

#undef DECLARE_DYN_IN

#define __outbc(value,port) outb_t(value,port)

#define __outwc(value,port) outw_t(value,port)
#define __outlc(value,port) outl_t(value,port)

/****************************************************************************/
#endif /* __ASM_ARM_ARCH_IO_H */
