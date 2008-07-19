/* Symbol size test program.

   Copyright 2006 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifdef SYMBOL_PREFIX
#define SYMBOL(str)	SYMBOL_PREFIX #str
#else
#define SYMBOL(str)	#str
#endif

void
trap (void)
{
  asm ("int $0x03");
}

/* Jump from a function with its symbol size set, to a function
   named by a local label.  If GDB does not look at the sizes of
   symbols, we will still appear to be in the first function.  */

asm(".text\n"
    "    .align 8\n"
    "    .globl " SYMBOL (main) "\n"
    SYMBOL (main) ":\n"
    "    pushl %ebp\n"
    "    mov   %esp, %ebp\n"
    "    call  .Lfunc\n"
    "    ret\n"
    "    .size " SYMBOL (main) ", .-" SYMBOL (main) "\n"
    ".Lfunc:\n"
    "    pushl %ebp\n"
    "    mov   %esp, %ebp\n"
    "    call  " SYMBOL (trap) "\n");
