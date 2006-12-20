//
// "$Id$"
//
// Warning/error message code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-1999 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@easysw.com".
//

// This method is in it's own source file so that stdlib and stdio
// do not need to be included in Fl.C:
// You can also override this by redefining all of these.

#include <FL/Fl.H>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <config.h>

#ifndef WIN32

static void warning(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputc('\n', stderr);
  fflush(stderr);
}

static void error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputc('\n', stderr);
  fflush(stderr);
  ::exit(1);
}

#else

#include <windows.h>
#  if !HAVE_VSNPRINTF || defined(__hpux)
extern "C" {
int vsnprintf(char* str, size_t size, const char* fmt, va_list ap);
}
#  endif /* !HAVE_VSNPRINTF */

static void warning(const char *format, ...) {
  va_list args;
  char buf[1024];
  va_start(args, format);
  vsnprintf(buf, 1024, format, args);
  va_end(args);
  MessageBox(0,buf,"Warning",MB_ICONEXCLAMATION|MB_OK);
}

static void error(const char *format, ...) {
  va_list args;
  char buf[1024];
  va_start(args, format);
  vsnprintf(buf, 1024, format, args);
  va_end(args);
  MessageBox(0,buf,"Error",MB_ICONSTOP|MB_SYSTEMMODAL);
  ::exit(1);
}

#endif

void (*Fl::warning)(const char* format, ...) = ::warning;
void (*Fl::error)(const char* format, ...) = ::error;
void (*Fl::fatal)(const char* format, ...) = ::error;

//
// End of "$Id$".
//
