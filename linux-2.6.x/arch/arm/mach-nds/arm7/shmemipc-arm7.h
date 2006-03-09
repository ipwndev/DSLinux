/*
 * Copyright (c) 2005 Stefan Sperling <stsp@stsp.in-berlin.de>
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
 *
 */

/* 
 * This file belongs to the inter-processor-communication framework
 * using shared memory. It uses ipcsync for communication.
 * Data is transfered via shared memory. See shmemipc.h for a detailed
 * explanation of what this code is supposed to do.
 *
 * This is the ARM7 side of things. Authors of device drivers that use
 * shmemipc should implement the ARM7 part of their driver in this file.
 */

#ifndef SHMEMIPC_ARM7_H
#define SHMEMIPC_ARM7_H

#include "asm/arch/ipcsync.h"
#include "asm/arch/shmemipc.h"
#include "arm7.h"

/* The Linux-specific definitions of these are no use here. */
#undef shmemipc_lock
#undef shmemipc_unlock

/* These locks should be used to lock write access to shared memory.
 * They disable and enable the ARM9 interrupt respectively. */
#define shmemipc_lock() NDS_IE &= ~IRQ_ARM9
#define shmemipc_unlock() NDS_IE |= IRQ_ARM9

void shmemipc_serve_flush_request(void);
void shmemipc_flush_complete(void);
#endif /* SHMEMIPC_ARM7_H */
