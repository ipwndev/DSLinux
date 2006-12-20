#  PowerPC-32 mpn_lshift -- Shift a number left.

# Copyright (C) 1995, 1998, 2000 Free Software Foundation, Inc.

# This file is part of the GNU MP Library.

# The GNU MP Library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.

# The GNU MP Library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.

# You should have received a copy of the GNU Library General Public License
# along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
# the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
# MA 02111-1307, USA.


# INPUT PARAMETERS
# res_ptr	r3
# s1_ptr	r4
# size		r5
# cnt		r6

include(`../config.m4')

ASM_START()
PROLOGUE(mpn_lshift)
	cmpi	cr0,r5,12	# more than 12 limbs?
	slwi	r0,r5,2
	add	r4,r4,r0	# make r4 point at end of s1
	add	r7,r3,r0	# make r7 point at end of res
	bgt	.LBIG		# branch if more than 12 limbs

	mtctr	r5		# copy size into CTR
	subfic	r8,r6,32
	lwzu	r11,-4(r4)	# load first s1 limb
	srw	r3,r11,r8	# compute function return value
	bdz	.Lend1

.Loop:	lwzu	r10,-4(r4)
	slw	r9,r11,r6
	srw	r12,r10,r8
	or	r9,r9,r12
	stwu	r9,-4(r7)
	bdz	.Lend2
	lwzu	r11,-4(r4)
	slw	r9,r10,r6
	srw	r12,r11,r8
	or	r9,r9,r12
	stwu	r9,-4(r7)
	bdnz	.Loop

.Lend1:	slw	r0,r11,r6
	stw	r0,-4(r7)
	blr
.Lend2:	slw	r0,r10,r6
	stw	r0,-4(r7)
	blr

.LBIG:
	stmw	r24,-32(r1)	# save registers we are supposed to preserve
	lwzu	r9,-4(r4)
	subfic	r8,r6,32
	srw	r3,r9,r8	# compute function return value
	slw	r0,r9,r6
	addi	r5,r5,-1

	andi.	r10,r5,3	# count for spill loop
	beq	.Le
	mtctr	r10
	lwzu	r28,-4(r4)
	bdz	.Lxe0

.Loop0:	slw	r12,r28,r6
	srw	r24,r28,r8
	lwzu	r28,-4(r4)
	or	r24,r0,r24
	stwu	r24,-4(r7)
	mr	r0,r12
	bdnz	.Loop0		# taken at most once!

.Lxe0:	slw	r12,r28,r6
	srw	r24,r28,r8
	or	r24,r0,r24
	stwu	r24,-4(r7)
	mr	r0,r12

.Le:	srwi	r5,r5,2		# count for unrolled loop
	addi	r5,r5,-1
	mtctr	r5
	lwz	r28,-4(r4)
	lwz	r29,-8(r4)
	lwz	r30,-12(r4)
	lwzu	r31,-16(r4)

.LoopU:	slw	r9,r28,r6
	srw	r24,r28,r8
	lwz	r28,-4(r4)
	slw	r10,r29,r6
	srw	r25,r29,r8
	lwz	r29,-8(r4)
	slw	r11,r30,r6
	srw	r26,r30,r8
	lwz	r30,-12(r4)
	slw	r12,r31,r6
	srw	r27,r31,r8
	lwzu	r31,-16(r4)
	or	r24,r0,r24
	stw	r24,-4(r7)
	or	r25,r9,r25
	stw	r25,-8(r7)
	or	r26,r10,r26
	stw	r26,-12(r7)
	or	r27,r11,r27
	stwu	r27,-16(r7)
	mr	r0,r12
	bdnz	.LoopU

	slw	r9,r28,r6
	srw	r24,r28,r8
	slw	r10,r29,r6
	srw	r25,r29,r8
	slw	r11,r30,r6
	srw	r26,r30,r8
	slw	r12,r31,r6
	srw	r27,r31,r8
	or	r24,r0,r24
	stw	r24,-4(r7)
	or	r25,r9,r25
	stw	r25,-8(r7)
	or	r26,r10,r26
	stw	r26,-12(r7)
	or	r27,r11,r27
	stwu	r27,-16(r7)
	mr	r0,r12

	stw	r0,-4(r7)
	lmw	r24,-32(r1)	# restore registers
	blr
EPILOGUE(mpn_lshift)
