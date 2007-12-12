#include "syshdrs.h"
#ifdef PRAGMA_HDRSTOP
#	pragma no_pch
#	pragma hdrstop
#endif

/******************************************************************************/

const char gVersion[] = "@(#) NcFTP 3.2.1/349 Jul 29 2007, 09:55 AM";

/******************************************************************************/

#ifdef O_S
const char gOS[] = O_S;
#elif (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
const char gOS[] = "Windows";
#elif defined(__CYGWIN__)
const char gOS[] = "Cygwin";
#else
const char gOS[] = "UNIX";
#endif

/******************************************************************************/

const char gCopyright[] = "@(#) \
Copyright (c) 1992-2005 by Mike Gleason.\n\
All rights reserved.\n\
";

/******************************************************************************/
