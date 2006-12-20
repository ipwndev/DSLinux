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

#ifndef _INCLUDES_H_
#define _INCLUDES_H_


#include "config.h"
#include "options.h"
#include "debug.h"

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/param.h> /* required for BSD4_4 define */
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/wait.h>

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <limits.h>
#include <netinet/in.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <syslog.h>
#include <netdb.h>
#include <ctype.h>
#include <stdarg.h>
#include <dirent.h>

#include <arpa/inet.h>

#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif

#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifdef HAVE_LASTLOG_H
#include <lastlog.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#ifdef HAVE_LIBUTIL_H
#include <libutil.h>
#endif

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#ifndef DISABLE_ZLIB
#include <zlib.h>
#endif

#ifdef HAVE_UTIL_H
#include <util.h>
#endif

#ifdef HAVE_SHADOW_H
#include <shadow.h>
#endif

#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#include "libtomcrypt/mycrypt_custom.h"
#include "libtommath/tommath.h"

#include "compat.h"

#ifndef HAVE_UINT16_T
#ifndef HAVE_U_INT16_T
typedef unsigned short u_int16_t;
#endif /* HAVE_U_INT16_T */
typedef u_int16_t uint16_t;
#endif /* HAVE_UINT16_T */

#ifndef LOG_AUTHPRIV
#define LOG_AUTHPRIV LOG_AUTH
#endif

#endif /* _INCLUDES_H_ */
