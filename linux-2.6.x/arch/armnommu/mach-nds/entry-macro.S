/*
 *  arch/armnommu/mach-nds/entry-macro.S
 *
 * Copyright (C) 2005 Malcolm Parsons <pepsiman@blueyonder.co.uk
 *
 *  defines machine dependent entry macros.
 *  included in the arch/armnommu/kernel/entry.S
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

		.macro	disable_fiq
		.endm

        .macro get_irqnr_and_base, irqnr, irqstat, base, tmp
		ldr	r4, =0x04000214			/* Get IF reg */
		ldr	r0, [r4]

		mov	r4, #1				/* IRQ counter */
1:
		tst	r0, #0x0001			/* test if irq r4 active */
		movne	\irqnr, r4
		bne	2f

		mov	r0, r0, lsr #1			/* prepare test next irq */
		add	r4, r4, #1
		teq	r4, #15				/* test if irqs ended*/
		bne	1b
 
		moveq	\irqnr, #0			/* no irq found */
		beq	3f
2:
		adr	\base, flag_irqs
		ldr	r0, [\base, \irqnr, lsl #2]
		ldr	r4, =0x04000214
		str	r0, [r4]			/* irq acquired */
3:
		adr	\base, irq_prio_gba
		.endm
		
		.macro	irq_prio_table
irq_prio_gba:
		.byte   0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34

flag_irqs:
		.word	0x00000000,0x00000001, 0x00000002,0x00000004
		.word	0x00000008,0x00000010, 0x00000020,0x00000040
		.word	0x00000080,0x00000100, 0x00000200,0x00000400
		.word	0x00000800,0x00001000, 0x00002000,0x00004000
		.word	0x00000000,0x00010000, 0x00020000,0x00040000
		.word	0x00080000,0x00100000, 0x00200000,0x00400000
		.word	0x00800000,0x01000000, 0x02000000,0x04000000
		.word	0x08000000,0x10000000, 0x20000000,0x00000000
		.endm

