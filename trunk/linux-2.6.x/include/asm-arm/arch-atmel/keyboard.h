/*
 *  linux/include/asm-armnommu/arch-atmel/keyboard.h
 *
 *  Copyright (C) 2003 SAMSUNG ELECTRONICS 
 *                         Hyok S. Choi (hyok.choi@samsung.com)
 *
 *  just a cite from other architecture :)
 */
#ifndef __ASM_ARM_ARCH_ATMEL_KEYBOARD_H
#define __ASM_ARM_ARCH_ATMEL_KEYBOARD_H

#define kbd_setkeycode(sc,kc) (-EINVAL)
#define kbd_getkeycode(sc) (-EINVAL)
#define kbd_translate(sc,kcp,rm) ({ *(kcp) = (sc); 1; })
#define kbd_unexpected_up(kc) (0200)
#define kbd_leds(leds)
#define kbd_init_hw()
#define kbd_enable_irq()
#define kbd_disable_irq()

#endif /* __ASM_ARM_ARCH_ATMEL_KEYBOARD_H */
