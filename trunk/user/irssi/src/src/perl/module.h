#include "common.h"

#ifdef NEED_PERL_H
#  include <EXTERN.h>
#  ifndef _SEM_SEMUN_UNDEFINED
#    define HAS_UNION_SEMUN
#  endif
#  include <perl.h>

#  undef _
#  undef PACKAGE

/* For compatibility with perl 5.004 and older */
#  ifndef ERRSV
#    define ERRSV GvSV(errgv)
#  endif

extern PerlInterpreter *my_perl; /* must be called my_perl or some perl implementations won't work */
#endif

#define MODULE_NAME "perl/core"

/* Change this every time when some API changes between irssi's perl module
   (or irssi itself) and irssi's perl libraries. */
#define IRSSI_PERL_API_VERSION 20011214
