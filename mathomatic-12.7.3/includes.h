/*
 * Standard include files for Mathomatic.
 *
 * Copyright (C) 1987-2007 George Gesslein II.
 */

/* include files from /usr/include: */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#if	UNIX
#include <libgen.h>
#endif
#include <limits.h>
#include <math.h>
#include <setjmp.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#if	I18N
#include <libintl.h>
#endif
#if	READLINE
#include <readline/readline.h>
#endif

/* include files from the current directory: */
#include "am.h"		/* the main include file for Mathomatic, contains tunable parameters */
#include "complex.h"	/* floating point complex number arithmetic function prototypes */
#include "proto.h"	/* global function prototypes */
#include "externs.h"	/* global variable extern definitions */
