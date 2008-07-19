#define	V	"11.25"
/*
 * Nail - a mail user agent derived from Berkeley Mail.
 *
 * Copyright (c) 2000-2004 Gunnar Ritter, Freiburg i. Br., Germany.
 */
/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
#ifdef	DOSCCS
static char sccsid[] = "@(#)version.c	2.331 (gritter) 7/29/05";
#endif
#endif /* not lint */

/*
 * Just keep track of the date/sid of this version of Mail.
 * Load this file first to get a "total" Mail version.
 */
/*char	*version = "8.1 6/6/93";*/
const char *version = "nail " V " 7/29/05";
#ifndef	lint
static const char *versionid
#ifdef	__GNUC__
__attribute__ ((unused))
#endif
= "@(#)nail " V " 7/29/05";
#endif	/* !lint */
/* SLIST */
/*
aux.c:static char sccsid[] = "@(#)aux.c	2.82 (gritter) 3/4/05";
base64.c:static char sccsid[] = "@(#)base64.c	2.12 (gritter) 7/20/05";
cache.c:static char sccsid[] = "@(#)cache.c	1.60 (gritter) 7/24/05";
cmd1.c:static char sccsid[] = "@(#)cmd1.c	2.95 (gritter) 4/15/05";
cmd2.c:static char sccsid[] = "@(#)cmd2.c	2.45 (gritter) 5/28/05";
cmd3.c:static char sccsid[] = "@(#)cmd3.c	2.81 (gritter) 4/13/05";
cmdtab.c:static char sccsid[] = "@(#)cmdtab.c	2.50 (gritter) 12/2/04";
collect.c:static char sccsid[] = "@(#)collect.c	2.51 (gritter) 7/13/05";
def.h: *	Sccsid @(#)def.h	2.103 (gritter) 7/29/05
dotlock.c:static char sccsid[] = "@(#)dotlock.c	2.7 (gritter) 10/2/04";
edit.c:static char sccsid[] = "@(#)edit.c	2.23 (gritter) 6/9/05";
extern.h: *	Sccsid @(#)extern.h	2.156 (gritter) 7/15/05
fio.c:static char sccsid[] = "@(#)fio.c	2.68 (gritter) 11/6/04";
getname.c:static char sccsid[] = "@(#)getname.c	2.4 (gritter) 10/2/04";
getopt.c:	Sccsid @(#)getopt.c	1.6 (gritter) 10/2/04	
glob.h: *	Sccsid @(#)glob.h	2.25 (gritter) 7/5/05
head.c:static char sccsid[] = "@(#)head.c	2.16 (gritter) 6/9/05";
hmac.c:	Sccsid @(#)hmac.c	1.7 (gritter) 10/2/04	
imap.c:static char sccsid[] = "@(#)imap.c	1.214 (gritter) 7/24/05";
imap_gssapi.c:static char sccsid[] = "@(#)imap_gssapi.c	1.9 (gritter) 10/2/04";
imap_search.c:static char sccsid[] = "@(#)imap_search.c	1.28 (gritter) 4/14/05";
junk.c:static char sccsid[] = "@(#)junk.c	1.72 (gritter) 7/14/05";
lex.c:static char sccsid[] = "@(#)lex.c	2.82 (gritter) 7/5/05";
list.c:static char sccsid[] = "@(#)list.c	2.59 (gritter) 3/5/05";
lzw.c: * Sccsid @(#)lzw.c	1.10 (gritter) 10/3/04
macro.c:static char sccsid[] = "@(#)macro.c	1.12 (gritter) 10/2/04";
maildir.c:static char sccsid[] = "@(#)maildir.c	1.18 (gritter) 7/5/05";
main.c:static char sccsid[] = "@(#)main.c	2.45 (gritter) 7/5/05";
md5.c:	Sccsid @(#)md5.c	1.7 (gritter) 10/2/04	
md5.h:	Sccsid @(#)md5.h	1.7 (gritter) 10/2/04	
mime.c:static char sccsid[]  = "@(#)mime.c	2.63 (gritter) 7/29/05";
names.c:static char sccsid[] = "@(#)names.c	2.21 (gritter) 6/9/05";
nss.c:static char sccsid[] = "@(#)nss.c	1.43 (gritter) 7/15/05";
openssl.c:static char sccsid[] = "@(#)openssl.c	1.23 (gritter) 7/15/05";
pop3.c:static char sccsid[] = "@(#)pop3.c	2.41 (gritter) 7/5/05";
popen.c:static char sccsid[] = "@(#)popen.c	2.19 (gritter) 12/26/04";
quit.c:static char sccsid[] = "@(#)quit.c	2.27 (gritter) 11/3/04";
rcv.h: *	Sccsid @(#)rcv.h	2.6 (gritter) 10/2/04
send.c:static char sccsid[] = "@(#)send.c	2.82 (gritter) 3/22/05";
sendout.c:static char sccsid[] = "@(#)sendout.c	2.88 (gritter) 7/29/05";
smtp.c:static char sccsid[] = "@(#)smtp.c	2.34 (gritter) 7/13/05";
ssl.c:static char sccsid[] = "@(#)ssl.c	1.37 (gritter) 7/15/05";
strings.c:static char sccsid[] = "@(#)strings.c	2.5 (gritter) 10/2/04";
temp.c:static char sccsid[] = "@(#)temp.c	2.7 (gritter) 10/2/04";
thread.c:static char sccsid[] = "@(#)thread.c	1.56 (gritter) 7/14/05";
tty.c:static char sccsid[] = "@(#)tty.c	2.27 (gritter) 7/13/05";
v7.local.c:static char sccsid[] = "@(#)v7.local.c	2.9 (gritter) 10/2/04";
vars.c:static char sccsid[] = "@(#)vars.c	2.10 (gritter) 10/2/04";
*/
