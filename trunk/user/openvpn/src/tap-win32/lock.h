/*
 *  TAP-Win32 -- A kernel driver to provide virtual tap device
 *               functionality on Windows.  Originally derived
 *               from the CIPE-Win32 project by Damion K. Wilson,
 *               with extensive modifications by James Yonan.
 *
 *  All source code which derives from the CIPE-Win32 project is
 *  Copyright (C) Damion K. Wilson, 2003, and is released under the
 *  GPL version 2 (see below).
 *
 *  All other source code is Copyright (C) 2002-2005 OpenVPN Solutions LLC,
 *  and is released under the GPL version 2 (see below).
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program (see the file COPYING included with this
 *  distribution); if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

typedef struct
{
  volatile long count;
} MUTEX;

#define MUTEX_SLEEP_TIME  10000 // microseconds

#define INIT_MUTEX(m) { (m)->count = 0; }

#define ACQUIRE_MUTEX_BLOCKING(m)                         \
{                                                         \
    while (NdisInterlockedIncrement (&((m)->count)) != 1) \
    {                                                     \
        NdisInterlockedDecrement(&((m)->count));          \
        NdisMSleep(MUTEX_SLEEP_TIME);                     \
    }                                                     \
}

#define RELEASE_MUTEX(m)                                  \
{                                                         \
        NdisInterlockedDecrement(&((m)->count));          \
}

#define ACQUIRE_MUTEX_NONBLOCKING(m, result)              \
{                                                         \
    if (NdisInterlockedIncrement (&((m)->count)) != 1)    \
    {                                                     \
        NdisInterlockedDecrement(&((m)->count));          \
        result = FALSE;                                   \
    }                                                     \
    else                                                  \
    {                                                     \
	result = TRUE;                                    \
    }                                                     \
}

#define ACQUIRE_MUTEX_ADAPTIVE(m, result)                 \
{                                                         \
    result = TRUE;                                        \
    while (NdisInterlockedIncrement (&((m)->count)) != 1) \
    {                                                     \
        NdisInterlockedDecrement(&((m)->count));          \
        if (KeGetCurrentIrql () < DISPATCH_LEVEL)         \
            NdisMSleep(MUTEX_SLEEP_TIME);                 \
        else                                              \
        {                                                 \
	    result = FALSE;                               \
	    break;                                        \
        }                                                 \
    }                                                     \
}
