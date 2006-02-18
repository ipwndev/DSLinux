/*----------------------------------------------------------------------------
 * (C) 1997-2001 Armin Biere 
 *
 *     $Id$
 *----------------------------------------------------------------------------
 */

extern "C"
{
#include "ccmalloc.h"
#include <stdlib.h>


void ccmalloc_atexit(void(*f)(void))
{
#ifdef HAVE_ATEXIT
  atexit(f);
#else
  ccmalloc_abort ("unsupported platform/compiler: atexit() missing!\n");
#endif
}

};

class CCMalloc_InitAndReport
{
public:
  CCMalloc_InitAndReport ()
  {
    ccmalloc_static_initialization();
  }

   ~CCMalloc_InitAndReport ()
  {
    ccmalloc_report ();
  }
};

static CCMalloc_InitAndReport ccmalloc_initAndReport;
