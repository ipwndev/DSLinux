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

#include "shmemipc-arm7.h"
#include "spi.h"

void shmemipc_flush_complete(void)
{
}

void shmemipc_serve_flush_request(void)
{
	switch (SHMEMIPC_BLOCK_ARM9->user) {
	case SHMEMIPC_USER_SOUND:
		/* Handle flush request. */
		ipcsync_trigger_remote_interrupt(SHMEMIPC_FLUSH_COMPLETE);
		break;
	case SHMEMIPC_USER_WIFI:
		/* Handle flush request. */
		ipcsync_trigger_remote_interrupt(SHMEMIPC_FLUSH_COMPLETE);
		break;
	case SHMEMIPC_USER_FIRMWARE:
		/* This is a special case. We are not supposed to flush anything.
		 * Linux wants us to read firmware data into our block and
		 * send a "flush complete" when we are done. */
		shmemipc_lock();
		read_firmware(SHMEMIPC_BLOCK_ARM9->firmware.from,
			      /* Linux wants len bytes, we deliver len/2 16-bit words */
			      (u8 *) SHMEMIPC_BLOCK_ARM9->firmware.data,
			      (int)(SHMEMIPC_BLOCK_ARM9->firmware.len));
		shmemipc_unlock();
		ipcsync_trigger_remote_interrupt(SHMEMIPC_FLUSH_COMPLETE);
		break;
	}
}
