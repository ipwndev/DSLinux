/*
 * Standard include files for Mathomatic.
 *
 * Copyright (c) 1987-2005 George Gesslein II.
 */

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

#include "am.h"
#include "complex.h"
#include "proto.h"
#include "externs.h"
