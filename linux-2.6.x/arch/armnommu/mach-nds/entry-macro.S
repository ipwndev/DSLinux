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
		ldr	\irqstat, =0x04000214			/* Get IF reg */
		ldr	\irqstat, [\irqstat]

		mov	\irqnr, #0				/* IRQ counter */
1001:
		tst	\irqstat, #1			/* test if irq active */
		bne	1002f

		add	\irqnr, \irqnr, #1
		mov	\irqstat, \irqstat, lsr #1			/* prepare test next irq */
		cmp	\irqnr, #NR_IRQS				/* test if irqs ended*/
		bcc	1001b
1002:
		.endm
		
		.macro	irq_prio_table
		.endm

