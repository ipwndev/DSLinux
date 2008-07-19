/*
	rebuffer.c
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <assert.h>

#include <audiofile.h>
#include "afinternal.h"
#include "modules.h"
#include "pcm.h"
#include "util.h"
#include "units.h"

#define CHNK(X)
#define DEBG(X)

#ifndef min
#define min(a,b) ((a)<(b) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) ((a)>(b) ? (a) : (b))
#endif

/* ===== REBUFFERING modules (use templates) */

/* defines module floatrebufferv2f and floatrebufferf2v */

#define PRFX(word) float ## word
#define INITFUNC(word) _af_initfloat ## word
#define NAMESTRING(word) "float" #word
#define TYPE float
#include "rebuffer.template"
#undef PRFX
#undef INITFUNC
#undef NAMESTRING
#undef TYPE

/* defines module int2rebufferv2f and int2rebufferf2v */

#define PRFX(word) int2 ## word
#define INITFUNC(word) _af_initint2 ## word
#define NAMESTRING(word) "int2" #word
#define TYPE schar2
#include "rebuffer.template"
#undef PRFX
#undef INITFUNC
#undef NAMESTRING
#undef TYPE
