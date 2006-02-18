/* Nessus
 * Copyright (C) 1998 - 2001 Renaud Deraison
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2,
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * In addition, as a special exception, Renaud Deraison
 * gives permission to link the code of this program with any
 * version of the OpenSSL library which is distributed under a
 * license identical to that listed in the included COPYING.OpenSSL
 * file, and distribute linked combinations including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */           
 
#include <config.h>
#ifdef NESSUSNT
#include <ntcompat.h>
#include <nessuslib.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <stdarg.h>
#include "wstuff.h"


void
print_error(char * data, ...)
{
  va_list param;
  char * buffer;
  buffer = emalloc(4096);
  va_start(param, data);
  vsprintf(buffer, data, param);
  MessageBox(NULL, buffer,"Nessusd error", MB_OK | MB_ICONSTOP);
  efree(&buffer);
}        


#endif
