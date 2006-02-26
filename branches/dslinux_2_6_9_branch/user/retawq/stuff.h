/* retawq/stuff.h - general stuff
   This file is part of retawq (<http://retawq.sourceforge.net/>), a network
   client created by Arne Thomassen; retawq is basically released under certain
   versions of the GNU General Public License and WITHOUT ANY WARRANTY.
   Read the file COPYING for license details, README for program information.
   Copyright (C) 2001-2005 Arne Thomassen <arne@arne-thomassen.de>
*/

#ifndef __retawq_stuff_h__
#define __retawq_stuff_h__

/* Compile-time configuration */

#include ".config"

#undef OPTION_TG /* make sure it won't be used anywhere accidentally... */

#if OPTION_BUILTIN_DNS
#undef OPTION_THREADING
#define OPTION_THREADING 0 /* no need for threading */
#endif

#define CONFIG_ASYNC_DNS (OPTION_THREADING || OPTION_BUILTIN_DNS)
#define CONFIG_ANY_MAIL (OPTION_POP || OPTION_IMAP)

/* flags in OPTION_EXECEXT */
#define EXECEXT_SHELL (1)
#define EXECEXT_ALL (EXECEXT_SHELL)

/* flags in OPTION_EXECINT */
#define EXECINT_RELIAWFUL (1)

/* flags in CONFIG_BLOAT */
#define BLOAT_SSC (1)
#define BLOAT_UIIP (2)
#define BLOAT_HELP (4)
#define BLOAT_HELPARG (8)
#define BLOAT_COLORHL (16)
#define BLOAT_SCROLL_BARS (32)
#define BLOAT_TERMWINTITLE (64)
#define BLOAT_ICONLOGO (128)
#define BLOAT_COLORS (256)

/* flags in CONFIG_HTTP */
#define HTTP_11 (1)
#define HTTP_PUT (2)
#define HTTP_AUTH_BASIC (4)
#define HTTP_AUTH_DIGEST (8)
#define HTTP_AUTH_ANY (HTTP_AUTH_BASIC | HTTP_AUTH_DIGEST) /* any non-proxy */
#define HTTP_PROXYAUTH (16)

/* flags in CONFIG_HTML */
#define CONFIG_HTML_COLORNAMES 0 /* (CONFIG_HTML & 3) */
#define HTML_TABLES (4)
#define HTML_FRAMES (8)
#define HTML_FORM_FILE_UPLOAD (16)

/* flags in CONFIG_MENUS */
#define MENUS_CONTEXT (1)
#define MENUS_UHIST (2)
#define MENUS_WLIST (4)
#define MENUS_HTML (8)
#define MENUS_BAR (16)
#define MENUS_ALL (31) /* (sum of all the above) */

/* flags in CONFIG_EXTRA */
#define EXTRA_DOWNLOAD (1)
#define EXTRA_DUMP (2)
#define EXTRA_PAGER (4)

#define DO_PAGER ( (CONFIG_DEBUG) && (CONFIG_EXTRA & EXTRA_PAGER) )

#if CONFIG_PLATFORM == 1
#undef const
#define const
#endif


/* Text/graphics configuration I: numerical values for CONFIG_TG */

#define TG_INVALID 0
#define TG_BACKGROUND 1
#define TG_CONSOLE 2
#define TG_CURSES 3
#define TG_NCURSES 4
#define TG_XCURSES 5
#define TG_BICURSES 6
#define TG_FLEXCURSES 7
#define TG_X 8
#define TG_GTK 9
#define TG_SVGA 10
#define TG_FRAMEBUFFER 11

/* text/graphics classes, for convenience */
#define TGC_IS_CURSES ( (CONFIG_TG == TG_CURSES) || (CONFIG_TG == TG_NCURSES) || (CONFIG_TG == TG_XCURSES) || (CONFIG_TG == TG_BICURSES) || (CONFIG_TG == TG_FLEXCURSES) )
#define TGC_IS_GRAPHICS ( (CONFIG_TG == TG_X) || (CONFIG_TG == TG_GTK) )
#define TGC_IS_PIXELING ( (TGC_IS_GRAPHICS) || (CONFIG_TG == TG_SVGA) || (CONFIG_TG == TG_FRAMEBUFFER) )
#define TGC_IS_WINDOWING (CONFIG_TG > TG_CONSOLE) /* virtual or real windows */

#if CONFIG_TG == TG_CONSOLE /* default runmode is console, so enable console */
#undef CONFIG_CONSOLE
#define CONFIG_CONSOLE 1
#endif

/* window kinds */
#define DO_WK_INFO    ((CONFIG_WK & 1) && (TGC_IS_CURSES))
#define DO_WK_CUSTOM_CONN (CONFIG_DEBUG && (CONFIG_WK & 2) && (TGC_IS_CURSES) && (CONFIG_FTP))
#define DO_WK_EDITOR  (CONFIG_DEBUG && (CONFIG_WK & 4) && (TGC_IS_CURSES))
#define DO_WK_FILEMGR (CONFIG_DEBUG && (CONFIG_WK & 8) && (TGC_IS_CURSES))
#define DO_WK_MAILMGR (CONFIG_DEBUG && (CONFIG_WK & 16) && (TGC_IS_CURSES) && (CONFIG_ANY_MAIL))

#define CONFIG_MAILTO ( (DO_WK_EDITOR) /* && (OPTION_SMTP) */ )
#define CONFIG_CUSTOM_CONN ((CONFIG_CONSOLE) || (DO_WK_CUSTOM_CONN))

#if CONFIG_CUSTOM_CONN
#if (!CONFIG_FTP)
#error "You enabled custom connections (CONFIG_CONSOLE) but disabled CONFIG_FTP. This cannot work."
#endif
#if (!(CONFIG_EXTRA & EXTRA_DOWNLOAD))
#error "You enabled custom connections (CONFIG_CONSOLE) but disabled downloads in CONFIG_EXTRA. This cannot work."
#endif
#endif

#define OFWAX 0

#define MIGHT_USE_SCROLL_BARS ( (CONFIG_TG == TG_XCURSES) && (CONFIG_BLOAT & BLOAT_SCROLL_BARS) )


/* Standard header file includes */

#include <stdio.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#else
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#endif

#if HAVE_STRING_H
#if (!STDC_HEADERS) && HAVE_MEMORY_H
#include <memory.h>
#endif
#include <string.h>
#endif

#if HAVE_STRINGS_H
#include <strings.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_ERRNO_H
#include <errno.h>
#endif

#if HAVE_LIMITS_H
#include <limits.h>
#endif

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_TERMIOS_H && ( (CONFIG_CONSOLE) || (CONFIG_TG == TG_BICURSES) )
#include <termios.h>
#define MIGHT_DO_TERMIOS 1
#else
#define MIGHT_DO_TERMIOS 0
#endif

typedef unsigned char tUint8;
typedef unsigned int tUint32;
typedef signed short tSint16;


/* Strings */

#define STRBUF_SIZE (4096)
extern const char strCopyright[], strProgramLegalese[], strRetawq[],
  strOptHelp[], strOptVersion[], strProgramVersion[], strText[], strGraphics[];
extern /*@observer@*/ const char strEmpty[], strNewline[];
extern const char strMinus[], strSlash[], strAsterisk[], strLt[], strGt[],
  strHm[], strQm[], strDoubleQuote[], strSpacedDash[], strDoubleDot[],
  str1dot0[], str1dot1[];
extern const char strPercsPercs[], strPercd[], strBracedNumstr[];
extern const char strHexnum[];
extern const char strA[], strShtml[], strReset[], strSelect[], strSubmit[],
  strOn[], strClose[], strQuit[], strGet[], strPost[], strLocalhost[];
extern const char strUnknown[], strBracedDisabled[], strImage[], strCheckbox[],
  strButton[], strYes[], strNo[], strFileUc[], strDirectoryUc[], strLink[],
  strLoginFailed[], strFwdact[], strByte[], strBytes[], strOff[],strBadValue[],
  strErrorTrail[];
#if !CONFIG_SESSIONS
extern const char strFsessions[];
#endif
#if !CONFIG_JUMPS
extern const char strFjumps[];
#endif
#if OPTION_EXECEXT != EXECEXT_ALL
extern const char strFexecext[];
#endif
#if CONFIG_CONSOLE
extern const char strConsoleDiscard[];
#endif
#if (CONFIG_TG == TG_X) || (CONFIG_TG == TG_XCURSES)
extern const char strCantOpenXws[];
#endif
#if OPTION_LOCAL_CGI || OPTION_TRAP || (OPTION_EXECEXT & EXECEXT_SHELL)
extern const char strSoftwareId[];
#endif

/* ugly :-) */
#define strHelp    (strOptHelp + 2)
#define strVersion (strOptVersion + 2)
#define strSpace   (strSpacedDash + 2)
#define strPercs   (strPercsPercs + 2)
#define strHtml    (strShtml + 1)

/* scheme strings: */
extern const char strHttp[], strFtp[], strLocal[], strFile[], strAbout[],
  strFinger[], strLocalCgi[], strNntp[], strNews[], strCvs[], strGopher[],
  strInfo[], strMailto[], strJavascript[], strHttps[], strFtps[], strPop[],
  strPops[], strExecextShell[];


/* Helper macros */

#define CONFIG_EXDEBUG 0

#ifndef NULL
#define NULL ((void*) 0)
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#endif

#ifdef INT_MAX
#define MY_ATOI_INT_MAX (INT_MAX / 11)
  /* (avoids overflows in my_atoi() and is still big enough for any "real-life"
      case) */
#else
#define MY_ATOI_INT_MAX (99999999)
#endif

/* compare two numbers; in Perl we'd use the "<=>" operator... */
#define my_numcmp(num1, num2) \
  ( ((num1) > (num2)) ? (+1) : ( ((num1) < (num2)) ? (-1) : (0) ) )

#ifdef S_SPLINT_S

/* make splint happy; __sunused and __sallocator are splint-specific variants
   of __cunused and __callocator; this rubbish is necessary because splint and
   gcc expect such annotations in different places within a declaration... */

#define __sunused /*@unused@*/
#define __dnr /*@noreturn@*/
#define __sallocator /*@only@*/
#define my_enum1 typedef
#define my_enum2(ctype)

#else

/* make _me_ happy :-) */

#define __sunused
#define __dnr
#define __sallocator
#define my_enum1
#define my_enum2(ctype) ; typedef ctype

#endif

#if USING_CONFIGURE
#define __my_inline inline
#elif defined(__GNUC__)
#define __my_inline inline
#else
#define __my_inline /* nothing (we don't know for _sure_ how to do it) */
#endif

#ifdef DO_NOT_INLINE
#define my_inline /* nothing (to make the program smaller) */
#else
#define my_inline __my_inline
#endif

#if defined(__GNUC__) && ( (__GNUC__ > 2) || ( (__GNUC__ == 2) && (__GNUC_MINOR__ >= 5) ) )
#define __cunused __attribute__((__unused__))
#define my_spf_attr __attribute__((__format__(printf, 4, 5))) /* shows bugs */
#define does_not_return __attribute__((__noreturn__))
#if CONFIG_EXDEBUG
#define __init __attribute__((__section__ ("textinit")))
#endif
/* #define __callocator __attribute__((__malloc__)) */
#else
#define __cunused
#define my_spf_attr
#endif

#ifndef __init
#define __init
#endif
#ifndef __initdata
#define __initdata
#endif
#ifndef __callocator
#define __callocator
#endif
#ifndef does_not_return
#define does_not_return __dnr
#endif
#define const_after_init
#if CONFIG_DEBUG
#define one_caller /* nothing (gcc/gdb and line numbers...) */
#else
#define one_caller __my_inline /* (function is called from only one place) */
#endif

/* Mark that we checked that the library function won't overflow the buffer: */
#define sprint_safe sprintf

/* Some snprintf() implementations in C libraries are rubbish. Just one example
   from a Linux manual page: "Linux libc4.[45] does not have a snprintf, but
   provides a libbsd that contains an snprintf equivalent to sprintf, i.e.,
   one that ignores the size argument. Thus, the use of snprintf with early
   libc4 leads to serious security problems." Congratulations... */
#define snprint_safe DOES_NOT_EXIST__DO_NOT_USE_SNPRINTF /* clear? :-) */

/* Some sscanf() implementations in C libraries are rubbish: they try to write
   into the <format> parameter, causing a crash if operating on read-only
   string constants. (Information found in "info gcc".) Thus: */
#if 1
#define PREPARE_SSCANF_FORMAT(bufvar, bufsize, formatstr) \
  char bufvar[bufsize]; strcpy(bufvar, formatstr); /* for buggy libraries */
#else
#define PREPARE_SSCANF_FORMAT(bufvar, bufsize, formatstr) \
  static const char bufvar[] = formatstr; /* for non-buggy libraries */
#endif

#define zero2one(x) ( ((x) == 0) ? (1) : (x) )
#define bytebytes(x) ( ((x) == 1) ? _(strByte) : _(strBytes) )
#define null2empty(str) ( (str == NULL) ? (strEmpty) : (str) )
#define __unconstify(_t, _v) ( (_t) (_v) ) /* discard "const", avoid warning */
#define unconstify(str) __unconstify(char*, str)

#define is_control_char(unsigned_char) \
  ( ((unsigned_char) < ' ') || ((unsigned_char) == 127) )
#define TO_EOS(str) do { while (*str != '\0') str++; } while (0)
  /* go to end of string */

#define ARRAY_ELEMNUM(arr) (sizeof(arr) / sizeof(arr[0]))

/* We need library function variants which are _guaranteed_ to be
   locale-independent, even if i18n is enabled. */
#define my_isdigit(ch) ( ((ch) >= '0') && ((ch) <= '9') )
#define my_islower(ch) ( ((ch) >= 'a') && ((ch) <= 'z') )
#define my_isupper(ch) ( ((ch) >= 'A') && ((ch) <= 'Z') )
#define my_isalpha(ch) (my_islower(ch) || my_isupper(ch))
#define my_isalnum(ch) (my_isalpha(ch) || my_isdigit(ch))
#define my_tolower(ch) ( my_isupper(ch) ? ((ch) + ('a' - 'A')) : (ch) )
#define my_toupper(ch) ( my_islower(ch) ? ((ch) - ('a' - 'A')) : (ch) )


/* Boolean */

/* It seems that "false", "False" and "FALSE" are already used by compilers and
   libraries; it can't be true... :-) Let's hope that our choice is safe. */
/*@-booltype tBoolean@*/ /*@-booltrue truE@*/ /*@-boolfalse falsE@*/
my_enum1 enum { falsE = 0, truE = 1 } my_enum2(unsigned char) tBoolean;
#define flip_boolean(var) \
  do { var = ( (var == falsE) ? (truE) : (falsE) ); } while (0)
#define boolean2bool(x) ( ((x) == falsE) ? (0) : (1) )
#define cond2boolean(condition) ( (condition) ? (truE) : (falsE) )
#define cond2bool(condition) ( (condition) ? (1) : (0) )


/* Text/graphics configuration II */

#define MIGHT_USE_COLORS (CONFIG_BLOAT & BLOAT_COLORS)

#include "tgmode.inc"

#if MIGHT_DO_TERMIOS
extern tBoolean do_restore_termios;
extern struct termios saved_termios;
#endif

#if TGC_IS_GRAPHICS

/* We are Pixels of Borg - curses is irrelevant. */
#undef OPTION_CED
#define OPTION_CED 1

#define strTG strGraphics

#if CONFIG_TG == TG_X
#include <X11/Xlib.h>
extern Display* xws_display;
typedef unsigned int tKey;
typedef int tCoordinate;
#else
#include <gtk/gtk.h>
typedef guint tKey; /*struct _GdkEventKey->keyval,/usr/include/gdk/gdktypes.h*/
typedef gint16 tCoordinate; /* struct _GdkPoint, gtk/gdk/gdktypes.h */
#endif

#else /* #if TGC_IS_GRAPHICS */

#define strTG strText

typedef int tKey;
typedef signed short tCoordinate;

#endif /* #if TGC_IS_GRAPHICS */

#if CONFIG_TG == TG_XCURSES
#undef OPTION_CED
#define OPTION_CED 1
#endif

#if (OPTION_TEXTMODEMOUSE) && ( ( (TGC_IS_CURSES) && (defined(NCURSES_MOUSE_VERSION)) ) || (CONFIG_TG == TG_XCURSES) )
/* "user wants" && "library can" */
#define CONFIG_DO_TEXTMODEMOUSE 1
#define TEXTMODEMOUSE_MASK (BUTTON1_CLICKED | BUTTON2_CLICKED |BUTTON3_CLICKED)
#else
#define CONFIG_DO_TEXTMODEMOUSE 0
#endif

extern tBoolean need_tglib_cleanup;
extern const_after_init unsigned char __lfdmbs;
#define lfdmbs(x) (__lfdmbs & (1 << (x)))


/* i18n configuration; this must be done _after_ OPTION_CED (re-)definition */

#if (OPTION_I18N) && (OPTION_CED == 0)
/* We need special code to make translated texts seven-bit clean: */

#define NEED_SPECIAL_GETTEXT 1
extern char* my_do_gettext(char** buffer, const char* str);
#define declare_local_i18n_buffer static char* local_i18n_buffer = NULL;
#define my_gettext(str) my_do_gettext(&local_i18n_buffer, str)
#define i18n_strdup(str) my_strdup(str) /* orig destroyed by i18n_cleanup */
#define i18n_cleanup \
  if (local_i18n_buffer != NULL) (void) my_do_gettext(&local_i18n_buffer,NULL);

#else

#define NEED_SPECIAL_GETTEXT 0
#define declare_local_i18n_buffer /* nothing */
#define my_gettext(str) gettext(str)
#define i18n_strdup(str) (str) /* no need to copy */
#define i18n_cleanup

#endif

#if OPTION_I18N

#if HAVE_LIBINTL_H
#include <libintl.h>
#endif

#undef _
#ifndef _
#define _(str) my_gettext(str)
#endif

#ifndef N_
#ifdef gettext_noop
#define N_(str) gettext_noop(str)
#else
#define N_(str) (str)
#endif
#endif
#define unconstify_or_(str) _(str)

#else /* #if OPTION_I18N */

#define _(str) (str)
#define N_(str) (str)
#define unconstify_or_(str) unconstify(str)

#endif /* #if OPTION_I18N */

#define localized_size(size) (size) /* IMPLEMENTME! */


/* Curses */

#if MIGHT_USE_COLORS
extern tBoolean use_colors;
#endif

#if TGC_IS_CURSES

#define CURSES_MINCOLS (30)
#define CURSES_MINLINES (10)
#define CURSES_MAXCOLS (250)
#define CURSES_MAXLINES (250)
/* (All values must fit into an "unsigned char".) */

#if MIGHT_USE_COLORS

my_enum1 enum
{ ccBlack = 0, ccRed = 1, ccGreen = 2, ccYellow = 3, ccBlue = 4, ccMagenta = 5,
  ccCyan = 6, ccWhite = 7
} my_enum2(unsigned char) tColorCode;

my_enum1 enum
{ /* cpnHardwired = 0, -- not usable for us */ cpnDefault = 1, cpnRed = 2,
  cpnGreen = 3, cpnYellow = 4, cpnBlue = 5
} my_enum2(short) tColorPairNumber;
#define cpnMax (5)

typedef chtype tColorBitmask;
extern tColorBitmask color_bitmask[cpnMax + 1];

#define my_color_attr(cpn) (color_bitmask[cpn]) /*only do this if use_colors!*/
#define my_set_color(cpn) \
  do { if (use_colors) { (void) attron(my_color_attr(cpn)); } } while (0)

#else /* colors */

#define my_set_color(cpn) do { } while (0)

#endif /* colors */

#endif /* curses */


/* HTML forms; active elements; content */

typedef unsigned int tHtmlInputLength;
#define MAX_HTML_INPUT_LENGTH (STRBUF_SIZE / 2)

enum
{ aekUnknown = 0, aekLink = 1, aekFormText = 2, aekFormPassword = 3,
  aekFormCheckbox = 4, aekFormRadio = 5, aekFormSubmit = 6, aekFormReset = 7,
  aekFormFile = 8, aekFormTextarea = 9, aekFormSelect = 10, aekFormButton = 11,
  aekFormImage = 12, aekFormHidden = 13
};
#define MAX_AEK (13) /* associated UI strings in main.c */
typedef unsigned char tActiveElementKind;
#define is_form_aek(kind) \
  ( ((kind) >= aekFormText) && ((kind) <= aekFormHidden) )
#define has_input_length(kind) \
  ( ((kind) == aekFormText) || ((kind) == aekFormPassword) )

enum
{ aefNone = 0, aefCheckedSelected = 0x01, aefDisabled = 0x02,
  aefReadonly = 0x04, aefMultiple = 0x08, aefButtonTag = 0x10
};
typedef unsigned char tActiveElementFlags;
#define aefChangeable (aefCheckedSelected)

typedef signed int tActiveElementNumber;
#define INVALID_AE (-1)

typedef signed int tHtmlFormNumber;
#define INVALID_HTML_FORM_NUMBER (-1)

#if CONFIG_JAVASCRIPT
struct tJavascriptEventHandler;
#endif

typedef struct
{ const char *data, /* an attribute value - href for links, name otherwise */
    *render; /* e.g. text to be rendered for this active element */
#if CONFIG_JAVASCRIPT
  const struct tJavascriptEventHandler* eh;
#endif
  tHtmlInputLength size, maxlength;
  tActiveElementKind kind;
  tActiveElementFlags flags;
} tActiveElementBase;
/* tActiveElementBase contains "constant" information which only depends on the
   parsed resource (HTML source code), e.g. the target of a link or the maximum
   length of a text-input element, while tActiveElement (in main.c) contains
   view-dependent information like layout coordinates and current contents of
   text-input elements. */
/* For aekFormSelect kind, <render> points to the list of options, and
   <maxlength> is the number of options. */

enum { hffNone = 0, hffMethodPost = 0x01, hffEncodingMultipart = 0x02 };
typedef unsigned char tHtmlFormFlags;

typedef struct
{ const char* action_uri;
  tActiveElementNumber first_ae, last_ae;
  tHtmlFormFlags flags;
} tHtmlForm;

typedef struct tContentblock
{ struct tContentblock* next;
  char* data;
  size_t usable, used;
} tContentblock;

my_enum1 enum
{ rckUnknown = 0, rckPlainText = 1, rckHtml = 2
} my_enum2(unsigned char) tResourceContentKind; /* CHECKME: rename! */

my_enum1 enum
{ cafNone = 0, cafNeedUnmap = 0x01, cafHtmlRedirection = 0x02
} my_enum2(unsigned char) tCantentFlags;

/* a can/tent for con-tent; or maybe an abbr. for "can't entice"? :-) */
typedef struct
{ tContentblock *content, *lastcontent, *lhpp_content;
  void* tree; /* built from <content> (e.g. tree of HTML tags) */
  tActiveElementBase* aebase;
  tHtmlForm* form;
  const char *redirection, *major_html_title;
  size_t lhpp_byte; /* last HTML parser position: byte within content block */
  size_t refcount;
  tActiveElementNumber aenum, aemax;
  tHtmlFormNumber hfnum, hfmax;
  tResourceContentKind kind;
  tCantentFlags caf;
} tCantent;


/* To have or not to have, that is the question! :-) */

#if HAVE_MEMCPY
#define my_memcpy(dest, src, cnt) (void) memcpy(dest, src, cnt)
#elif HAVE_BCOPY
#define my_memcpy(dest, src, cnt) bcopy(src, dest, cnt)
#else
#define my_memcpy(dest, src, cnt) \
  do \
  { char* _d = (char*) (dest); \
    const char* _s = (const char*) (src); \
    size_t _c = (cnt); \
    while (_c-- > 0) *_d++ = *_s++; \
  } while (0)
#endif

#if HAVE_MEMSET
#define my_memset(start, value, length) memset(start, value, length)
#else
#define my_memset(start, value, length) \
  do \
  { char* _s = (char*) (start); \
    const char _v = (char) (value); \
    size_t count = (length); \
    while (count-- > 0) *_s++ = _v; \
  } while (0)
#endif

#if (!HAVE_MEMSET) && (HAVE_BZERO)
  /* Use bzero() because it might be much faster than our stupid byte-by-byte
     cleaning fallback; but use it only in this case -- it's marked "legacy",
     SUSv3 says: "The memset() function is preferred over this function." */
#define my_memclr(start, length) bzero(start, length)
#else
#define my_memclr(start, length) my_memset(start, 0, length)
#endif

#define my_memclr_var(var) my_memclr(&(var), sizeof(var))
#define my_memclr_arr(arr) my_memclr((arr), sizeof(arr))

#if HAVE_MMAP

#include <sys/mman.h>
#define my_mmap(len, fd) mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0)
#define my_munmap(start, len) (void) munmap(start, len)

#else /* #if HAVE_MMAP */

#ifndef MAP_FAILED
#define MAP_FAILED ((void*) (-1))
#endif

extern void* nonbroken_mmap(size_t, int);
#define my_mmap(len, fd) nonbroken_mmap(len, fd)
#define my_munmap(start, len) memory_deallocate(start)

#endif /* #if HAVE_MMAP */

#if defined(MADV_SEQUENTIAL)
#define my_madvise_sequential(ptr, size) \
  (void) madvise(ptr, size, MADV_SEQUENTIAL)
#elif defined(POSIX_MADV_SEQUENTIAL)
#define my_madvise_sequential(ptr, size) \
  (void) posix_madvise(ptr, size, POSIX_MADV_SEQUENTIAL)
#else
#define my_madvise_sequential(ptr, size) do { } while (0)
#endif

#if HAVE_STRCASECMP
#define my_strcasecmp(s1, s2) strcasecmp(s1, s2)
#else
extern int nonbroken_strcasecmp(const char*, const char*);
#define my_strcasecmp(s1, s2) nonbroken_strcasecmp(s1, s2)
#endif

#if 0
#if HAVE_STRNCASECMP
#define my_strncasecmp(s1, s2, n) strncasecmp(s1, s2, n)
#else
extern int nonbroken_strncasecmp(const char*, const char*, size_t);
#define my_strncasecmp(s1, s2, n) nonbroken_strncasecmp(s1, s2, n)
#endif
#endif

#if HAVE_STRCHR
#define my_strchr(str, ch) strchr(str, ch)
#else
extern char* nonbroken_strchr(const char*, int);
#define my_strchr(str, ch) nonbroken_strchr(str, ch)
#endif

#if HAVE_STRERROR
#define my_strerror(errnum) strerror(errnum) /* CHECKME: use strerror_r()? */
#define __my_strerror(errnum) my_strerror(errnum)
#else
#define my_strerror(errnum) _(strUnknown) /* CHECKME! */
#define __my_strerror(errnum) strUnknown /* CHECKME! */
#endif

#if HAVE_STRRCHR
#define my_strrchr(str, ch) strrchr(str, ch)
#else
extern char* nonbroken_strrchr(const char*, int);
#define my_strrchr(str, ch) nonbroken_strrchr(str, ch)
#endif

#if HAVE_STRSTR
#define my_strstr(haystack, needle) strstr(haystack, needle)
#else
extern char* nonbroken_strstr(const char*, const char*);
#define my_strstr(haystack, needle) nonbroken_strstr(haystack, needle)
#endif


/* Miscellaneous */

/* platform-specific settings */
#define USE_LWIP (CONFIG_PLATFORM == 1)
#define NEED_FD_REGISTER ( (USE_LWIP) /* || (....) */ )
#define CAN_HANDLE_SIGNALS (CONFIG_PLATFORM != 1)
#define CAN_ISSUE_WARNINGS (CONFIG_PLATFORM != 1)
#if CONFIG_PLATFORM == 1
#define chDirsep ('_')
#define strDirsep "_"
#else
#define chDirsep ('/')
#define strDirsep strSlash
#endif

#if OPTION_LOCAL_CGI || OPTION_EXECEXT || OFWAX
#define MIGHT_FORK_EXEC 1
#define __initfe /* function not only used at initialization time */
#else
#define MIGHT_FORK_EXEC 0
#define __initfe __init /* function only used at initialization time */
#endif

#if MIGHT_FORK_EXEC && defined(FD_CLOEXEC)
extern void make_fd_cloexec(int);
#else
#define make_fd_cloexec(fd) do { } while (0)
#endif

#define TLS_GNUTLS  (1)
#define TLS_OPENSSL (2)
#define TLS_MATRIX  (3)
#define TLS_BUILTIN (4)

#if OPTION_TLS
#define strTls ", tls"
#else
#define strTls ""
#endif

#if OPTION_TLS == TLS_GNUTLS
#define strTLS "TLS (GnuTLS)"
#elif OPTION_TLS == TLS_OPENSSL
#define strTLS "TLS (OpenSSL)"
#elif OPTION_TLS == TLS_MATRIX
#define strTLS "TLS (MatrixSSL)"
#elif OPTION_TLS
#define strTLS "TLS"
#endif

my_enum1 enum
{ pmEnvironed = 0
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
  , pmDownload = 1
#endif
#if CONFIG_EXTRA & EXTRA_DUMP
  , pmDump = 2
#endif
#if CONFIG_CONSOLE
  , pmConsole = 3
#endif
#if DO_PAGER
  , pmPager = 4
#endif
} my_enum2(unsigned char) tProgramMode;
extern const_after_init tProgramMode program_mode;
#define is_environed (program_mode == pmEnvironed) /* using curses/GTK/... */

#if CONFIG_CONSOLE
#define is_promptable ( (is_environed) || (program_mode == pmConsole) )
#else
#define is_promptable (is_environed)
#endif

extern /*@null@*/ const char* initial_messages;
#if CONFIG_CONSOLE
extern /*@null@*/ const char* initial_console_msgs;
#endif
extern unsigned char launch_uri_count;
extern const char* launch_uri[2];

my_enum1 enum
{ ftUnknown = 0, ftDontOverwrite = 1, ftMayOverwrite = 2, ftAppend = 3
} my_enum2(unsigned char) tFileTreatment;

typedef unsigned char tRgbPart;

/* A fast, flexible binary search algorithm; the standard library function
   bsearch() only returns a useless _member_ of the array, while we want to get
   an array _index_ by which we can access key, value, member, neighbor member
   or whatever else we just need. */
typedef signed short tMbsIndex;
#define INVALID_INDEX ((tMbsIndex) (-1))
#define my_binary_search(_min_index, _max_index, comparison, action) \
  { tMbsIndex min = _min_index, max = _max_index, idx; \
    do \
    { int res; \
      idx = (min + max) / 2; \
      res = comparison; /* depends on <idx> */ \
      if (res < 0) max = idx - 1; \
      else if (res > 0) min = idx + 1; \
      else { perform_action: action; } \
    } while (min <= max); \
    idx = INVALID_INDEX; goto perform_action; /* not found */ \
  }

/* Simple bit manipulation helpers; <address> is the start address of a
   bitfield, <bit> is the number of the desired bit within the field. */
#define __my_byte(address, bit) (*((unsigned char*) ((address) + ((bit) / 8))))
#define __my_bit(bit) (1 << ((bit) % 8))
#define my_bit_set(address, bit) __my_byte(address, bit) |= __my_bit(bit)
#define my_bit_clear(address, bit) __my_byte(address, bit) &= ~__my_bit(bit)
#define my_bit_flip(address, bit) __my_byte(address, bit) ^= __my_bit(bit)
#define my_bit_test(address, bit) (__my_byte(address, bit) & __my_bit(bit))

#define list_reverse(_head, _type) \
  do \
  { _type *head = NULL, *temp = _head; \
    while (temp != NULL) \
    { _type* next = temp->next; \
      temp->next = head; \
      head = temp; \
      temp = next; \
    } \
    _head = head; \
  } while (0)

#define list_extract(_headaddr0, _entry, _type) \
  do \
  { _type **headaddr0 = _headaddr0, *head0 = *headaddr0, *entry = _entry; \
    if ( (head0 == NULL) || (entry == NULL) ) { /* "should not happen" */ } \
    else if (head0 == entry) *headaddr0 = head0->next; \
    else \
    { while (head0->next != NULL) \
      { if (head0->next == entry) { head0->next = entry->next; break; } \
        head0 = head0->next; \
      } \
    } \
  } while (0)

/* CHECKME: use a sizeof() test with configure to avoid (_bogus_!) compiler
   warnings on 64-bit machines! */
#define MY_INT_TO_POINTER(x) ( (void*) ((int) (x)) )
#define MY_POINTER_TO_INT(x) ((int) (x))

/* Suppress _bogus_ compiler warnings about uninitialized variables (in order
   to make all "real" warnings more visible): */
#if 1
#define SHUT_UP_COMPILER(value) = (value)
#else
#define SHUT_UP_COMPILER(value) /* nothing */
#endif

#if CONFIG_DEBUG
extern const_after_init int debugfd;
#define __debugmsg(msg, len) (void) my_write(debugfd, msg, len)
#define debugmsg(_dbgmsg) \
  do \
  { const char* dbgmsg = (_dbgmsg); /* (evaluate it only once) */ \
    __debugmsg(dbgmsg, strlen(dbgmsg)); \
  } while (0)
#else
#define __debugmsg(msg, len) do { } while (0)
#define debugmsg(msg) do { } while (0)
#endif

#define __exdebugmsg(msg, len) do { } while (0)
#define exdebugmsg(msg) do { } while (0)

extern const_after_init int fd_stdin, fd_stdout, fd_stderr, fd_keyboard_input;

typedef unsigned short tPortnumber;
/* (The correct formal type would be in_port_t, but this seems to be
    not-so-portable; e.g. glibc before 2000-04-01 doesn't have it.) */

#define __dealloc(ptr) do { if (ptr != NULL) memory_deallocate(ptr); } while(0)
#define dealloc(ptr) \
  do { if (ptr != NULL) { memory_deallocate(ptr); ptr = NULL; } } while (0)
#define my_strdedup(dest, src) /* deallocate and duplicate */ \
  do { __dealloc(dest); (dest) = my_strdup(src); } while (0)

/* some dumb library headers define what we need as variable identifiers... */
#undef min
#undef max
#undef _min
#undef _max


/* Data handling mechanism */

my_enum1 enum
{ dhmcInit = 0, dhmcGet = 1, dhmcPut = 2, dhmcNotificationSetup = 3,
  dhmcNotify = 4, dhmcControl = 5
} my_enum2(unsigned char) tDhmCommand;

my_enum1 enum
{ dhmnfNone = 0, dhmnfRemoval = 0x01, dhmnfOnce = 0x02, dhmnfDataChange = 0x04,
  dhmnfMetadataChange = 0x08, dhmnfAttachery = 0x10
} my_enum2(unsigned char) tDhmNotificationFlags;
#define dhmnfGotData (dhmnfDataChange)
#define dhmnfAll ((dhmnfMetadataChange << 1) - 1)
/* dhmnfOnce is semantically a simplified, fast combination of dhmnfGotData,
   dhm_notification_off() and dhmnfRemoval. */

typedef void (*tDhmNotificationCallback)(void*, tDhmNotificationFlags);

typedef struct tDhmNotificationEntry
{ struct tDhmNotificationEntry* next;
  tDhmNotificationCallback callback;
  void* callback_data;
  tDhmNotificationFlags flags;
} tDhmNotificationEntry;

my_enum1 enum
{ dhmccRefcount0 = 0, dhmccFirstUnused = 1
} my_enum2(unsigned char) tDhmControlCode;

typedef void (*tDhmControlHandler)(void* /*obj*/, void* /*ctrldata*/,
  tDhmControlCode);

typedef struct
{ void* data;
  tDhmControlCode code;
} tDhmControlData;

typedef struct
{ void* object;
  tDhmControlHandler control_handler;
  tDhmNotificationEntry* notifications;
#if CONFIG_DEBUG
  const char* debugstr;
#endif
  size_t refcount;
} tDhmGenericData;

my_enum1 enum
{ dhmnSet = 0, dhmnOr = 1, dhmnAndnot = 2, dhmnXor = 3
} my_enum2(unsigned char) tDhmNotimode;

typedef struct
{ tDhmNotificationCallback callback;
  void* callback_data;
  tDhmNotificationFlags flags;
  tDhmNotimode mode;
} tDhmNotificationSetup;

#if 0
typedef void (*tDhmHandler)(tDhmGenericData**, const void*, tDhmCommand);
extern tDhmHandler dhm_generic_handler;
#else
extern void dhm_generic_handler(tDhmGenericData**, const void*, tDhmCommand);
#endif

#define __dhm_handler(obj) (dhm_generic_handler)
  /* (might become e.g. ((obj)->dhm_handler) in a later version?) */

#define dhm_handle_command(obj, cmddata, cmd) \
  (__dhm_handler(obj))(&((obj)->dhm_data), (cmddata), (cmd))

#if CONFIG_DEBUG
#define dhm_set_debugstr(obj, dbgstr) \
  do \
  { (((obj)->dhm_data)->debugstr) = dbgstr; \
    debugmsg("dhm_init(): "); debugmsg(dbgstr); debugmsg(strNewline); \
  } while (0)
#else
#define dhm_set_debugstr(obj, dbgstr) do { } while (0)
#endif

#define dhm_init(obj, _control_handler, dbgstr) \
  do \
  { dhm_handle_command((obj), (obj), dhmcInit); \
    dhm_set_debugstr((obj), (dbgstr)); \
    (obj)->dhm_data->control_handler = (_control_handler); \
  } while (0)

#define dhm_get(obj) dhm_handle_command((obj), NULL, dhmcGet)

#define dhm_put(obj) dhm_handle_command((obj), NULL, dhmcPut) /*lost interest*/

#define dhm_attach(dest, obj) do { dhm_get(obj); (dest) = (obj); } while (0)

#define dhm_detach(obj) do { dhm_put(obj); (obj) = NULL; } while (0)

#define dhm_notification_setup(obj, callback, cbdata, flags, mode) \
  do \
  { const tDhmNotificationSetup _dns = { callback, cbdata, flags, mode }; \
    dhm_handle_command((obj), &_dns, dhmcNotificationSetup); \
  } while (0)

#define dhm_notification_off(obj, callback, cbdata) /* lost interest */ \
  dhm_notification_setup((obj), (callback), (cbdata), dhmnfAll, dhmnAndnot)

#define dhm_notify(obj, flags) \
  do \
  { const tDhmNotificationFlags _df = (flags); \
    dhm_handle_command((obj), &_df, dhmcNotify); \
  } while (0)

#define dhm_control(obj, ctrldata, ctrlcode) \
  do \
  { const tDhmControlData _cd = { (ctrldata), (ctrlcode) }; \
    dhm_handle_command((obj), &_cd, dhmcControl); \
  } while (0)

/* General usage of the data handling mechanism:
   - dhm_init() is called by the "owner" of an object, usually right after the
     object has been allocated
   - dhm_get() and dhm_put() are called by "users" of the object, normally via
     dhm_attach() and dhm_detach()
   - dhm_notification_setup() and dhm_notification_off() are called by "users"
     of the object if 1. the object is completely non-refcounted or 2. get/put
     isn't appropriate for a specific "user"
   - dhm_notify() is called by the "owner" to inform the "users" about
     something, esp. dhmnfRemoval
   - dhm_control() can be called by the dhm itself (esp. for dhmccRefcount0) or
     by "users" of the object to set/query information or to start/stop actions
*/


/* Run-time Configuration */

typedef struct tConfigProxy
{ struct tConfigProxy* next;
  char *hosts_pattern, *proxy_hostname;
#if CONFIG_HTTP & HTTP_PROXYAUTH
  char *auth_user, *auth_pass;
#endif
  tPortnumber hosts_portnumber, proxy_portnumber; /* (in network byte order) */
} tConfigProxy;

typedef struct tConfigProtocolVersion
{ struct tConfigProtocolVersion* next;
  char *hosts_pattern, *protstr;
  tPortnumber hosts_portnumber;
} tConfigProtocolVersion;

#if OPTION_COOKIES
enum { ccfNone = 0, ccfAllowed = 0x01 };
typedef unsigned char tConfigCookieFlags;

typedef struct tConfigCookie
{ struct tConfigCookie* next;
  const char* hosts_pattern;
  tConfigCookieFlags flags;
} tConfigCookie;
#endif /* #if OPTION_COOKIES */

#if OPTION_LOCAL_CGI
enum { clcfNone = 0, clcfAllowed = 0x01 };
typedef unsigned char tConfigLocalCgiFlags;

typedef struct tConfigLocalCgi
{ struct tConfigLocalCgi* next;
  const char* path_pattern;
  tConfigLocalCgiFlags flags;
} tConfigLocalCgi;
#endif /* #if OPTION_LOCAL_CGI */

typedef struct tConfigLogin
{ struct tConfigLogin* next;
  char *user, *password, *hosts_pattern;
  tPortnumber hosts_portnumber; /* (in network byte order) */
} tConfigLogin;

#if CONFIG_JUMPS
typedef struct tConfigJump
{ struct tConfigJump* next;
  const char* name; /* "name" is a shortcut for "shortcut" :-) */
  char *uri, **arg;
  unsigned short argcount, maxargcount;
} tConfigJump;
#endif

#if CONFIG_LOCALDIR > 1
typedef struct tConfigLocaldirsort
{ struct tConfigLocaldirsort* next;
  const char* path_pattern;
  const_after_init char* sorting;
} tConfigLocaldirsort;
typedef struct tConfigLocaldirformat
{ struct tConfigLocaldirformat* next;
  const char *path_pattern, *format;
} tConfigLocaldirformat;
#endif /* #if CONFIG_LOCALDIR > 1 */

#if CONFIG_FTP && OPTION_TLS
my_enum1 enum
{ ftmAutodetect = 0, ftmAuthTls = 1, ftmAuthTlsDataclear = 2, ftmAuthSsl = 3,
  ftmTls = 4
} my_enum2(unsigned char) tFtpTlsMethod;
typedef struct tConfigFtpTlsMethod
{ struct tConfigFtpTlsMethod* next;
  char* hosts_pattern;
  tPortnumber hosts_portnumber; /* (in network byte order) */
  tFtpTlsMethod ftm;
} tConfigFtpTlsMethod;
#endif

#if OPTION_EXECEXT
typedef struct tExecextParam
{ struct tExecextParam* next;
  char* param;
} tExecextParam;
#endif

#if CONFIG_RTCONFIG && OPTION_BIRTCFG
extern const char strBirtcfg[]; /* see auto-generated birtcfg.inc */
#endif

my_enum1 enum
{ cfNone = 0, cfColorsOff = 0x01, cfColorsReverse = 0x02,
  cfUserDefinedConfigPath = 0x04, cfTermwintitle = 0x08,
  cfDontConfirmQuit = 0x10, cfDontConfirmClose = 0x20,
  cfDontConfirmOverwrite = 0x40, cfDontConfirmSubmit = 0x80,
  cfDontConfirmReset = 0x100, cfDontConfirmRepost = 0x200,
  cfDontConfirmHtml = 0x400, cfDontConfirmEnable = 0x800
#if CONFIG_DISK_CACHE
  , cfUseDiskCache = 0x1000, cfMayWriteDiskCache = 0x2000
#endif
#if OPTION_EXECEXT & EXECEXT_SHELL
  , cfExecextShellCustom = 0x4000
#endif
#if CONFIG_HTML & HTML_FRAMES
  , cfHtmlFramesSimple = 0x8000
#endif
#if MIGHT_USE_SCROLL_BARS
  , cfUseScrollBars = 0x10000
#endif
#if CONFIG_RTCONFIG
  , cfRtExternal = 0x20000
#if OPTION_BIRTCFG
  , cfRtBuiltin = 0x40000
#define cfRtAll (cfRtExternal | cfRtBuiltin)
#else
#define cfRtAll (cfRtExternal)
#endif
#endif
} my_enum2(tUint32) tConfigurationFlags;

typedef struct
{ const char* path; /* path to the directory which contains config file etc. */
#if CONFIG_DISK_CACHE
  const char* cachepath; /* path to the disk cache directory */
#endif
  const char* user_agent; /* for HTTP header "User-Agent" */
  const char* languages; /* for HTTP header "Accept-Language" */
  const char *home_uri, *search_engine, *bookmarks; /* special URIs */
#if CONFIG_EXTRA & (EXTRA_DOWNLOAD | EXTRA_DUMP)
  const char* pm_uri; /* for pmDownload/pmDump */
#endif
  const_after_init tConfigProtocolVersion* http_version;
  const_after_init tConfigProxy* http_proxies;
#if OPTION_TLS
  const_after_init tConfigProxy* https_proxies;
#endif
#if CONFIG_FTP
  const_after_init tConfigLogin* ftp_login;
#if OPTION_TLS
  const_after_init tConfigFtpTlsMethod* ftp_tls_method;
#endif
#endif
#if CONFIG_JUMPS
  const_after_init tConfigJump* jumps;
#endif
#if CONFIG_LOCALDIR > 1
  const_after_init tConfigLocaldirsort* lds;
  const_after_init tConfigLocaldirformat* ldf;
#endif
#if OPTION_COOKIES
  const_after_init tConfigCookie* http_cookies;
#endif
#if OPTION_TLS && OPTION_COOKIES
  const_after_init tConfigCookie* https_cookies;
#endif
#if OPTION_LOCAL_CGI
  const_after_init tConfigLocalCgi* local_cgi;
#endif
#if OPTION_EXECEXT & EXECEXT_SHELL
  const_after_init tExecextParam* execext_shell;
#endif
#if CONFIG_SESSIONS
  const char *session_default, *session_resume;
#endif
#if OPTION_NEWS
  const char* news_server;
#endif
  size_t ramcachesize;
#if CONFIG_DISK_CACHE
  size_t diskcachesize;
#endif
#if TGC_IS_GRAPHICS
  tCoordinate width, height; /* default window-contents width/height */
#endif
  tConfigurationFlags flags;
#if TGC_IS_CURSES
  char char_yes, char_no; /* (mostly for i18n) */
#endif
#if CONFIG_CONSOLE
  char console_backspace;
#endif
  char char_file, char_dir, char_link; /* (mostly for i18n) */
  unsigned char redirections; /* maximum number of automatic redirections */
} tConfiguration;

extern tConfiguration config;


/* Functions */

extern int streqcase3(const char*, const char*);
#define streqcase(str1, str2) (streqcase3(str1, str2) == 0)
extern int strneqcase3(const char*, const char*, size_t);
#define strneqcase(str1, str2, maxlen) (strneqcase3(str1, str2, maxlen) == 0)
extern void my_atoi(const char*, /*@reldef@*/ int*, /*@reldef@*/ const char**,
  int);
extern char* my_strnchr(const char*, int, int);
extern char* my_strncasestr(const char*, const char*, const size_t);
extern tBoolean is_suffix(const char* /*string*/, const char* /*suffix*/);
extern tBoolean my_pattern_matcher(const char*, const char*);
extern void does_not_return do_quit(void);
extern void does_not_return do_quit_sig(void);
extern void does_not_return fatal_error(int, const char*);
extern __sallocator /*@notnull@*/ /*@out@*/ void* __callocator
  __memory_allocate_ll(size_t);
extern __sallocator /*@notnull@*/ void* __callocator
  memory_allocate_ll(size_t);
extern __sallocator /*@notnull@*/ void* memory_reallocate_ll(/*@only@*/ void*,
  size_t);
extern void memory_deallocate_ll(/*@only@*/ /*@notnull@*/ const void*);
extern __sallocator char* __callocator my_strdup(const char*);
extern __sallocator char* __callocator my_strndup(const char*, size_t);
extern __sallocator char* __callocator my_strdup_tolower(const char*);
extern __sallocator char* __callocator my_strndup_tolower(const char*, size_t);

my_enum1 enum
{ /* low-level */
  mapUnknown = 0, mapOther = 1, mapPermanent = 2, mapString = 3,
  mapScripting = 4,
  /* networking */
  mapResource = 5, mapResourceRequest = 6, mapContentblock = 7,
  mapConnection = 8, mapUriData = 9, mapSinkingData = 10, mapWritedata = 11,
  mapRpsd = 12,
  /* user interface */
  mapWindow = 13, mapWksd = 14, mapWindowView = 15, mapCantent = 16,
  mapLineInput = 17, mapUserQuery = 18, mapGtk = 19, mapKeymap = 20,
  mapRendering = 21, mapHtmlNode = 22
} my_enum2(unsigned char) tMemoryAllocationPool; /* (mostly for debugging) */

#ifndef memory_allocate
#define __memory_allocate(s, m) __memory_allocate_ll(s)
#define memory_allocate(s, m) memory_allocate_ll(s)
#define memory_reallocate(p, s, m) memory_reallocate_ll(p, s)
#define memory_deallocate(p) memory_deallocate_ll(p)
#endif

extern tBoolean is_time_valid;
extern time_t my_time(void);
extern tBoolean my_memdiff(const void*, const void*, size_t);
extern const char* get_homepath(void);
#if CONFIG_LOCALDIR > 1
extern void check_localdirsort(char*);
#endif

#define MIGHT_NEED_TIMEOUTS ( (CONFIG_TG == TG_BICURSES) || ( (CONFIG_TG == TG_XCURSES) && (MIGHT_USE_SCROLL_BARS) ) || (OPTION_EXECINT & EXECINT_RELIAWFUL) || (USE_LWIP) )

#if MIGHT_NEED_TIMEOUTS
typedef tBoolean (*tTimeoutHandler)(int*);
extern void timeout_register(tTimeoutHandler);
extern void timeout_unregister(tTimeoutHandler);
#endif

#if NEED_FD_REGISTER
my_enum1 enum
{ fdkNone = 0, fdkFile = 0x01, fdkPipe = 0x02, fdkSocket = 0x04,
  fdkOther = 0x08 /* ..., fdkStdin, fdkStdout, fdkStderr, fdkTerminal, ...? */
} my_enum2(unsigned char) tFdKind;
extern void fd_register_init(void);
extern void fd_register(int*, tFdKind);
extern tFdKind fd_register_lookup(int*);
#if CONFIG_TG == TG_XCURSES
extern tBoolean fd_register_rlookup(int*, tFdKind);
#endif
extern int my_pipe(/*@out@*/ int*);
extern int my_isatty(int);
#define my_write_str_unregistried(fd, str) write(fd, str, strlen(str))/*FIXME*/
#define my_close_unregistried close /* FIXME! */
#define my_fstat_unregistried fstat /* FIXME! */
#else
#define fd_register(fdptr, kind) do { } while (0)
#define my_write_str_unregistried my_write_str
#define my_close_unregistried my_close
#define my_fstat_unregistried my_fstat
#define my_pipe(f) pipe(f)
#define my_isatty(x) isatty(x)
#endif

my_enum1 enum
{ fdofNone = 0, fdofRead = 0x01, fdofWrite = 0x02
} my_enum2(unsigned char) tFdObservationFlags;

typedef void (*tFdObservationHandler)(void*, tFdObservationFlags);

#if OPTION_POLL
#define fd_is_observable(fd) (truE) /* never a problem */
#else
extern tBoolean fd_is_observable(int);
#endif

extern void fd_observe_init(void);
extern void fd_observe(int, tFdObservationHandler, void*, tFdObservationFlags);
extern void fd_observe_change_handler(int, tFdObservationHandler);
extern void fd_unobserve(int);
extern void fd_multiplex(void);
extern void does_not_return fatal_tmofd(int);

extern int my_create(const char*, int, mode_t);
extern int my_open(const char*, int);
extern void my_close(int);
extern ssize_t my_read(int, /*@out@*/ void*, size_t);
extern ssize_t __my_write(int, const void*, size_t);
extern ssize_t my_write(int, const void*, size_t);
#define my_write_str(fd, str) \
  do { const char* _x = (str); (void) my_write(fd, _x, strlen(_x)); } while (0)
extern void my_write_crucial(int, const void*, size_t);
extern unsigned char my_mmap_file_readonly(const char*, /*@out@*/ void**,
  /*@out@*/ size_t*);
extern int my_stat(const char*, /*@out@*/ struct stat*);
extern int my_fstat(int, /*@out@*/ struct stat*);

/* confus^Wclarification for sockets */
#define my_read_sock my_read
#define my_write_sock my_write
#define my_close_sock my_close

/* clarification for pipes */
#define my_read_pipe my_read
#define my_write_pipe my_write
#define my_close_pipe my_close

/* clarification for sockets/pipes */
#define my_read_sopi my_read
#define my_write_sopi my_write
#define my_close_sopi my_close

#define my_strcpy_conv(__dest, __src, __func) \
  do \
  { const char* _src = (__src); char ch, *_dest = (__dest); /* eval once */ \
    do { ch = *_src++; *_dest++ = (__func); } while (ch != '\0'); \
  } while (0)
#define my_strcpy_tolower(_dest, _src) \
  my_strcpy_conv(_dest, _src, my_tolower(ch))
#define my_strcpy_toupper(_dest, _src) \
  my_strcpy_conv(_dest, _src, my_toupper(ch))

extern void my_spf_attr my_spf(char*, size_t, /*@out@*/ char**,
  const char*, ...);
#define my_spf_cleanup(staticbuf, spfbuf) \
  do { if (staticbuf != spfbuf) memory_deallocate(spfbuf); } while (0)
#define my_spf_use(spfbuf) (spfbuf) /* (just to clarify the code) */

#if (CONFIG_TG == TG_BICURSES) || (CONFIG_TG == TG_XCURSES)
extern tBoolean env_termsize(/*@out@*/ int*, /*@out@*/ int*); /* see init.c */
#endif

#define MIGHT_WANT_TERMSIZE ( ( (TGC_IS_CURSES) && (CONFIG_TG != TG_XCURSES) ) /* || (CONFIG_CONSOLE) */ )
#if MIGHT_WANT_TERMSIZE
extern tBoolean calc_termsize(/*@out@*/ int*, /*@out@*/ int*); /* see init.c */
#endif

#if MIGHT_FORK_EXEC
#if CAN_HANDLE_SIGNALS
extern void reset_signals(void); /* see init.c */
#else
static __my_inline void reset_signals(void) { }
#endif
#endif

#if CONFIG_KEYMAPS /* see main.c */
extern unsigned char keymap_command_key_register(const char*, const char*);
extern unsigned char keymap_lineinput_key_register(const char*, const char*);
#endif

#endif /* #ifndef __retawq_stuff_h__ */
