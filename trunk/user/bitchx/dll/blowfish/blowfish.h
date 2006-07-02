#include "irc.h"
#include "struct.h"
#include "ircaux.h"
#include "ctcp.h"
#include "status.h"
#include "lastlog.h"
#include "server.h"
#include "screen.h"
#include "vars.h"
#include "misc.h"
#include "output.h"
#include "module.h"
#include "hash2.h"

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef WANT_TCL
#	include <tcl.h>
#	include "tcl_bx.h"

#ifndef STDVAR
#	define STDVAR (ClientData cd, Tcl_Interp *irp, int argc, char *argv[])
#endif

#ifndef BADARGS
#define BADARGS(nl,nh,example) \
	if ((argc<(nl)) || (argc>(nh))) { \
		Tcl_AppendResult(intp,"wrong # args: should be \"",argv[0], \
		(example),"\"",NULL); \
		return TCL_ERROR; \
	}
#endif /* BADARGS */

#endif /* WANT_TCL */

#define INIT_MODULE
#include "modval.h"

#define MAXKEYBYTES 56          /* 448 bits */
#define bf_N             16
#define noErr            0
#define DATAERROR         -1
#define KEYBYTES         8

#define UBYTE_08bits  unsigned char
#define UWORD_16bits  unsigned short

#  define UWORD_32bits  unsigned int
/*
#else
#  if SIZEOF_LONG==4
#  define UWORD_32bits  unsigned long
#  endif
#endif
*/
/* choose a byte order for your hardware */

#ifdef WORDS_BIGENDIAN
/* ABCD - big endian - motorola */
union aword {
  UWORD_32bits word;
  UBYTE_08bits byte [4];
  struct {
    unsigned int byte0:8;
    unsigned int byte1:8;
    unsigned int byte2:8;
    unsigned int byte3:8;
  } w;
};
#endif  /* WORDS_BIGENDIAN */

#ifndef WORDS_BIGENDIAN
/* DCBA - little endian - intel */
union aword {
  UWORD_32bits word;
  UBYTE_08bits byte [4];
  struct {
    unsigned int byte3:8;
    unsigned int byte2:8;
    unsigned int byte1:8;
    unsigned int byte0:8;
  } w;
};
#endif  /* !WORDS_BIGENDIAN */
