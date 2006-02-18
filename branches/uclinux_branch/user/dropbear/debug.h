/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "includes.h"

/* Debugging */

/* Work well for valgrind - don't clear environment, be nicer with signals
 * etc. Don't use this normally, it might cause problems */
/* #define DEBUG_VALGRIND */

/* Define this to print trace statements - very verbose */
/* #define DEBUG_TRACE */

/* All functions writing to the cleartext payload buffer call
 * CHECKCLEARTOWRITE() before writing. This is only really useful if you're
 * attempting to track down a problem */
#define CHECKCLEARTOWRITE() assert(ses.writepayload->len == 0 && \
		ses.writepayload->pos == 0)

/* Define this, compile with -pg and set GMON_OUT_PREFIX=gmon to get gmon
 * output when Dropbear forks. This will allow it gprof to be used.
 * It's useful to run dropbear -F, so you don't fork as much */
/*#define DEBUG_FORKGPROF*/

/* A couple of flags, not usually useful, and mightn't do anything */

/*#define DEBUG_KEXHASH*/
/*#define DEBUG_RSA*/

/* you don't need to touch this block */
#ifdef DEBUG_TRACE
#define TRACE(X) (dropbear_trace X)
#else /*DEBUG_TRACE*/
#define TRACE(X)
#endif /*DEBUG_TRACE*/

/* For testing as non-root on shadowed systems, include the crypt of a password
 * here. You can then log in as any user with this password. Ensure that you
 * make your own password, and are careful about using this. This will also
 * disable some of the chown pty code etc*/
/* #define DEBUG_HACKCRYPT "hL8nrFDt0aJ3E" */ /* this is crypt("password") */

#endif
