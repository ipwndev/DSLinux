/*
 * gl_getline.h
 *
 * Copyright (C) 1991, 1992, 1993 by Chris Thewalt (thewalt@ce.berkeley.edu)
 *
 * Permission to use, copy, modify, and distribute this software 
 * for any purpose and without fee is hereby granted, provided
 * that the above copyright notices appear in all copies and that both the
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "as is" without express or
 * implied warranty.
 *
 * Thanks to the following people who have provided enhancements and fixes:
 *   Ron Ueberschaer, Christoph Keller, Scott Schwartz, Steven List,
 *   DaviD W. Sanderson, Goran Bostrom, Michael Gleason, Glenn Kasten,
 *   Edin Hodzic, Eric J Bivona, Kai Uwe Rommel, Danny Quah, Ulrich Betzler
 */

/*
 * Note:  This version has been updated by
 *        Mike Gleason (http://www.NcFTP.com/contact/)
 */

#ifndef gl_getline_h
#define gl_getline_h

/* unix systems can #define POSIX to use termios, otherwise 
 * the bsd or sysv interface will be used 
 */

#define GL_BUF_SIZE 2048

/* Result codes available for gl_get_result() */
#define GL_OK 0				/* Valid line of input entered */
#define GL_EOF (-1)			/* End of input */
#define GL_INTERRUPT (-2)		/* User hit Ctrl+C */
#define GL_SUSPEND (-3)			/* User hit Ctrl+Z */

typedef int (*gl_in_hook_proc)(char *);
typedef int (*gl_out_hook_proc)(char *);
typedef int (*gl_tab_hook_proc)(char *, int, int *, size_t);
typedef size_t (*gl_strlen_proc)(const char *);
typedef char * (*gl_tab_completion_proc)(const char *, int);

char *gl_getline(char *);		/* read a line of input */
void gl_dispose(void);
void gl_setwidth(int);			/* specify width of screen */
void gl_setheight(int);			/* specify height of screen */
void gl_histadd(const char *const);	/* adds entries to hist */
void gl_tab_completion(gl_tab_completion_proc);
char *gl_local_filename_completion_proc(const char *, int);
void gl_set_home_dir(const char *homedir);
void gl_histsavefile(const char *const path);
void gl_histloadfile(const char *const path);
char *gl_getpass(const char *const prompt, char *const pass, int dsize);
int gl_get_result(void);

#ifndef _gl_getline_c_

extern gl_in_hook_proc gl_in_hook;
extern gl_out_hook_proc gl_out_hook;
extern gl_tab_hook_proc gl_tab_hook;
extern gl_strlen_proc gl_strlen;
extern gl_tab_completion_proc gl_completion_proc;
extern int gl_filename_quoting_desired;
extern const char *gl_filename_quote_characters;
extern int gl_ellipses_during_completion;
extern int gl_completion_exact_match_extra_char;
extern char gl_buf[GL_BUF_SIZE];

#endif /* ! _gl_getline_c_ */

#endif /* gl_getline_h */
