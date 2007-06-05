/* retawq/stuff.c - general stuff
   This file is part of retawq (<http://retawq.sourceforge.net/>), a network
   client created by Arne Thomassen; retawq is basically released under certain
   versions of the GNU General Public License and WITHOUT ANY WARRANTY.
   Read the file COPYING for license details, README for program information.
   Copyright (C) 2001-2005 Arne Thomassen <arne@arne-thomassen.de>
*/

#include "stuff.h"
#include "resource.h"

#include <time.h>

declare_local_i18n_buffer

const_after_init int fd_stdin = -1, fd_stdout = -1, fd_stderr = -1,
  fd_keyboard_input = -1;
tConfiguration config;

const_after_init tProgramMode program_mode =
#if CONFIG_TG == TG_CONSOLE
  pmConsole
#else
  pmEnvironed
#endif
  ;

const char* initial_messages = NULL;
#if CONFIG_CONSOLE
const char* initial_console_msgs = NULL;
#endif
unsigned char launch_uri_count = 0;
const char* launch_uri[2]; /* documents to load on launch (if environed) */

#if MIGHT_DO_TERMIOS
tBoolean do_restore_termios = falsE;
struct termios saved_termios;
#endif

tBoolean need_tglib_cleanup = falsE;
const_after_init unsigned char __lfdmbs = 0;

#if MIGHT_USE_COLORS
tBoolean use_colors = falsE;
#if TGC_IS_CURSES
tColorBitmask color_bitmask[cpnMax + 1];
#endif
#endif

#if CONFIG_DEBUG
const_after_init int debugfd = 2;
static char debugstrbuf[STRBUF_SIZE];
#endif


/* Strings */

const char strRetawq[] = "retawq", strCopyright[] = "retawq " RETAWQ_VERSION
  " - Copyright (C) 2001-2005 Arne Thomassen <arne@arne-thomassen.de>\n",
  strProgramVersion[] = "retawq " RETAWQ_VERSION, strProgramLegalese[] =
  "retawq is basically released under certain versions of the\n"
  "GNU General Public License and WITHOUT ANY WARRANTY.\n"
  "The project home page is <http://retawq.sourceforge.net/>.\n";
const char strEmpty[] = "", strNewline[] = "\n", strMinus[] = "-",
  strSlash[] = "/", strAsterisk[] = "*", strLt[] = "<", strGt[] = ">",
  strHm[] = "#", strQm[] = "?", strDoubleQuote[] = "\"",
  strSpacedDash[] = " - ", strDoubleDot[] = "..", str1dot0[] = "1.0",
  str1dot1[] = "1.1", strHexnum[] = "0123456789abcdef";
static const char strPercld[] = "%ld",
  strTooManyObjects[] = N_("too many objects");
const char strPercsPercs[] = "%s%s", strPercd[] = "%d",
  strBracedNumstr[] = " (%d %s)"; /* for sprintf()-style formatting */
const char strA[] = "a", strShtml[] = "shtml", strReset[] = "reset",
  strSelect[] = "select", strSubmit[] = "submit", strOn[] = "on",
  strClose[] = "close", strQuit[] = "quit", strGet[] = "GET",
  strPost[] = "POST", strLocalhost[] = "localhost", strOptHelp[] = "--help",
  strOptVersion[] = "--version";
const char strText[] = N_("text"), strGraphics[] = N_("graphics"),
  strUnknown[] = N_("(unknown)"), strCheckbox[] = N_("checkbox"),
  strBracedDisabled[] = N_("(disabled)"), strButton[] = N_("button"),
  strImage[] = N_("image"), strYes[] = N_("yes"), strNo[] = N_("no"),
  strFileUc[] = N_("File"), strDirectoryUc[] = N_("Directory"),
  strLink[] = N_("link"), strLoginFailed[] = N_("Login failed"),
  strFwdact[] = N_("feature \"%s\" was disabled at compile-time%s"),
  strByte[] = N_("byte"), strBytes[] = N_("bytes"), strOff[] = N_("off"),
  strBadValue[] = N_("bad value"), strErrorTrail[] = N_("; error #%d, %s\n");
#if !CONFIG_SESSIONS
const char strFsessions[] = N_("Fsessions");
#endif
#if !CONFIG_JUMPS
const char strFjumps[] = N_("Fjumps");
#endif
#if OPTION_EXECEXT != EXECEXT_ALL
const char strFexecext[] =
  N_("F(this kind of) execution of external programs"); /* CHECKME! */
#endif
#if CONFIG_CONSOLE
const char strConsoleDiscard[] = "cdis";
#endif
#if (CONFIG_TG == TG_X) || (CONFIG_TG == TG_XCURSES)
const char strCantOpenXws[] = N_("can't open X Window System display");
#endif
#if OPTION_LOCAL_CGI || OPTION_TRAP || (OPTION_EXECEXT & EXECEXT_SHELL)
const char strSoftwareId[] = "retawq/" RETAWQ_VERSION;
#endif


/* scheme strings: */
const char strHttp[] = "http", strFtp[] = "ftp", strLocal[] = "local",
  strFile[] = "file", strAbout[] = "about", strFinger[] = "finger",
  strLocalCgi[] = "local-cgi", strNntp[] = "nntp", strNews[] = "news",
  strCvs[] = "cvs", strGopher[] = "gopher", strInfo[] = "info",
  strMailto[] = "mailto", strJavascript[] = "javascript", strHttps[] = "https",
  strFtps[] = "ftps", strPop[] = "pop", strPops[] = "pops",
  strExecextShell[] = "execext-shell";


/* Functions */

int streqcase3(const char* _str1, const char* _str2)
/* This function is a useful mixture of strcmp() and strcasecmp(): it compares
   the lowercase variant of <str1> with <str2> (not "with the lowercase
   variant of <str2>"!). The "3" in the function name reminds of the
   "tristate" nature of the return value (< 0, == 0, > 0), as opposed to
   the "two-state" (== 0, != 0) return value of streqcase(). */
{ const unsigned char *str1 = (const unsigned char*) _str1,
    *str2 = (const unsigned char*) _str2;
  unsigned char ch1, ch2;
  do
  { ch1 = *str1++; ch2 = *str2++;
    if (ch1 == '\0') break;
    ch1 = (unsigned char) my_tolower(ch1);
  } while (ch1 == ch2);
  return((int) (ch1 - ch2));
}

int strneqcase3(const char* _str1, const char* _str2, size_t maxlen)
/* like streqcase3(), but compares at most <maxlen> characters */
{ int retval;
  if (maxlen <= 0) retval = 0;
  else
  { const unsigned char *str1 = (const unsigned char*) _str1,
      *str2 = (const unsigned char*) _str2;
    unsigned char ch1, ch2;
    do
    { ch1 = *str1++; ch2 = *str2++;
      if (ch1 == '\0') break;
      ch1 = (unsigned char) my_tolower(ch1);
    } while ( (ch1 == ch2) && (--maxlen > 0) );
    retval = ((int) (ch1 - ch2));
  }
  return(retval);
}

void my_atoi(const char* src, int* value, const char** next, int maxvalue)
/* similar to atoi(), but allows only non-negative values, allows to define a
   maximum possible value, and additionally returns the position of the first
   non-numeric character */
{ int v = 0;
  while (1)
  { char ch = *src;
    if (my_isdigit(ch))
    { v = 10 * v + ((int)(ch - '0'));
      src++;
      if (v > maxvalue)
      { v = maxvalue;
        while (my_isdigit(*src)) { src++; } /* skip number */
        goto done;
      }
    }
    else
    { done:
      *value = v;
      if (next != NULL) *next = src;
      return;
    }
  }
}

char* my_strnchr(const char* str, int _ch, int count)
/* like strchr(), but checks at most <count> characters */
{ char ch = ((char) _ch);
  while (count-- > 0)
  { char c = *str++;
    if (c == '\0') break;
    else if (c == ch) return(unconstify(str) - 1);
  }
  return(NULL);
}

char* my_strncasestr(const char* haystack, const char* needle,
  const size_t hlen)
/* some mixture of strncasecmp() and strstr() */
{ const size_t nlen = strlen(needle);
  size_t h = 0, n = 0;
  do
  { if (my_tolower(haystack[h]) == my_tolower(needle[n])) n++;
    else { h -= n; n = 0; }
    h++;
  } while ( (n < nlen) && (h < hlen) );
  if (n >= nlen) return(unconstify(haystack) + h - nlen - 1); /* found */
  else return(NULL);
}

my_inline tBoolean is_suffix(const char* str, const char* suff)
/* returns whether <str> ends with (or equals!) <suff> */
{ size_t str_len = strlen(str), suff_len = strlen(suff);
  if ( (str_len >= suff_len) && (!strcmp(str + (str_len - suff_len), suff)) )
    return(truE);
  else return(falsE);
}

tBoolean my_pattern_matcher(const char* pattern, const char* str)
/* a _very_ simple pattern matching algorithm - but it's enough for us */
{ if (*pattern == '*') return(is_suffix(str, pattern + 1));
  else
  { if (!strcmp(pattern, str)) return(truE);
    else return(falsE);
  }
}

static void does_not_return do_quit_msg(int exitcode, const char* msg)
{ if (need_tglib_cleanup)
  {
#if CONFIG_TG == TG_X
    (void) XCloseDisplay(xws_display);
#elif CONFIG_TG == TG_GTK
    gtk_main_quit();
#elif TGC_IS_CURSES
    (void) endwin();
#endif
  }
#if MIGHT_DO_TERMIOS
  if (do_restore_termios)
  { int err;
    unsigned char loopcount = 0;
    do
    { err = tcsetattr(0, TCSAFLUSH, &saved_termios);
    } while ( (err == -1) && (errno == EINTR) && (++loopcount < 100) );
  }
#endif
  if (msg != NULL)
  { const int fd = ( (exitcode != 0) ? fd_stderr : fd_stdout );
    /* if (lfdmbs(....)) */ my_write_str(fd, msg);
    debugmsg(msg);
  }
  resource_quit();
  exit(exitcode);
}

void do_quit(void)
/* quit without errors */
{ do_quit_msg(0, NULL);
}

void do_quit_sig(void)
/* terminated by a signal */
{ do_quit_msg(2, _("retawq: terminated by a signal\n"));
}

void fatal_error(int err, const char* str)
/* quits the program due to a fatal error; <err> must be errno or an E...
   constant or 0. */
{ static const char prefix[] = N_("retawq: fatal error: ");
    /* ugly colon-ialism... */
  static tBoolean is_recursive = falsE;
  const tBoolean is_rec = is_recursive;
  char buf[1000];
  const char* ptr;
  is_recursive = truE;
  /* if (lfdmbs(2)) */
  { const char* p = ( is_rec ? prefix : _(prefix) );
    my_write_str(fd_stderr, p); my_write_str(fd_stderr, str);
    debugmsg(p); debugmsg(str);
  }
  if (err <= 0) ptr = strNewline;
  else
  { const char* errstr = (is_rec ? __my_strerror(err) : my_strerror(err));
    const tBoolean too_long = cond2boolean(strlen(errstr) > 200);
    sprint_safe(buf, (is_rec ? strErrorTrail : _(strErrorTrail)), err,
      (too_long ? (is_rec ? strUnknown : _(strUnknown)) : errstr));
    ptr = buf;
  }
  do_quit_msg(1, ptr);
}

static void does_not_return out_of_memory(void)
{ fatal_error(ENOMEM, _("out of memory"));
}


#if !CONFIG_DEBUG
my_inline /* gcc/gdb and line numbers... */
#endif
void* __memory_allocate_ll(size_t size)
{ void* ptr = malloc(size);
  if (ptr == NULL) out_of_memory();
  return(ptr);
}

void* memory_allocate_ll(size_t size)
{ void* ptr = __memory_allocate_ll(size);
  my_memclr(ptr, size);
  return(ptr);
}

void* memory_reallocate_ll(void* ptr, size_t size)
{ void* ptr2 = realloc(ptr, size);
  if (ptr2 == NULL) out_of_memory();
  return(ptr2);
}

void memory_deallocate_ll(const void* ptr)
{ /* guess what... */
  free(__unconstify(void*, ptr));
}

#if NEED_SPECIAL_GETTEXT
#define is_special_char(ch) ( (ch == 'ä') || (ch == 'ö') || (ch == 'ü') || \
  (ch == 'Ä') || (ch == 'Ö') || (ch == 'Ü') || (ch == 'ß') )
char* my_do_gettext(char** buffer, const char* str)
{ if (str == NULL) /* deallocate old strings */
  { char* ptr = *buffer;
    while (ptr != NULL)
    { char* next = *((char**) ptr);
      memory_deallocate(ptr);
      ptr = next;
    }
    *buffer = NULL;
    return(NULL);
  }
  else /* translate and make seven-bit-clean */
  { static const char charmap[] = "?????????\t\n??\r?????????????????? !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~??????????????????????????????????!????|????\"?-?-????'??????\"????AAAAAA?CEEEEIIIIDNOOOOO*OUUUUYPsaaaaaa?ceeeeiiiidnooooo/ouuuuypy";
    const char *s = gettext(str), *src;
    char *buf, *dest, *retval, ch;
    size_t len;
    if (s == NULL) return(NULL); /* "should not happen" */
    src = s; len = 0;
    while ( (ch = *src++) != '\0' ) { len++; if (is_special_char(ch)) len++; }
    buf = __memory_allocate(sizeof(char*) + len + 1, mapString);
    dest = retval = buf + sizeof(char*);
    src = s;
    while ( (ch = *src++) != '\0' )
    { *dest++ = charmap[(unsigned short) ((unsigned char) ch)];
      if (is_special_char(ch))
      { if (ch == 'ß') *dest++ = 's';
        else *dest++ = 'e';
      }
    }
    *dest = '\0';
    *((char**) buf) = *buffer;
    *buffer = buf;
    return(retval);
  }
}
#undef is_special_char
#endif /* #if NEED_SPECIAL_GETTEXT */

char* my_strdup(const char* str)
/* duplicates a string; the standard library function strdup() isn't portable,
   and callers always have to check against a NULL return value - two reasons
   for using something different... */
{ size_t size = strlen(str) + 1;
  char* retval = __memory_allocate(size, mapString);
  my_memcpy(retval, str, size);
  return(retval);
}

char* my_strndup(const char* str, size_t len)
{ char* retval = __memory_allocate(len + 1, mapString);
  strncpy(retval, str, len); retval[len] = '\0';
  return(retval);
}

char* my_strdup_tolower(const char* str)
/* duplicates a string, changing uppercase to lowercase */
{ char* retval = __memory_allocate(strlen(str) + 1, mapString);
  my_strcpy_tolower(retval, str);
  return(retval);
}

char* my_strndup_tolower(const char* str, size_t len)
{ char *retval = __memory_allocate(len + 1, mapString), *ptr = retval;
  while (len-- > 0)
  { const char ch = *str++;
    *ptr++ = my_tolower(ch);
  }
  *ptr = '\0';
  return(retval);
}

tBoolean is_time_valid = falsE;
time_t my_time(void)
{ static time_t current_time = 0;
  if (!is_time_valid)
  { time_t t = time(NULL);
    if (t != -1) { current_time = t; is_time_valid = truE; }
  }
  return(current_time);
}

tBoolean my_memdiff(const void* _a, const void* _b, size_t count)
/* returns whether the two memory regions have different contents; somewhat
   similar to memcmp(), but I didn't want to go through all the portability
   mess related to memcmp() for this simple thing... */
{ tBoolean retval = falsE;
  const char *a = (const char*) _a, *b = (const char*) _b;
  while (count-- > 0) { if (*a++ != *b++) { retval = truE; break; } }
  return(retval);
}

const char* get_homepath(void)
/* tries to find out the user's home directory */
{ static const char* homepath = NULL;
  if (homepath == NULL) /* not yet calculated */
  { const char* const env = getenv("HOME"); /* user's home directory */
    if ( (env == NULL) || (*env == '\0') ) homepath = strEmpty;
    else
    { tBoolean has_trailslash = cond2boolean(env[strlen(env) - 1] == chDirsep);
      char* spfbuf;
      my_spf(NULL, 0, &spfbuf, strPercsPercs, env,
        (has_trailslash ? strEmpty : strDirsep));
      homepath = my_spf_use(spfbuf);
    }
  }
  return(homepath);
}

#if CONFIG_LOCALDIR > 1
void check_localdirsort(char* str)
/* checks and possibly fixes the "sorting" string for local directory sorting,
   in both run-time configuration entries and URI parts */
{ const char* src;
  char ch, *dest;
  unsigned char bits = 0;
  if (*str == '_')
  { /* disables all sorting, no further check necessary */
    *(str + 1) = '\0'; /* ignore any further characters */
    return;
  }
  src = dest = str;
  while ( (ch = *src++) != '\0' )
  { static const char allowed[] = "gimnstu";
      /* (group ID, case-insensitive name, modification time, name, size,
          type (file/directory/...), user ID) */
    const char lch = my_tolower(ch), *pos = my_strchr(allowed, lch);
    if (pos != NULL) /* it's an _allowed_ sorting option */
    { char val = (char) (pos - allowed);
      if (!my_bit_test(&bits, val)) /* only if option not yet used */
      { my_bit_set(&bits, val);
        *dest++ = ch;
      }
    }
  }
  *dest = '\0';
}
#endif


/* Timeouts for I/O multiplexing */

#if MIGHT_NEED_TIMEOUTS

typedef struct tTimeout
{ struct tTimeout* next;
  tTimeoutHandler handler;
} tTimeout;

static tTimeout* timeout_handlers = NULL;

static tTimeout* timeout_lookup(tTimeoutHandler handler)
{ tTimeout* retval = timeout_handlers;
  while (retval != NULL)
  { if (retval->handler == handler) break;
    retval = retval->next;
  }
  return(retval);
}

void timeout_register(tTimeoutHandler handler)
{ tTimeout* t = timeout_lookup(handler);
  if (t == NULL)
  { /* not yet registered; "should" be true, lookup is extra care */
    t = (tTimeout*) __memory_allocate(sizeof(tTimeout), mapOther);
    t->handler = handler; t->next = timeout_handlers; timeout_handlers = t;
  }
}

void timeout_unregister(tTimeoutHandler handler)
{ tTimeout* t = timeout_lookup(handler);
  if (t != NULL) /* "should" be true */
  { list_extract(&timeout_handlers, t, tTimeout); memory_deallocate(t); }
}

#endif /* #if MIGHT_NEED_TIMEOUTS */


/* File descriptor handling I: register */

#if NEED_FD_REGISTER

typedef struct tFdData
{ struct tFdData* next;
  int unique_fd, lib_fd;
  tFdKind kind;
} tFdData;

#define FD_REGISTER_LEN (1 << 4)
#define fd_register_hash(fd) ((fd) & (FD_REGISTER_LEN - 1))
typedef unsigned char tFdRegisterIndex;
static tFdData* fd_data[FD_REGISTER_LEN];

void __init fd_register_init(void)
{ my_memclr_arr(fd_data);
}

static one_caller int get_unique_fd(void)
/* The returned number need not have any kind of "meaning", it just has to be
   _unique_ within the register, so that the associated data can be looked up
   reliably. */
{ static int num = 0;
  int retval = num++;
  if (retval < 0) fatal_error(0, _(strTooManyObjects)); /* overflow; "rare" */
  return(retval);
}

void fd_register(int* _lib_fd, tFdKind kind)
{ tFdData **head, *data;
  int lib_fd = *_lib_fd, unique_fd = *_lib_fd = get_unique_fd();
  head = &(fd_data[fd_register_hash(unique_fd)]);
  data = (tFdData*) __memory_allocate(sizeof(tFdData), mapOther);
  data->unique_fd = unique_fd; data->lib_fd = lib_fd;
  data->kind = kind; data->next = *head; *head = data;
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "fd_register(): %d, %d, %d\n", unique_fd, lib_fd,
    kind);
  debugmsg(debugstrbuf);
#endif
}

static tFdData* __fd_register_lookup(int unique_fd)
{ tFdData* data;
  if (unique_fd < 0) return(NULL); /* may happen, e.g. if no stdin fd exists */
  data = fd_data[fd_register_hash(unique_fd)];
  while (data != NULL)
  { if (data->unique_fd == unique_fd) break;
    data = data->next;
  }
  if (data == NULL) /* "should not happen" */
    fatal_error(0, _("can't lookup file descriptor"));
  return(data);
}

tFdKind fd_register_lookup(int* fd)
{ tFdKind retval;
  const tFdData* data = __fd_register_lookup(*fd);
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "fd_register_lookup(): %d, ", *fd);
  debugmsg(debugstrbuf);
#endif
  if (data != NULL) { *fd = data->lib_fd; retval = data->kind; }
  else retval = fdkNone;
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "%d, %d\n", *fd, retval); debugmsg(debugstrbuf);
#endif
  return(retval);
}

#if CONFIG_TG == TG_XCURSES
tBoolean fd_register_rlookup(int* fd, tFdKind kind)
/* reverse lookup; not (yet?) used, just for completeness... */
{ tBoolean retval = falsE;
  tFdRegisterIndex idx;
  for (idx = 0; idx < FD_REGISTER_LEN; idx++)
  { tFdData* data = fd_data[idx];
    while (data != NULL)
    { if ( (data->lib_fd == *fd) && (data->kind & kind) )
      { *fd = data->unique_fd; retval = truE; goto out; }
      data = data->next;
    }
  }
  out:
  return(retval);
}
#endif

static tFdKind fd_unregister(int* fd)
{ tFdKind retval;
  tFdData **head, *data;
  if (*fd < 0) retval = fdkNone;
  else
  { head = &(fd_data[fd_register_hash(*fd)]); data = __fd_register_lookup(*fd);
    retval = data->kind; *fd = data->lib_fd; list_extract(head, data, tFdData);
    memory_deallocate(data);
  }
  return(retval);
}

#endif /* #if NEED_FD_REGISTER */


/* File descriptor handling II: observing, multiplexing */

static size_t fd_observatory_trashcount = 0;

#if OPTION_POLL == 0

#if (HAVE_SYS_TIME_H) && (MIGHT_NEED_TIMEOUTS)
#include <sys/time.h> /* might be needed for struct timeval */
#endif

#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

static fd_set fds_read, fds_write, fds_r, fds_w, fds_e;
static int highest_select_fd = 0;

tBoolean fd_is_observable(int fd)
{ tBoolean retval;
#if NEED_FD_REGISTER
  tFdKind kind = fd_register_lookup(&fd);
#endif
#if USE_LWIP
  if (kind & fdkSocket) retval = truE;
  else
#endif
  { retval = cond2boolean(fd < FD_SETSIZE); }
  return(retval);
}

#elif OPTION_POLL == 1

#if HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif

static size_t fd_observatory_count = 0;

#else

#error "Bad value of compile-time configuration variable OPTION_POLL"

#endif

typedef struct tFdObservationData
{ struct tFdObservationData* next;
  tFdObservationHandler handler;
  void* handler_data; /* (e.g. tConnection*) */
  int fd; /* (unique_fd, that is) */
  tFdObservationFlags flags;
  tBoolean is_trashed;
} tFdObservationData;

#define FD_OBSERVATORY_LEN (1 << 3)
#define fd_observe_hash(fd) ((fd) & (FD_OBSERVATORY_LEN - 1))
static tFdObservationData* fd_observatory[FD_OBSERVATORY_LEN];

one_caller void __init fd_observe_init(void)
{ my_memclr_arr(fd_observatory);
#if OPTION_POLL == 0
  FD_ZERO(&fds_read); FD_ZERO(&fds_write);
  fds_r = fds_e = fds_read; fds_w = fds_write;
#endif
}

static void fd_observatory_cleanup(void)
{ if (fd_observatory_trashcount > 0) /* must do some cleanup; IMPROVEME! */
  { unsigned char cnt;
    for (cnt = 0; cnt < FD_OBSERVATORY_LEN; cnt++)
    { tFdObservationData* data;
      recheck_list: data = fd_observatory[cnt];
      while (data != NULL)
      { if (data->is_trashed)
        { list_extract(&(fd_observatory[cnt]), data, tFdObservationData);
          memory_deallocate(data); fd_observatory_trashcount--;
          if (fd_observatory_trashcount > 0) goto recheck_list;
          else goto trash_disposed;
        }
        data = data->next;
      }
    }
    trash_disposed: {}
  }
}

static tFdObservationData* fd_observe_lookup(int fd)
{ tFdObservationData* data = fd_observatory[fd_observe_hash(fd)];
  while (data != NULL)
  { if ( (data->fd == fd) && (!(data->is_trashed)) ) break;
    data = data->next;
  }
  return(data);
}

void fd_observe(int fd, tFdObservationHandler handler, void* handler_data,
  tFdObservationFlags flags)
{ tFdObservationData* data = fd_observe_lookup(fd);
#if NEED_FD_REGISTER
  tFdKind kind;
#endif
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf,
    "fd_observe(): fd=%d, flags=%d, handler=%p, handler_data=%p\n", fd, flags,
    handler, handler_data);
  debugmsg(debugstrbuf);
#endif
  if (data == NULL)
  { const int h = fd_observe_hash(fd);
    data = (tFdObservationData*) memory_allocate(sizeof(tFdObservationData),
      mapOther);
    data->fd = fd; data->next = fd_observatory[h]; fd_observatory[h] = data;
#if OPTION_POLL == 1
    fd_observatory_count++;
#endif
  }
  data->handler = handler; data->handler_data = handler_data;
  data->flags = flags;

#if NEED_FD_REGISTER
  kind = fd_register_lookup(&fd);
#endif
#if USE_LWIP
  if (kind & fdkSocket)
#endif
  {
#if OPTION_POLL == 0
    if (highest_select_fd < fd) highest_select_fd = fd;
    if (flags & fdofRead) FD_SET(fd, &fds_read);
    else { FD_CLR(fd, &fds_read); FD_CLR(fd, &fds_r); FD_CLR(fd, &fds_e); }
    if (flags & fdofWrite) FD_SET(fd, &fds_write);
    else { FD_CLR(fd, &fds_write); FD_CLR(fd, &fds_w); }
#endif
  }
}

void fd_unobserve(int fd)
{ tFdObservationData* data = fd_observe_lookup(fd);
#if NEED_FD_REGISTER
  tFdKind kind;
#endif
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "fd_unobserve(%d): %p\n", fd, data);
  debugmsg(debugstrbuf);
#endif
  if (data != NULL)
  { /* (We only clear here and deallocate at the end of fd_multiplex().) */
    data->is_trashed = truE; fd_observatory_trashcount++;
#if OPTION_POLL == 1
    if (fd_observatory_count > 0) fd_observatory_count--; /* "should" be true*/
#endif
#if NEED_FD_REGISTER
    kind = fd_register_lookup(&fd);
#endif
#if USE_LWIP
    if (kind & fdkSocket)
#endif
    {
#if OPTION_POLL == 0
      FD_CLR(fd, &fds_read); FD_CLR(fd, &fds_r); FD_CLR(fd, &fds_e);
      FD_CLR(fd, &fds_write); FD_CLR(fd, &fds_w); debugmsg("FD_CLR()\n");
#endif
    }
  }
}

void fd_multiplex(void)
{ unsigned short loopcount = 0;
  int count;
  unsigned char cnt;
#if MIGHT_NEED_TIMEOUTS
  tTimeout* toh;
  tBoolean do_timeout;
  int msec;
#endif

  fd_observatory_cleanup(); loop:

  /* step #1: calculate the timeout, if any */

#if MIGHT_NEED_TIMEOUTS
  toh = timeout_handlers; msec = 10000000; do_timeout = falsE;
  while (toh != NULL)
  { int m;
    if ((toh->handler)(&m)) { do_timeout = truE; if (msec > m) msec = m; }
    toh = toh->next;
  }
#endif

  /* step #2: call into the operating system */

#if OPTION_POLL == 0

  {
#if MIGHT_NEED_TIMEOUTS
    struct timeval tov, *tovp;
    if (!do_timeout) tovp = NULL;
    else { my_memclr_var(tov); tov.tv_usec = msec * 1000; tovp = &tov; }
#else
#define tovp (NULL)
#endif
#if CONFIG_DEBUG
    { int c = 0;
      debugmsg("multiplex A");
      while (c <= highest_select_fd)
      { if (FD_ISSET(c, &fds_read))
        { sprint_safe(debugstrbuf, " r%d", c); debugmsg(debugstrbuf); }
        if (FD_ISSET(c, &fds_write))
        { sprint_safe(debugstrbuf, " w%d", c); debugmsg(debugstrbuf); }
        c++;
      }
      debugmsg(strNewline);
    }
#endif
    fds_r = fds_e = fds_read; fds_w = fds_write;
#if USE_LWIP
    count = lwip_select(highest_select_fd + 1, &fds_r, &fds_w, &fds_e, tovp);
#else
    count = select(highest_select_fd + 1, &fds_r, &fds_w, &fds_e, tovp);
#endif
#if CONFIG_DEBUG
    { const int e = errno;
      sprint_safe(debugstrbuf, "multiplex B: %d, %d\n", count, errno);
      debugmsg(debugstrbuf); errno = e;
    }
#endif
#undef tovp
  }

#endif

  /* step #3: handle the returned file descriptors */

  is_time_valid = falsE; /* we probably slept */
  if (count <= 0)
  { if ( (count == -1) && (errno == EINTR) && (++loopcount < 10000) )
      goto loop;
#if MIGHT_NEED_TIMEOUTS
    else if ( (count == 0) && (do_timeout) ) return; /* timed out */
#endif
    else fatal_error(((count == -1) ? errno : 0),_("I/O multiplexing failed"));
  }

  resource_preplex();
  for (cnt = 0; cnt < FD_OBSERVATORY_LEN; cnt++)
  { const tFdObservationData* data = fd_observatory[cnt];
    while (data != NULL)
    { int fd;
      tFdObservationFlags flags;
#if NEED_FD_REGISTER
      tFdKind kind;
#endif
      if (data->is_trashed) goto do_next;
      fd = data->fd; flags = data->flags;
#if NEED_FD_REGISTER
      kind = fd_register_lookup(&fd);
#endif
#if USE_LWIP
      /* if (kind & fdkSocket) goto do_next; -- CHECKME! */
#endif
#if OPTION_POLL == 0
      if (!(FD_ISSET(fd, &fds_r) || FD_ISSET(fd, &fds_e))) flags &= ~fdofRead;
      if (!FD_ISSET(fd, &fds_w)) flags &= ~fdofWrite;
#endif
      if (flags) /* something can be done */
      { tFdObservationHandler handler = data->handler;
        void* handler_data = data->handler_data;
#if CONFIG_DEBUG
        sprint_safe(debugstrbuf,
          "fd observatory: fd=%d, flags=%d, handler=%p, handler_data=%p\n",
          fd, flags, handler, handler_data);
        debugmsg(debugstrbuf);
#endif
        (handler)(handler_data, flags);
        count--; if (count <= 0) goto finish; /* done */
      }
      do_next: data = data->next;
    }
  }
  finish: fd_observatory_cleanup(); resource_postplex();
}

void fatal_tmofd(int fd)
{
#if OPTION_POLL == 0
#if NEED_FD_REGISTER
  tFdKind kind = fd_register_lookup(&fd);
#endif
#if USE_LWIP
  if (!(kind & fdkSocket))
#endif
  { char buf[1024];
    sprint_safe(buf,
      _("%s (given value is %d, highest select()-able value is %d)"),
      _(strResourceError[reTmofd]), fd, FD_SETSIZE - 1);
    fatal_error(0, buf);
  }
#endif
  fatal_error(0, _(strResourceError[reTmofd]));
}


/* File descriptor handling III: the usual stuff */

int my_create(const char* path, int flags, mode_t mode)
{ int retval;
  unsigned char loopcount = 0;
  do
  { retval = open(path, flags | O_NOCTTY, mode);
  } while ( (retval == -1) && (errno == EINTR) && (++loopcount < 100) );
  if (retval >= 0) fd_register(&retval, fdkFile);
  return(retval);
}

int my_open(const char* path, int flags)
{ int retval;
  unsigned char loopcount = 0;
  do
  { retval = open(path, flags | O_NOCTTY);
  } while ( (retval == -1) && (errno == EINTR) && (++loopcount < 100) );
  if (retval >= 0) fd_register(&retval, fdkFile);
  return(retval);
}

void my_close(int fd)
{
#if NEED_FD_REGISTER
  tFdKind kind;
#endif
  fd_unobserve(fd);
#if NEED_FD_REGISTER
  kind = fd_unregister(&fd);
#endif
#if USE_LWIP
  if (kind & fdkSocket) (void) lwip_close(fd);
  else
#endif
  { int err;
    unsigned short loopcount = 0;
#if CONFIG_DEBUG
    char buf[100];
    sprint_safe(buf, "my_close(%d)\n", fd); debugmsg(buf);
#endif
    do
    { err = close(fd);
    } while ( (err == -1) && (errno == EINTR) && (++loopcount < 10000) );
  }
}

ssize_t my_read(int fd, void* buf, size_t count)
{ ssize_t retval;
#if NEED_FD_REGISTER
  tFdKind kind = fd_register_lookup(&fd);
#endif
#if USE_LWIP
  if (kind & fdkSocket)
  { errno = 0; /* silly old lwIP versions didn't set errno on error */
    retval = lwip_read(fd, buf, count);
  }
  else
#endif
  { unsigned char loopcount = 0;
    do
    { retval = read(fd, buf, count);
    } while ( (retval == -1) && (errno == EINTR) && (++loopcount < 100) );
  }
  return(retval);
}

ssize_t __my_write(int fd, const void* buf, size_t count)
/* may write less than <count> bytes */
{ ssize_t retval;
#if NEED_FD_REGISTER
  tFdKind kind = fd_register_lookup(&fd);
#endif
#if USE_LWIP
  if (kind & fdkSocket)
  { errno = 0; /* silly old lwIP versions didn't set errno on error */
    retval = lwip_write(fd, buf, count);
  }
  else
#endif
  { unsigned char loopcount = 0;
    do
    { retval = write(fd, buf, count);
    } while ( (retval == -1) && (errno == EINTR) && (++loopcount < 100) );
  }
  return(retval);
}

ssize_t my_write(int fd, const void* buf, size_t count)
/* will write <count> bytes (or fail on error) */
{ size_t countleft = count;
  while (countleft > 0)
  { ssize_t err = __my_write(fd, buf, countleft);
    if (err == 0) { err = -1; errno = EIO; } /* CHECKME! */
    if (err < 0) return(err);
    else if (err > (ssize_t) countleft) break; /* "can't happen" */
    countleft -= err; buf = ((char*) buf) + err;
  }
  return((ssize_t) count);
}

void my_write_crucial(int fd, const void* buf, size_t count)
{ ssize_t err = my_write(fd, buf, count);
  if (err != (ssize_t) count)
    fatal_error(((err == -1) ? errno : 0), _("write() failed"));
}

unsigned char my_mmap_file_readonly(const char* filename, void** _b,
  size_t* _s)
/* tries to open a regular file in read-only mode and to map it into memory;
   return value: 0=error, 1=empty, 2=fine */
{ unsigned char retval = 0; /* assume failure */
  int fd = my_open(filename, O_RDONLY);
  struct stat statbuf;
  size_t size;
  void* filebuf;
  if (fd < 0) goto out;
  if (my_fstat(fd, &statbuf) != 0) goto cleanup;
  if (!S_ISREG(statbuf.st_mode))
  { if (S_ISDIR(statbuf.st_mode)) errno = EISDIR;
    goto cleanup;
  }
  size = statbuf.st_size;
  if (size <= 0) { retval = 1; goto cleanup; }
  filebuf = my_mmap(size, fd);
  if (filebuf == MAP_FAILED) goto cleanup;
  my_madvise_sequential(filebuf, size);
  *_b = filebuf; *_s = size; retval = 2;
  cleanup: { int e = errno; my_close(fd); errno = e; }
  out: return(retval);
}

int my_stat(const char* filename, struct stat* sb)
{ int retval;
  unsigned char loopcount = 0;
  do
  { retval = stat(filename, sb);
  } while ( (retval == -1) && (errno == EINTR) && (++loopcount < 100) );
  return(retval);
}

int my_fstat(int fd, struct stat* sb)
{ int retval;
  unsigned char loopcount = 0;
#if NEED_FD_REGISTER
  (void) fd_register_lookup(&fd);
#endif
  do
  { retval = fstat(fd, sb);
  } while ( (retval == -1) && (errno == EINTR) && (++loopcount < 100) );
  return(retval);
}

#if NEED_FD_REGISTER

int my_pipe(int* fdpair)
{ int retval = pipe(fdpair);
  if (retval == 0)
  { fd_register(&(fdpair[0]), fdkPipe); fd_register(&(fdpair[1]), fdkPipe); }
  return(retval);
}

int my_isatty(int fd)
{ (void) fd_register_lookup(&fd);
  return(isatty(fd));
}

#endif /* #if NEED_FD_REGISTER */

#if MIGHT_FORK_EXEC && defined(FD_CLOEXEC)
void make_fd_cloexec(int fd)
{
#if NEED_FD_REGISTER
  tFdKind kind = fd_register_lookup(&fd);
#endif
#if USE_LWIP
  if (!(kind & fdkSocket))
#endif
  { int flags = fcntl(fd, F_GETFD, 0);
    if (flags != -1) (void) fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
  }
}
#endif


/* Data handling mechanism */

void dhm_generic_handler(tDhmGenericData** _dhm_data, const void* cmddata,
  tDhmCommand cmd)
{ tDhmGenericData* data = *_dhm_data;
  tDhmNotificationFlags old_flags, new_flags, flags;
  const tDhmNotificationSetup* dns;
  tDhmNotificationEntry* dne;
  const tDhmControlData* dcd;
  tDhmControlCode dcc;
  void* dcdd;
#if CONFIG_DEBUG
  static size_t depth = 0;
  depth++;
  sprint_safe(debugstrbuf,
    "dhm_generic_handler(%p, %p, %d:%d) - %s@%p, depth=%d\n", data,cmddata,cmd,
    ( (cmd == dhmcNotify) ? (*((const tDhmNotificationFlags*) cmddata)) : 0 ),
    ( (data != NULL) ? data->debugstr : strQm ), ( (data != NULL) ?
    data->object : NULL ), depth);
  debugmsg(debugstrbuf);
#endif
  switch (cmd)
  { case dhmcInit:
      data = *_dhm_data = memory_allocate(sizeof(tDhmGenericData), mapOther);
      data->object = __unconstify(void*, cmddata); break;
    case dhmcGet: data->refcount++; break;
    case dhmcPut:
      if (data->refcount > 0) data->refcount--; /* "should" be true */
      if (data->refcount <= 0)
      { dcc = dhmccRefcount0; dcdd = NULL; goto do_control; }
      break;
    case dhmcNotificationSetup:
      dns = (const tDhmNotificationSetup*) cmddata;
      dne = data->notifications;
      while (dne != NULL)
      { if ( (dne->callback == dns->callback) &&
             (dne->callback_data == dns->callback_data) )
          break; /* found */
        dne = dne->next;
      }
      old_flags = ( (dne != NULL) ? dne->flags : dhmnfNone );
      flags = dns->flags;
      switch (dns->mode)
      { case dhmnSet: new_flags = flags; break;
        case dhmnOr: new_flags = old_flags | flags; break;
        case dhmnAndnot: new_flags = old_flags & ~flags; break;
        case dhmnXor: new_flags = old_flags ^ flags; break;
        default: debugmsg("BUG: dhmn\n"); new_flags = dhmnfNone; break;
      }
      if (dne != NULL)
      { if (new_flags != dhmnfNone) dne->flags = new_flags;
        else /* lost interest */
        { list_extract(&(data->notifications), dne, tDhmNotificationEntry);
          memory_deallocate(dne);
        }
      }
      else if (new_flags != dhmnfNone)
      { dne = __memory_allocate(sizeof(tDhmNotificationEntry), mapOther);
        dne->next = data->notifications; data->notifications = dne;
        dne->callback = dns->callback; dne->callback_data = dns->callback_data;
        dne->flags = new_flags;
      }
      break;
    case dhmcNotify:
      flags = *((const tDhmNotificationFlags*) cmddata);
      dne = data->notifications;
      while (dne != NULL)
      { new_flags = flags & dne->flags;
        if (new_flags != dhmnfNone) /* this callback wants to be notified */
          (dne->callback)(dne->callback_data, new_flags);
        dne = dne->next;
      }
      if (flags & (dhmnfRemoval | dhmnfOnce))
      { /* The object might be deallocated soon, so we better clean up... */
        dne = data->notifications;
        while (dne != NULL)
        { tDhmNotificationEntry* next = dne->next;
          memory_deallocate(dne); dne = next;
        }
        memory_deallocate(data); *_dhm_data = NULL;
      }
      break;
    case dhmcControl:
      dcd = (const tDhmControlData*) cmddata;
      dcc = dcd->code; dcdd = dcd->data;
      do_control:
      if (data->control_handler != NULL)
        (data->control_handler)(data->object, dcdd, dcc);
      break;
  }
#if CONFIG_DEBUG
  sprint_safe(debugstrbuf, "dhm_generic_handler(): leaving depth %d\n", depth);
  debugmsg(debugstrbuf); depth--;
#endif
}


/* sprintf() replacement */

#if HAVE_STDARG_H
#include <stdarg.h>
#elif HAVE_VARARGS_H
#include <varargs.h>
#endif

void my_spf(char* staticbuf, size_t staticbufsize, char** spfbuf,
  const char* format, ...)
/* similar to sprintf(), but without the problem of buffer overflows; if the
   resulting string would not fit in the static buffer, a dynamic buffer is
   allocated. - Call either my_spf_cleanup() or my_spf_use() afterwards. */
/* The design of this function is based on the fact that the static buffer is
   "almost always" large enough, so we do the expensive dynamic allocation
   only in the few cases where it's absolutely necessary for correctness or
   where we'd want a dynamic buffer anyway. */
{ va_list vl;
  char numbuf[50], ch, *dest, *desttmp;
  const char* tmp;
  size_t destlen;
  tBoolean inside, is_long SHUT_UP_COMPILER(falsE);

  /* Find out how long the resulting string can become at most: */
  destlen = strlen(format) + 1;
  tmp = format; inside = falsE;
  va_start(vl, format);
  while ( (ch = *tmp++) != '\0' )
  { if (inside)
    { inside = falsE;
      if (ch == 's') /* found a "%s" specifier */
      { const char* const s = va_arg(vl, char*);
        if (s != NULL) destlen += strlen(s);
      }
      else if (ch == 'd') /* found a "%d" specifier */
      { if (is_long)
        { const long int num = va_arg(vl, long int);
          sprint_safe(numbuf, strPercld, num); /* IMPROVEME! */
        }
        else
        { const int num = va_arg(vl, int);
          sprint_safe(numbuf, strPercd, num); /* IMPROVEME! */
        }
        destlen += strlen(numbuf);
      }
      else if (ch == 'c') /* found a "%c" specifier */
      { (void) va_arg(vl, int); /* (only needed to proceed in the vl list) */
        destlen++;
      }
      else if (ch == 'p') /* found a "%p" specifier */
      { (void) va_arg(vl, void*); /* (only needed to proceed in the vl list) */
        destlen += (2 + 2 * sizeof(void*));
      }
      else if (ch == 'l') { inside = is_long = truE; destlen--; goto dontdec; }
      else goto dontdec;
      destlen -= 2; /* we'll strip the two characters "%s"/"%d"/... */
      dontdec: {}
    }
    else
    { if (ch == '%') { inside = truE; is_long = falsE; }
    }
  }
  va_end(vl);

  /* Build the string: */
  if ( (staticbuf != NULL) && (staticbufsize >= destlen) ) dest = staticbuf;
  else dest = __memory_allocate(destlen, mapString);
  desttmp = dest; tmp = format; inside = falsE;
  va_start(vl, format);
  while ( (ch = *tmp++) != '\0' )
  { if (inside)
    { inside = falsE;
      if (ch == 's') /* found a "%s" specifier */
      { const char* s = va_arg(vl, char*);
        char ch0;
        if (s != NULL) { while ( (ch0 = *s++) != '\0' ) *desttmp++ = ch0; }
      }
      else if (ch == 'd') /* found a "%d" specifier */
      { char ch0, *s = numbuf;
        if (is_long)
        { const long int num = va_arg(vl, long int);
          sprint_safe(numbuf, strPercld, num); /* IMPROVEME! */
        }
        else
        { const int num = va_arg(vl, int);
          sprint_safe(numbuf, strPercd, num); /* IMPROVEME! */
        }
        while ( (ch0 = *s++) != '\0' ) *desttmp++ = ch0;
      }
      else if (ch == 'c') /* found a "%c" specifier */
      { const char ch0 = (char) va_arg(vl, int);
        if (ch0 != '\0') *desttmp++ = ch0;
      }
      else if (ch == 'p') /* found a "%p" specifier */
      { void* p = va_arg(vl, void*);
        unsigned long int l = (unsigned long int) p;
        unsigned short count = 2 * sizeof(void*);
        char* t;
        *desttmp++ = '0'; *desttmp++ = 'x';
        t = desttmp = desttmp + count;
        while (count-- > 0)
        { *--t = strHexnum[l & 15];
          l >>= 4;
        }
        /* CHECKME: don't output leading zeroes? */
      }
      else if (ch == 'l') { inside = is_long = truE; }
      else goto append;
    }
    else
    { if (ch == '%') { inside = truE; is_long = falsE; }
      else { append: *desttmp++ = ch; }
    }
  }
  va_end(vl);
  *desttmp = '\0';
  *spfbuf = dest;
}


/* Replacements for broken/missing C library functions; we don't care about
   performance here - the only purpose of these functions is to make the
   program compile and work, nothing more. */

#if !HAVE_MMAP
void* nonbroken_mmap(size_t len, int fd)
{ if (len > 0)
  { void *retval = __memory_allocate(len, mapOther), *p = retval;
    size_t lenleft = len;
    while (1)
    { ssize_t err = my_read(fd, p, lenleft);
      if (err <= 0) { memory_deallocate(retval); goto failed; }
      else if (err >= (ssize_t) lenleft) return(retval); /* done */
      lenleft -= err; p += err;
    }
  }
  failed:
  return(MAP_FAILED);
}
#endif

#if !HAVE_STRCASECMP
int nonbroken_strcasecmp(const char* _s1, const char* _s2)
{ const unsigned char *s1 = (const unsigned char*) _s1,
    *s2 = (const unsigned char*) _s2;
  while (1)
  { unsigned char _a = *s1++, a = my_tolower(_a),
      _b = *s2++, b = my_tolower(_b);
    if (a != b) return(a - b);
    else if (a == '\0') return(0);
  }
}
#endif

#if 0
#if !HAVE_STRNCASECMP
int nonbroken_strncasecmp(const char* _s1, const char* _s2, size_t n)
/* Nowadays we seem to use strneqcase() instead of my_strncasecmp() everywhere,
   so REMOVEME? */
{ const unsigned char *s1 = (const unsigned char*) _s1,
    *s2 = (const unsigned char*) _s2;
  while (n-- > 0)
  { unsigned char _a = *s1++, a = my_tolower(_a),
      _b = *s2++, b = my_tolower(_b);
    if (a != b) return(a - b);
    else if (a == '\0') return(0);
  }
  return(0);
}
#endif
#endif

#if !HAVE_STRCHR
char* nonbroken_strchr(const char* s, int _wanted_ch)
{ const char wanted_ch = (char) _wanted_ch;
  char ch;
  while ( (ch = *s) != '\0' )
  { if (ch == wanted_ch) return(unconstify(s));
    s++;
  }
  return(NULL);
}
#endif

#if !HAVE_STRRCHR
char* nonbroken_strrchr(const char* s, int _wanted_ch)
{ size_t len = strlen(s);
  if (len > 0)
  { const char wanted_ch = (char) _wanted_ch;
    s += len - 1;
    while (len-- > 0)
    { if (*s == wanted_ch) return(unconstify(s));
      s--;
    }
  }
  return(NULL);
}
#endif

#if !HAVE_STRSTR
char* nonbroken_strstr(const char* haystack, const char* needle)
{ const size_t hlen = strlen(haystack), nlen = strlen(needle);
  size_t h = 0, n = 0;
  do
  { if (haystack[h] == needle[n]) n++;
    else { h -= n; n = 0; }
    h++;
  } while ( (n < nlen) && (h < hlen) );
  if (n >= nlen) return(unconstify(haystack) + h - nlen - 1); /* found */
  else return(NULL);
}
#endif
