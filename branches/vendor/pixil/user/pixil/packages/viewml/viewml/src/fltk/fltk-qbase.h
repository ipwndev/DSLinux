#ifndef __FLTK_QBASE_H
#define __FLTK_QBASE_H

#include <stdio.h>
#include <stdarg.h>

#include "qstring.h"
#include "qstrlist.h"
#include "qevent.h"
#include "qlist.h"
#include "qarray.h"
#include "qbuffer.h"
#include "qregion.h"
#include "qcolor.h"

#include "qiodevice.h"
#include "qobjectdefs.h"
#include "qobject.h"
#include "qmetaobject.h"
#include "qconnection.h"

#ifndef _NANOX 

#include <X11/X.h>
#include <X11/Xlib.h>

#else

typedef unsigned long Window;
typedef unsigned long Display;
typedef unsigned long XEvent;

extern unsigned long KeyPress;

bool XCheckTypedEvent( Display *, unsigned long , 
		       XEvent * ); 
#endif

#include "fltk-qdefs.h"

const uint WCursorSet           = 0x00100000;   // misc widget flags
const uint WDestructiveClose    = 0x00200000;
const uint WPaintDesktop        = 0x00400000;
const uint WPaintUnclipped      = 0x00800000;
const uint WPaintClever         = 0x01000000;
const uint WConfigPending       = 0x02000000;
const uint WResizeNoErase       = 0x04000000;
const uint WRecreated           = 0x08000000;
const uint WExportFontMetrics   = 0x10000000;
const uint WExportFontInfo      = 0x20000000;
const uint WFocusSet            = 0x40000000;   // not used any more
const uint WState_TabToFocus    = 0x80000000;

typedef int WFlags;

#define debug printf
#define fatal printf

#define emit
#define stricmp strcasecmp
#define strnicmp strncasecmp

inline void warning(const char * format,...)
{
  va_list vl;
  va_start(vl,format);
  vprintf(format,vl);
  va_end(vl);
}

inline Display * qt_xdisplay() { return 0; }

#endif
