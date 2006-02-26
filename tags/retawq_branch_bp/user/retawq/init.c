/* retawq/init.c - initialization
   This file is part of retawq (<http://retawq.sourceforge.net/>), a network
   client created by Arne Thomassen; retawq is basically released under certain
   versions of the GNU General Public License and WITHOUT ANY WARRANTY.
   Read the file COPYING for license details, README for program information.
   Copyright (C) 2001-2005 Arne Thomassen <arne@arne-thomassen.de>
*/

#include "stuff.h"
#include "init.h"
#include "parser.h"
#include "resource.h"
#if CONFIG_JAVASCRIPT
#include "javascript.h"
#endif

#if OPTION_BIRTCFG
#include "birtcfg.inc"
#endif

#if CAN_HANDLE_SIGNALS
#include <signal.h>
#endif

#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_UNAME
#include <sys/utsname.h>
#endif

declare_local_i18n_buffer

#define KILO (1024)
#define MEGA (1024 * 1024)

static char strbuf[STRBUF_SIZE];

static const char strEn[] = "en", strReverse[] = "reverse", strNone[] = "none";

#define MIGHT_USE_TERMWINTITLE ( (TGC_IS_CURSES) && (CONFIG_TG != TG_XCURSES) && (CONFIG_BLOAT & BLOAT_TERMWINTITLE) )

#define MIGHT_HIGHLIGHT ((TGC_IS_CURSES || (CONFIG_TG == TG_CONSOLE)) && (CONFIG_BLOAT & BLOAT_COLORHL) && (MIGHT_USE_COLORS))

#if MIGHT_HIGHLIGHT
static unsigned char color_highlight = 0; /* for tColorPairNumber 0..7 */
#endif

static void __init store_initmsg(const char* msg)
{ char* spfbuf;
  my_spf(NULL, 0, &spfbuf, "%s\n<br>%s", null2empty(initial_messages), msg);
  __dealloc(initial_messages); initial_messages = my_spf_use(spfbuf);
#if CONFIG_CONSOLE
  { const size_t len = strlen(msg); /* IMPROVEME! */
    const tBoolean has_newline = cond2boolean((len > 0) && (msg[len-1]=='\n'));
    my_spf(NULL, 0, &spfbuf, "%s%s%s", null2empty(initial_console_msgs), msg,
      (has_newline ? strEmpty : strNewline));
    __dealloc(initial_console_msgs); initial_console_msgs = my_spf_use(spfbuf);
    /* CHECKME: this wastes memory unnecessarily if not pmConsole! */
  }
#endif
}

static void __init show_warning(const char* str)
{ const char* strR = _("retawq: ");
  size_t count = strlen(strR);
  char buf[1024], *spfbuf, *temp;
  my_spf(buf, 1024, &spfbuf, _("%swarning: %s\n"), strR, str);
    /* ugly colon-ialism... */
  if ( (lfdmbs(2))
#if CONFIG_CONSOLE
       && (program_mode != pmConsole) /* msg would be duplicate on console */
#endif
     )
  { my_write_str(fd_stderr, spfbuf); }

  /* call store_initmsg(), but skip the "retawq: " prefix; the code looks
     somewhat complicated because we don't trust the last _() call much;
     otherwise we could just say e.g. "temp = spfbuf + strlen(strR);"... */
  temp = spfbuf;
  while (count-- > 0)
  { if (*temp == '\0') goto out; /* "should not happen" */
    temp++;
  }
  *temp = my_toupper(*temp);
  store_initmsg(temp);
  out:
  my_spf_cleanup(buf, spfbuf);
}


/* Command-line options */

static one_caller void __init precheck_commandline(int argc, const char** argv)
/* This is done early during execution of the program in order to avoid
   unnecessary initializations (especially the expensive gtk_init() call in
   graphics mode) if there's a reason for premature exit, e.g. the command-line
   option "--version". */
{ while (--argc > 0)
  { const char* str = *++argv;
    if (*str != '-') { /* nothing; fastest check first, avoid lib calls */ }
    else if ( (!strcmp(str, strOptVersion)) || (!strcmp(str, "-v")) )
    { /* if (lfdmbs(1)) */
      { my_write_str(fd_stdout, strCopyright);
        my_write_str(fd_stdout, strProgramLegalese);
      }
      do_quit();
    }
#if CONFIG_BLOAT & BLOAT_HELP
    else if ( (!strcmp(str, strOptHelp)) || (!strcmp(str, "-h")) )
    { /*CHECKME: i18n? */
      static const char h[] __initdata = "Documentation is available in the directory docu/ in the source code package,\nin the directory \"" PATH_INSTALL_DOC "\" (if installed)\nand at <http://retawq.sourceforge.net/docu/>.\n";
      /* if (lfdmbs(1)) */
      { my_write_str(fd_stdout, strCopyright); my_write_str(fd_stdout, h);
#if CONFIG_BLOAT & BLOAT_HELPARG
        { static const char h2[] __initdata = "\n"
"Command-line options:\n"
#if MIGHT_USE_COLORS
"--colors=off - disable colors\n"
"--colors=reverse - use black-on-white colors\n"
#endif
#if CONFIG_CONSOLE
"--console - enter console runmode\n"
#endif
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
"--download=URL - download content from URL, write it to stdout; quit\n"
#endif
#if CONFIG_EXTRA & EXTRA_DUMP
"--dump=URL - receive document from URL; write it to stdout (layouted); quit\n"
#endif
"--help, -h - print this help text; quit\n"
#if DO_PAGER
"--pager - run as a simple pager\n"
#endif
#if CONFIG_SESSIONS
"--resume-session - resume from default session file\n"
"--resume-session=FILE - resume from given session file\n"
#endif
#if CONFIG_RTCONFIG
"--rtcfg=LETTERS - use "
#if OPTION_BIRTCFG
"built-in/external "
#endif
"run-time configuration?\n"
#endif
"--userdir=PATH - set retawq's user directory\n"
"--version, -v - print version information; quit\n"
"Additionally, you can specify the URLs of one or two documents to be shown.\n"
;
          my_write_str(fd_stdout, h2);
        }
#endif
      }
      do_quit();
    }
#endif
  }
}

static one_caller void __init process_commandline(int argc, const char** argv)
{ while (--argc > 0)
  { const char* str = *++argv;
    if (*str != '-') /* (fastest check first) */
    { if ( (is_environed) && (*str != '\0') && (launch_uri_count < 2) )
        launch_uri[launch_uri_count++] = str;
    }
    else if (!strncmp(str, "--userdir=", 10))
    { const char* temp = str + 10;
      if (*temp != '\0')
      { char* path;
        __dealloc(config.path);
        my_spf(NULL, 0, &path, strPercsPercs, temp,
          ( (temp[strlen(temp) - 1] != chDirsep) ? strDirsep : strEmpty ));
        config.path = my_spf_use(path);
        config.flags |= cfUserDefinedConfigPath;
      }
    }
    else if (!strncmp(str, "--resume-session", 16))
    {
#if CONFIG_SESSIONS
      const char* temp = str + 16;
      char ch = *temp;
      if (ch == '\0') my_strdedup(config.session_resume, strEmpty);
      else if (ch == '=')
      { temp++;
        if (*temp != '\0') my_strdedup(config.session_resume, temp);
        else goto uclo;
      }
      else goto uclo;
#else
      char* spfbuf;
      my_spf(strbuf, STRBUF_SIZE, &spfbuf, _(strFwdact), _(strFsessions) + 1,
        strEmpty);
      show_warning(spfbuf);
      my_spf_cleanup(strbuf, spfbuf);
#endif
    }
#if CONFIG_EXTRA & EXTRA_DOWNLOAD
    else if (!strncmp(str, "--download=", 11))
    { const char* temp = str + 11;
      if (*temp != '\0')
      { my_strdedup(config.pm_uri, temp); program_mode = pmDownload; }
    }
#endif
#if CONFIG_EXTRA & EXTRA_DUMP
    else if (!strncmp(str, "--dump=", 7))
    { const char* temp = str + 7;
      if (*temp != '\0')
      { my_strdedup(config.pm_uri, temp); program_mode = pmDump; }
    }
#endif
#if CONFIG_CONSOLE
    else if (!strcmp(str, "--console")) program_mode = pmConsole;
#endif
#if DO_PAGER
    else if (!strncmp(str, "--pager", 7))
    { const char* temp = str + 7;
      char ch;
      program_mode = pmPager;
      if (*temp == '\0') continue;
      else if (*temp != '=') goto uclo;
      temp++;
      while ( (ch = *temp++) != '\0' ) { /* IMPLEMENTME: h, r, s! */ }
    }
#endif
#if 0 /* IMPLEMENTME? */
    else if (!strncmp(str, "--batch=", 8))
    { const char* temp = str + 8;
      if (*temp != '\0')
      { my_strdedup(config.batchfile, temp); program_mode = pmBatch; }
    }
#endif
#if MIGHT_USE_COLORS
    else if (!strncmp(str, "--colors=", 9))
    { const char* temp = str + 9;
      if (!strcmp(temp, strOff)) config.flags |= cfColorsOff;
#if TGC_IS_CURSES
      else if (!strcmp(temp, strReverse)) config.flags |= cfColorsReverse;
#endif
    }
#endif
#if MIGHT_HIGHLIGHT
    else if (!strncmp(str, "--colorhl=", 10))
    { const char* temp = str + 10;
      char *hl, *pos;
      tColorPairNumber cpn;
      tBoolean is_last, negate;
      char ch;
      if (strlen(temp) > STRBUF_SIZE / 2) goto hl_out; /* can't be serious */
      strcpy(strbuf, temp); hl = strbuf; is_last = falsE;
      hl_loop:
      ch = *hl;
      if (ch == '\0') goto hl_out;
      if (ch == '!') { hl++; negate = truE; }
      else negate = falsE;
      pos = hl;
      while (1)
      { ch = *pos;
        if (ch == '\0') { is_last = truE; break; }
        else if (ch == ',') { *pos = '\0'; break; }
        pos++;
      }
      if (!strcmp(hl, "red"))
      { cpn = cpnRed;
        hl_flip:
        if (negate) color_highlight &= ~(1 << cpn);
        else color_highlight |= (1 << cpn);
      }
      else if (!strcmp(hl, "green")) { cpn = cpnGreen; goto hl_flip; }
      else if (!strcmp(hl, "yellow")) { cpn = cpnYellow; goto hl_flip; }
      else if (!strcmp(hl, "blue")) { cpn = cpnBlue; goto hl_flip; }
      else if (!strcmp(hl, "all")) color_highlight = ~((unsigned char) 0);
      else if (!strcmp(hl, strNone)) color_highlight = 0;
      if (!is_last) { hl = pos + 1; goto hl_loop; }
      hl_out: {}
    }
#endif
#if CONFIG_RTCONFIG
    else if (!strncmp(str, "--rtcfg=", 8))
    { const char* temp = str + 8;
      char ch;
      config.flags &= ~cfRtAll;
      while ( (ch = *temp++) != '\0' )
      { if (ch == 'e') config.flags |= cfRtExternal;
#if OPTION_BIRTCFG
        else if (ch == 'b') config.flags |= cfRtBuiltin;
#endif
      }
    }
#endif
    else
    { char* spfbuf;
      uclo:
      my_spf(strbuf, STRBUF_SIZE, &spfbuf,
        _("unknown command-line option: \"%s\"%s"), str,
#if (CONFIG_BLOAT & BLOAT_HELP) && (CONFIG_BLOAT & BLOAT_HELPARG)
        _("; try \"retawq --help\"")
#else
        strEmpty
#endif
        );
      fatal_error(0, spfbuf);
      /* my_spf_cleanup(strbuf, spfbuf); */
    }
  }
}


/* Internationalization */

#if OPTION_I18N

static /*@relnull@*/ /*@reldef@*/ const char* current_locale;

static one_caller void __init initialize_i18n(void)
{ (void) setlocale(LC_ALL, strEmpty);
  (void) bindtextdomain(strRetawq, PATH_INSTALL_LOCALE);
  (void) textdomain(strRetawq);
  current_locale = setlocale(LC_MESSAGES, NULL);
}

/* begin-autogenerated */
#define NUM_LANGUAGES 5
static const char* const language[NUM_LANGUAGES] = { "de", strEn, "es", "fr", "pt_BR" };
/* end-autogenerated */

static one_caller tMbsIndex __init lookup_language_code(void)
{ my_binary_search(0, NUM_LANGUAGES - 1, strncmp(current_locale,
    language[idx], 2), return(idx))
}

#endif /* #if OPTION_I18N */

static my_inline char __init str2char_(const char* const str)
/* translates the string and returns the first character of the result */
{
#if OPTION_I18N
  const char* const istr = _(str);
  return(*istr); /* ISTR that "ISTR" means "I seem to remember", but... :-) */
#else
  return(*str);
#endif
}


/* Text terminal preparation */

#if MIGHT_DO_TERMIOS
static void __init prepare_terminal(void)
{ struct termios t;
  my_memclr_var(t); /* for all those buggy libraries... */
  if (tcgetattr(0, &t) == 0)
  { int err;
    unsigned char loopcount = 0;
    saved_termios = t; /* save the old settings */
    t.c_iflag &= ~(INLCR | ISTRIP); t.c_iflag |= ICRNL;
    t.c_oflag &= ~(OCRNL | ONOCR | ONLRET); t.c_oflag |= OPOST | ONLCR;
    t.c_cflag &= ~(PARENB | CSIZE); t.c_cflag |= CS8; /* enable eight-bit */
    t.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL); /* disable echo */
    t.c_lflag &= ~ICANON; /* disable canonical mode */
    t.c_lflag |= ISIG; t.c_cc[VMIN] = 1; t.c_cc[VTIME] = 0;
    loop:
    err = tcsetattr(0, TCSAFLUSH, &t);
    if (err == 0) do_restore_termios = truE;
    else if ( (err == -1) && (errno == EINTR) && (++loopcount < 100) )
      goto loop;
  }
}
#endif


/* Curses library for text terminal control */

#if TGC_IS_CURSES

#if (CONFIG_TG == TG_BICURSES) || (CONFIG_TG == TG_XCURSES)

static tBoolean __init env2num(const char* str, /*@out@*/ int* _value,
  int minv, int maxv)
{ tBoolean retval = falsE;
  const char* vstr = getenv(str);
  if ( (vstr != NULL) && (my_isdigit(*vstr)) )
  { my_atoi(vstr, _value, &vstr, maxv);
    if ( (*vstr == '\0') && (*_value >= minv) ) retval = truE;
  }
  return(retval);
}

tBoolean __init env_termsize(int* _x, int* _y)
{ return(cond2boolean(env2num("COLUMNS", _x, CURSES_MINCOLS, CURSES_MAXCOLS) &&
    env2num("LINES", _y, CURSES_MINLINES, CURSES_MAXLINES)));
}

#endif /* bicurses/xcurses */

static one_caller void __init initialize_curses(void)
{ if (initscr() == NULL) fatal_error(0, _("can't initialize terminal"));
    /* (Some old curses libraries, e.g. on Minix 2.0, might return NULL instead
        of exiting with an error message.) */
  need_tglib_cleanup = truE;

#if MIGHT_USE_COLORS
  if (!(config.flags & cfColorsOff))
  { if ( (start_color() != ERR) && (has_colors() == TRUE) &&
         (COLOR_PAIRS > cpnMax) )
    { use_colors = truE; }
  }
#endif

  (void) cbreak(); (void) noecho(); (void) nonl();
  (void) intrflush(stdscr, FALSE); (void) keypad(stdscr, TRUE);
  (void) nodelay(stdscr, TRUE);

#if (CONFIG_DO_TEXTMODEMOUSE) && (!OFWAX)
  (void) mousemask(TEXTMODEMOUSE_MASK, NULL);
#endif

#if MIGHT_USE_COLORS
  if (use_colors)
  { const tBoolean do_rev = cond2boolean(config.flags & cfColorsReverse);
    tColorCode bg = (do_rev ? ccWhite : ccBlack);
    tColorPairNumber i;
    (void) init_pair(cpnDefault, (do_rev ? ccBlack : ccWhite), bg);
    (void) init_pair(cpnRed, ccRed, bg);
    (void) init_pair(cpnGreen, ccGreen, bg);
    (void) init_pair(cpnYellow, ccYellow, bg);
    (void) init_pair(cpnBlue, ccBlue, bg);
    for (i = cpnDefault; i <= cpnMax; i++)
    { tColorBitmask cb = COLOR_PAIR(i);
#if MIGHT_HIGHLIGHT
      if (color_highlight & (1 << i)) cb |= A_BOLD;
#endif
      color_bitmask[i] = cb;
    }
    bkgdset(COLOR_PAIR(cpnDefault));
    my_set_color(cpnDefault);
  }
#endif

#if MIGHT_USE_TERMWINTITLE
  if ( (config.flags & cfTermwintitle) && (lfdmbs(1)) )
  { const char* str;
    /* The testing order is important - we can have screen run inside xterm. */
    if ( ( (str = getenv("STY")) != NULL ) && (*str != '\0') )
    { /* GNU screen terminal */
      sprint_safe(strbuf, "\033k%s\033\\", strProgramVersion);
      my_write_str(fd_stdout, strbuf); debugmsg("STY\n");
    }
    else if ( ( (str = getenv("DISPLAY")) != NULL ) && (*str != '\0') )
    { /* xterm-like terminal */
      sprint_safe(strbuf, "\033]0;%s\a", strProgramVersion);
      my_write_str(fd_stdout, strbuf); debugmsg("DISPLAY\n");
    }
  }
#endif
}

#endif /* #if TGC_IS_CURSES */


/* Signals */

#if CAN_HANDLE_SIGNALS

#if (!defined(SIGCHLD)) && defined(SIGCLD)
#define SIGCHLD (SIGCLD) /* synonyms */
#endif

#define HANDLE_SIGCHLD (MIGHT_FORK_EXEC && defined(SIGCHLD))

#define signal_concerns_terminal(number) \
  ( ((number) == SIGINT) || ((number) == SIGQUIT) || ((number) == SIGHUP) )

#if HAVE_SIGACTION

static one_caller tBoolean __initfe signal_is_ignored(int signal_number)
{ struct sigaction signal_handler;
  return( (sigaction(signal_number, NULL, &signal_handler) == 0) ?
    cond2boolean(signal_handler.sa_handler == SIG_IGN) : falsE );
}

static void __initfe my_sigaction(int signal_number, void (*handler)(int))
{ struct sigaction signal_handler;
  if ( (!is_promptable) && (signal_concerns_terminal(signal_number)) &&
       (signal_is_ignored(signal_number)) )
  { /* We're running as an automated job, so we don't wanna handle
       terminal-related signals that we should rather ignore. */
    return;
  }
  my_memclr_var(signal_handler);
  signal_handler.sa_handler = handler;
  signal_handler.sa_flags = SA_RESTART;
#if HANDLE_SIGCHLD
  if (signal_number == SIGCHLD) signal_handler.sa_flags |= SA_NOCLDSTOP;
#endif
  (void) sigemptyset(&signal_handler.sa_mask);
  (void) sigaction(signal_number, &signal_handler, NULL);
}

#define signal_reestablish(sig, handler) do { /* nothing to do */ } while (0)

#else /* #if HAVE_SIGACTION */

/* We don't have sigaction(), so we use signal(). SUSv3 says, "Use of this
   function is unspecified in a multi-threaded process.", but we use it mostly
   at initialization time (or right after fork()), before extra threads are
   created. The only problem is signal_reestablish(), but we don't have a
   choice, so... */

static void __initfe my_sigaction(int signal_number, void (*handler)(int))
{ void (*old)(int);
  old = signal(signal_number, handler);
  if ( (!is_promptable) && (signal_concerns_terminal(signal_number)) &&
       (old == SIG_IGN) && (handler != SIG_IGN) )
  { (void) signal(signal_number, old); /* set it back */
  }
}

#define signal_reestablish(sig, handler) (void) signal(sig, handler)

#endif /* #if HAVE_SIGACTION */

#define notify_main(addr) (void) my_write_pipe(fd_any2main_write, addr, 4)

static void my_termination_signal_handler(__sunused int signal_number
  __cunused)
/* long name, short action :-) */
{ notify_main(strQuit);
}

#if CONFIG_CONSOLE
static void my_console_discard_handler(__sunused int signal_number __cunused)
{ notify_main(strConsoleDiscard);
  signal_reestablish(SIGINT, my_console_discard_handler);
}
#endif

#if defined(TIOCGWINSZ)

#define MY_TS_IOCTL (TIOCGWINSZ)
#define MY_TS_STRUCT struct winsize
#define MY_TS_X ws_col
#define MY_TS_Y ws_row

#elif defined(TIOCGSIZE)

#define MY_TS_IOCTL (TIOCGSIZE)
#define MY_TS_STRUCT struct ttysize
#define MY_TS_X ts_cols
#define MY_TS_Y ts_lines

#endif

#define MIGHT_CALC_TERMSIZE ( (MIGHT_WANT_TERMSIZE) && (defined(MY_TS_IOCTL)) )
#define MIGHT_SIG_TERMSIZE ( (MIGHT_CALC_TERMSIZE) && (defined(SIGWINCH)) && (HAVE_CURSES_RESIZETERM) )

#if MIGHT_WANT_TERMSIZE
tBoolean calc_termsize(int* _x, int* _y)
{ tBoolean retval = falsE;
#if MIGHT_CALC_TERMSIZE
  MY_TS_STRUCT window_size;
  unsigned char loopcount = 0;
  int err;
  loop:
  my_memclr_var(window_size); /* (extra care) */
  err = ioctl(1, MY_TS_IOCTL, &window_size);
  if (err == 0)
  { if ( ( (*_x = window_size.MY_TS_X) > 0 ) &&
         ( (*_y = window_size.MY_TS_Y) > 0 ) )
      retval = truE;
  }
  else if ( (err == -1) && (errno == EINTR) && (++loopcount < 100) ) goto loop;
#endif
  return(retval);
}
#endif

#if MIGHT_SIG_TERMSIZE
static void my_sigwinch_handler(__sunused int signal_number __cunused)
{ int cols, rows;
  if (calc_termsize(&cols, &rows))
  { if ( (cols >= CURSES_MINCOLS) && (cols <= CURSES_MAXCOLS) &&
         (rows >= CURSES_MINLINES) && (rows <= CURSES_MAXLINES) )
    { char buf[4];
      unsigned char* const temp = (unsigned char*) buf;
      buf[0] = 'w'; /* buf[3] = <whatever>; -- irrelevant */
      temp[1] = (unsigned char) cols; temp[2] = (unsigned char) rows;
      notify_main(buf);
    }
  }
  signal_reestablish(SIGWINCH, my_sigwinch_handler);
}
#endif /* #if MIGHT_SIG_TERMSIZE */

#if HANDLE_SIGCHLD

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

static void my_sigchld_handler(__sunused int signal_number __cunused)
{ int dummy;
#if HAVE_WAITPID
  while (waitpid(-1, &dummy, WNOHANG) > 0) { /* nothing */ }
#elif HAVE_WAIT3
  while (wait3(&dummy, WNOHANG, NULL) > 0) { /* nothing */ }
#else
  (void) wait(&dummy);
#endif
  signal_reestablish(SIGCHLD, my_sigchld_handler);
}

#endif /* #if HANDLE_SIGCHLD */

static one_caller void __init initialize_signals(void)
{ my_sigaction(SIGHUP, my_termination_signal_handler);
  my_sigaction(SIGTERM, my_termination_signal_handler);
  my_sigaction(SIGQUIT, my_termination_signal_handler);
  my_sigaction(SIGINT, (
#if CONFIG_CONSOLE
    (program_mode == pmConsole) ? my_console_discard_handler :
#endif
    (1 ? (SIG_IGN) : my_termination_signal_handler) ));
#if MIGHT_SIG_TERMSIZE
  if (is_environed) my_sigaction(SIGWINCH, my_sigwinch_handler);
#endif
#if HANDLE_SIGCHLD
  my_sigaction(SIGCHLD, my_sigchld_handler);
#endif

  my_sigaction(SIGPIPE, SIG_IGN);
#ifdef SIGIO
  my_sigaction(SIGIO, SIG_IGN);
#endif
  /* Nowadays the standard defines "ignore this signal" as default handling for
     SIGURG, but formerly (e.g. SUSv3 before issue 5) it was "abnormal
     termination". *SIGh* :-) We wanna be sure: */
  my_sigaction(SIGURG, SIG_IGN);
}

#if MIGHT_FORK_EXEC
void reset_signals(void)
/* Always call this right after fork() in the child process. The signals
   handled here should be the same as in initialize_signals() - except SIGURG,
   SIGPIPE and SIGIO, which should stay ignored. */
{ my_sigaction(SIGHUP, SIG_DFL);
  my_sigaction(SIGINT, SIG_DFL);
  my_sigaction(SIGTERM, SIG_DFL);
  my_sigaction(SIGQUIT, SIG_DFL);
#if MIGHT_SIG_TERMSIZE
  if (is_environed) my_sigaction(SIGWINCH, SIG_DFL);
#endif
#if HANDLE_SIGCHLD
  my_sigaction(SIGCHLD, SIG_DFL);
#endif
}
#endif

#endif /* #if CAN_HANDLE_SIGNALS */


/* Run-time configuration */

#if CONFIG_RTCONFIG

static const char strCache[] __initdata = "cache";
#if OPTION_COOKIES || OPTION_LOCAL_CGI
static const char strAllow[] __initdata = "allow",
  strDeny[] __initdata = "deny",
  strAllowDenyExp[] __initdata = N_("\"allow\" or \"deny\" expected");
#endif

static unsigned char allow_uname_in_user_agent __initdata = 0;

my_enum1 enum
{ cscUnknown = 0, cscBookmarks = 1, cscCache = 2, cscColors = 3,
  cscDontConfirm = 4, cscExecextShell = 5, cscFtpLogin = 6, cscFtpsMethod = 7,
  cscHome = 8, cscHttpCookies = 9, cscHttpProxies = 10, cscHttpVersion = 11,
  cscHttpsCookies = 12, cscHttpsProxies = 13, cscJumps = 14, cscKeymap = 15,
  cscLanguages = 16, cscLocalCgi = 17, cscLocalDirFormat = 18,
  cscLocalDirSort = 19, cscMayWrite = 20, cscNewsServerDefault = 21,
  cscRedirections = 22, cscScrollBars = 23, cscSearchEngine = 24,
  cscTermwintitle = 25, cscUserAgent = 26
} my_enum2(unsigned char) tConfigSectionCode;
#define MAX_CSC (26)
static tConfigSectionCode csc;

static const char* const strCsc[MAX_CSC + 1] __initdata =
{ "U"/*nknown*/, "bookmarks", strCache, "colors", "dont-confirm",
  "execext-shell", "ftp-login", "ftps-method", "home", "http-cookies",
  "http-proxies", "http-version", "https-cookies", "https-proxies", "jumps",
  "keymap", "languages", "local-cgi", "local-dir-format", "local-dir-sort",
  "may-write", "news-server-default", "redirections", "scroll-bars",
  "search-engine", "termwintitle", "user-agent"
};

static tBoolean inside_comment, is_line_indented;
#if OPTION_COOKIES || OPTION_LOCAL_CGI
static tBoolean allow_this;
#endif

#if CONFIG_KEYMAPS
my_enum1 enum
{ kkUnknown = 0, kkCommand = 1, kkLineInput = 2
} my_enum2(unsigned char) tKeymapKind;
static tKeymapKind current_keymap_kind;
#endif

static size_t linecount, wordcount;
static const char /*@null@*/ *wordstart, *ptr;
#if CONFIG_FTP
static /*@null@*/ char *ftp_login_user = NULL, *ftp_login_password = NULL;
static /*@null@*/ tConfigLogin* last_ftp_login = NULL;
#endif
#if CONFIG_KEYMAPS
static /*@null@*/ char* current_keymap_keystr = NULL;
#endif

#define IS_NEWLINE(ch) ( ((ch) == '\n') || ((ch) == '\r') )
#define IS_WHITESPACE(ch) ( ((ch) == ' ') || ((ch) == '\t') )

#define wordlen (ptr - wordstart - 1)

static tBoolean __init is_word(const char* str)
{ const size_t len = wordlen;
  return(cond2boolean( (len > 0) && (len == strlen(str)) &&
    (!my_memdiff(wordstart, str, len)) ));
}

static void __init copy_word(char* dest)
{ size_t len = wordlen, max = STRBUF_SIZE / 2;
  if (len > max) len = max; /* can't be serious */
  my_memcpy(dest, wordstart, len);
  dest[len] = '\0';
}

#define strdup_word(dest) dest = my_strndup(wordstart, wordlen)
#define strdup_word_tolower(dest) dest = my_strndup_tolower(wordstart, wordlen)

static void __init warn_section(tConfigSectionCode c, const char* str)
{ char buf[1024], *spfbuf;
  my_spf(buf, 1024, &spfbuf, _("configuration section %s (line %d): %s"),
    strCsc[c], linecount, str);
  show_warning(spfbuf);
  my_spf_cleanup(buf, spfbuf);
}

static one_caller void __init warn_section_fwdact(tConfigSectionCode c,
  const char* str)
{ char* spfbuf;
  if (*str == 'F') str++;
  my_spf(strbuf, STRBUF_SIZE, &spfbuf, _(strFwdact), str,
    _("; skipping section"));
  warn_section(c, strbuf);
  my_spf_cleanup(strbuf, spfbuf);
}

static my_inline void __init warn_section_wordcount(void)
{ warn_section(csc, _("too many arguments in line"));
  inside_comment = truE; /* ignore the rest of the line */
}

static one_caller tMbsIndex __init do_lookup_csc(void)
{ my_binary_search(0, MAX_CSC, strcmp(strbuf, strCsc[idx]), return(idx))
}

static one_caller void __init lookup_csc(void)
{ copy_word(strbuf); csc = cscUnknown;
  if (wordlen > 0)
  { tMbsIndex idx = do_lookup_csc();
    if (idx > 0) csc = (tConfigSectionCode) idx;
  }
  if (csc == cscUnknown)
  { char buf[1024], *spfbuf;
    my_spf(buf, sizeof(buf), &spfbuf,
      _("unknown section name \"%s\"; skipping section"), strbuf);
    show_warning(spfbuf);
    my_spf_cleanup(buf, spfbuf);
  }
}

static void __init interpret_portnumber(char* str, tPortnumber* dest,
  tPortnumber _default, tBoolean none2zero)
{ tPortnumber portnumber;
  char* temp;
  if ( (str != NULL) && ( (temp = my_strrchr(str, ':')) != NULL ) &&
       (my_strchr(temp + 1, ']') == NULL) ) /* port, not IPv6 address part */
  { int _portnumber;
    *temp = '\0';
    my_atoi(temp + 1, &_portnumber, NULL, 99999);
    if ( (_portnumber >= 0) && (_portnumber <= 65535) )
      portnumber = (tPortnumber) htons((tPortnumber) _portnumber);
    else portnumber = _default;
  }
  else portnumber = (none2zero ? 0 /*==htons(0)*/ : _default);
  *dest = portnumber;
}

#if OPTION_TLS
#define first_proxy \
  ( (csc == cscHttpsProxies) ? config.https_proxies : config.http_proxies )
#else
#define first_proxy (config.http_proxies)
#endif

static one_caller void __init handle_proxylinepart(void)
{ if (wordcount == 0)
  { tConfigProxy* p = memory_allocate(sizeof(tConfigProxy), mapPermanent);
    p->next = first_proxy;
#if OPTION_TLS
    if (csc == cscHttpsProxies) config.https_proxies = p;
    else
#endif
    { config.http_proxies = p; }
    if (!is_word(strNone)) strdup_word_tolower(p->proxy_hostname);
    interpret_portnumber(p->proxy_hostname, &(p->proxy_portnumber),
      (tPortnumber) htons(8080), truE);
  }
  else
  { tConfigProxy* p = first_proxy;
    if (p == NULL) return; /* "should not happen" */
    switch (wordcount)
    { case 1:
        strdup_word_tolower(p->hosts_pattern);
        interpret_portnumber(p->hosts_pattern, &(p->hosts_portnumber), 0,truE);
        break;
#if CONFIG_HTTP & HTTP_PROXYAUTH
      case 2: strdup_word(p->auth_user); break;
      case 3: strdup_word(p->auth_pass); break;
#endif
      default: warn_section_wordcount(); break;
    }
  }
}

static void __init handle_word(void)
{ if ( (wordstart == NULL) || (wordlen <= 0) ) return; /* nothing to do */
  if (!is_line_indented) /* section-header line */
  { char* temp;
    if (wordcount == 0) /* start of a new section */
    { const char* dis = NULL;
      lookup_csc();
      switch (csc)
      {
#if CONFIG_LOCALDIR <= 1
        case cscLocalDirSort: case cscLocalDirFormat:
          dis = _("Flocal directory options"); break;
#endif
#if !OPTION_COOKIES
        case cscHttpCookies: case cscHttpsCookies: dis = _("Fcookies"); break;
#endif
#if !OPTION_LOCAL_CGI
        case cscLocalCgi: dis = _("Flocal CGI"); break;
#endif
#if (!(OPTION_EXECEXT & EXECEXT_SHELL))
        case cscExecextShell: dis = _(strFexecext); break;
#endif
#if !OPTION_NEWS
        case cscNewsServerDefault: dis = _("Fnews"); break;
#endif
#if !CONFIG_FTP
#if !OPTION_TLS
        case cscFtpsMethod:
#endif
        case cscFtpLogin: dis = "FFTP"; break;
#endif
#if !MIGHT_USE_COLORS
        case cscColors: dis = _("Fcolors"); break;
#endif
#if !CONFIG_JUMPS
        case cscJumps: dis = _(strFjumps); break;
#endif
#if !CONFIG_KEYMAPS
        case cscKeymap: dis = _("Fkeymaps"); break;
#endif
#if !OPTION_TLS
#if OPTION_COOKIES /* (don't let the same case value occur more than once) */
        case cscHttpsCookies:
#endif
#if CONFIG_FTP /* (don't let the same case value occur more than once) */
        case cscFtpsMethod:
#endif
        case cscHttpsProxies: dis = "FTLS"; break;
#endif
        default: break; /* (some compilers reject empty switch blocks) */
      }
      if (dis != NULL) { warn_section_fwdact(csc, dis); csc = cscUnknown; }
      if (csc == cscUnknown) inside_comment = truE;
#if CONFIG_KEYMAPS
      else if (csc == cscKeymap) current_keymap_kind = kkUnknown;
#endif
      goto out;
    }
    /* "else": parameter within the section-header line; check which one: */
    switch (csc)
    {case cscLanguages:
      __dealloc(config.languages); strdup_word(config.languages);
      break;
#if MIGHT_USE_TERMWINTITLE
     case cscTermwintitle:
       if (wordcount > 1) warn_section_wordcount();
       else
       { int value;
         copy_word(strbuf);
         my_atoi(strbuf, &value, NULL, 9);
         if (value > 0) config.flags |= cfTermwintitle;
       }
       break;
#endif
     case cscUserAgent:
      { char ch = *wordstart;
        if ( (ch < '0') || (ch > '3') || (wordlen > 1) )
          warn_section(cscUserAgent, _("number expected (0..3)"));
        else allow_uname_in_user_agent = (unsigned char) (ch - '0');
      }
      break;
     case cscCache:
      { int value;
        char ch;
        size_t* target;
        copy_word(strbuf); /* IMPROVEME! */
        ch = my_tolower(*strbuf);
        if ( (ch == 'r') && (strbuf[1] == ':') ) /* RAM cache size */
        { const char* temp2;
          target = &(config.ramcachesize);
#if CONFIG_DISK_CACHE
          set_cachesize:
#endif
          my_atoi(strbuf + 2, &value, &temp2, MEGA);
          ch = my_tolower(*temp2);
          if (ch== 'k') value *= KILO;
          else if (ch == 'm')
          { if (value > 100) value = 100;
            value *= MEGA;
          }
          if (value > 100 * MEGA) value = 100 * MEGA;
          *target = ((size_t) value);
        }
#if CONFIG_DISK_CACHE
        else if ( (ch == 'd') && (strbuf[1] == ':') ) /* disk cache size */
        { target = &(config.diskcachesize); goto set_cachesize; }
#endif
      }
      break;
     case cscHome:
      memory_deallocate(config.home_uri);
      if (is_word(strOff)) config.home_uri = NULL;
      else strdup_word(config.home_uri);
      break;
#if MIGHT_USE_SCROLL_BARS
     case cscScrollBars:
      if (is_word(strOff)) config.flags &= ~cfUseScrollBars;
      else if (is_word(strOn)) config.flags |= cfUseScrollBars;
      else warn_section(cscScrollBars, _(strBadValue));
      break;
#endif
     case cscRedirections:
      { int value;
        copy_word(strbuf); /* IMPROVEME! */
        my_atoi(strbuf, &value, NULL, 99);
        if (value < 3) value = 3;
        else if (value > 20) value = 20;
        config.redirections = (unsigned char) value;
      }
      break;
     case cscSearchEngine:
      __dealloc(config.search_engine); strdup_word(config.search_engine);
      break;
     case cscBookmarks:
      __dealloc(config.bookmarks); strdup_word(config.bookmarks);
      break;
#if MIGHT_USE_COLORS
     case cscColors:
      if (is_word(strOff)) config.flags |= cfColorsOff;
#if TGC_IS_CURSES
      else if (is_word(strReverse)) config.flags |= cfColorsReverse;
#endif
      break;
#endif
     case cscDontConfirm: /* IMPROVEME: use a binary search? */
      if (is_word(strQuit)) config.flags |= cfDontConfirmQuit;
      else if (is_word(strClose)) config.flags |= cfDontConfirmClose;
      else if (is_word("overwrite")) config.flags |= cfDontConfirmOverwrite;
      else if (is_word("form-submit")) config.flags |= cfDontConfirmSubmit;
      else if (is_word("form-reset")) config.flags |= cfDontConfirmReset;
      else if (is_word("form-repost")) config.flags |= cfDontConfirmRepost;
      else if (is_word("enforce-html")) config.flags |= cfDontConfirmHtml;
      else if (is_word("enable")) config.flags |= cfDontConfirmEnable;
      break;
     case cscMayWrite:
#if CONFIG_DISK_CACHE
      if (is_word(strCache)) config.flags |= cfMayWriteDiskCache;
#endif
      break;
#if OPTION_NEWS
     case cscNewsServerDefault:
      if (wordcount > 1) warn_section_wordcount();
      else
      { __dealloc(config.news_server);
        strdup_word_tolower(config.news_server);
      }
      break;
#endif
#if TGC_IS_CURSES
#if CONFIG_KEYMAPS
     case cscKeymap:
      if (wordcount > 1) warn_section_wordcount();
      else if (is_word("command")) current_keymap_kind = kkCommand;
      else if (is_word("line-input")) current_keymap_kind = kkLineInput;
      break;
#endif
#endif
#if OPTION_EXECEXT & EXECEXT_SHELL
     case cscExecextShell:
      if (wordcount == 1)
      { if (config.execext_shell != NULL)
        { warn_section(cscExecextShell, _("ignoring repeated option"));
          ignore_shell: csc = cscUnknown; goto out;
        }
        if (*wordstart != chDirsep)
        { bad_shell: warn_section(cscExecextShell, _(strBadValue));
          goto ignore_shell;
        }
        else
        { struct stat statbuf;
          mode_t mode;
          strdup_word(temp);
          /* perform some tiny plausibility test to catch worst problems... */
          if (my_stat(temp, &statbuf) != 0)
          { bad_shell_dealloc: memory_deallocate(temp); goto bad_shell; }
          mode = statbuf.st_mode;
          if ( (!S_ISREG(mode)) || (statbuf.st_size <= 0) ||
               (!(mode & (S_IXUSR | S_IXGRP | S_IXOTH))) )
            goto bad_shell_dealloc;
          config.flags |= cfExecextShellCustom;
          goto store_shellword;
        }
      }
      else if (wordcount < 15)
      { tExecextParam* param;
        strdup_word(temp);
        store_shellword:
        param = memory_allocate(sizeof(tExecextParam), mapPermanent);
        param->param = temp; param->next = config.execext_shell;
        config.execext_shell = param;
      }
      else warn_section_wordcount();
      break;
#endif
    }
  }
  else /* indented, thus in subsection ("rule") line */
  { switch (csc)
    {case cscUnknown: inside_comment = truE; break;
     case cscHttpVersion:
      if (wordcount == 0)
      { tConfigProtocolVersion* v = (tConfigProtocolVersion*)
          memory_allocate(sizeof(tConfigProtocolVersion), mapPermanent);
        strdup_word_tolower(v->hosts_pattern);
        interpret_portnumber(v->hosts_pattern, &(v->hosts_portnumber), 0,truE);
        v->next = config.http_version; config.http_version = v;
      }
      else if (wordcount == 1)
      { if ( (is_word(str1dot0))
#if CONFIG_HTTP & HTTP_11
             || (is_word(str1dot1))
#endif
           )
        { strdup_word(config.http_version->protstr); }
        else warn_section(cscHttpVersion, _(strBadValue));
      }
      else warn_section_wordcount();
      break;
     case cscHttpProxies:
#if OPTION_TLS
     case cscHttpsProxies:
#endif
      handle_proxylinepart(); break;
#if OPTION_COOKIES
     case cscHttpCookies:
#if OPTION_TLS
     case cscHttpsCookies:
#endif
      switch (wordcount)
      { case 0:
         if (is_word(strAllow)) allow_this = truE;
         else
         { if (!is_word(strDeny)) warn_section(csc, _(strAllowDenyExp));
           allow_this = falsE;
         }
         break;
        case 1:
         { tConfigCookie* cc = (tConfigCookie*)
             memory_allocate(sizeof(tConfigCookie), mapPermanent);
           strdup_word_tolower(cc->hosts_pattern);
           if (allow_this) cc->flags |= ccfAllowed;
#if OPTION_TLS
           if (csc == cscHttpsCookies)
           { cc->next = config.https_cookies; config.https_cookies = cc; }
           else
#endif
           { cc->next = config.http_cookies; config.http_cookies = cc; }
         }
         break;
        default: warn_section_wordcount(); break;
      }
      break;
#endif /* #if OPTION_COOKIES */
#if OPTION_LOCAL_CGI
     case cscLocalCgi:
      switch (wordcount)
      { case 0:
         if (is_word(strAllow)) allow_this = truE;
         else
         { if (!is_word(strDeny))
             warn_section(cscLocalCgi, _(strAllowDenyExp));
           allow_this = falsE;
         }
         break;
        case 1:
         if ( (allow_this) && (is_word(strAsterisk) || is_word("/*")) )
         { warn_section(cscLocalCgi, _("won't allow everything (\"*\") for security reasons; skipping remainder of section"));
           csc = cscUnknown;
         }
         else if ( (*wordstart != chDirsep) && (!is_word(strAsterisk)) )
         { warn_section(cscLocalCgi,
           _("path must be absolute or \"*\"; skipping remainder of section"));
           csc = cscUnknown;
         }
         else
         { tConfigLocalCgi* clc = (tConfigLocalCgi*)
             memory_allocate(sizeof(tConfigLocalCgi), mapPermanent);
           strdup_word(clc->path_pattern);
           if (allow_this) clc->flags |= clcfAllowed;
           clc->next = config.local_cgi; config.local_cgi = clc;
         }
         break;
        default: warn_section_wordcount(); break;
      }
      break;
#endif /* #if OPTION_LOCAL_CGI */
#if CONFIG_FTP
     case cscFtpLogin:
      if (wordcount == 0)
      { dealloc(ftp_login_user); strdup_word(ftp_login_user); }
      else if (wordcount == 1)
      { dealloc(ftp_login_password); strdup_word(ftp_login_password); }
      else if (wordcount == 2)
      { tConfigLogin* l = memory_allocate(sizeof(tConfigLogin), mapPermanent);
        l->user = ftp_login_user; l->password = ftp_login_password;
        ftp_login_user = ftp_login_password = NULL; /* detach */
        strdup_word_tolower(l->hosts_pattern);
        interpret_portnumber(l->hosts_pattern, &(l->hosts_portnumber),
          rp2portnumber(rpFtp), truE);
        if (last_ftp_login == NULL) config.ftp_login = l;
        else last_ftp_login->next = l;
        last_ftp_login = l;
      }
      break;
#endif
#if CONFIG_JUMPS
     case cscJumps:
      if (wordcount == 0)
      { tConfigJump* j = (tConfigJump*)
          memory_allocate(sizeof(tConfigJump), mapPermanent);
        j->next = config.jumps; config.jumps = j;
        strdup_word(j->name);
      }
      else if (wordcount == 1) strdup_word(config.jumps->uri);
      else if (wordcount < 50) /* (should be enough for all types of usage:-)*/
      { tConfigJump* j = config.jumps;
        unsigned short count = j->argcount, maxcount = j->maxargcount;
        if (count >= maxcount) /* need to allocate more space */
        { if (maxcount < 6) maxcount += 2;
          else maxcount += 5;
          j->arg = (char**) memory_reallocate(j->arg, maxcount *
            sizeof(char*), mapPermanent);
          j->maxargcount = maxcount;
        }
        strdup_word(j->arg[count]);
        j->argcount++;
      }
      else warn_section_wordcount();
      break;
#endif
#if CONFIG_LOCALDIR > 1
     case cscLocalDirSort:
      if (wordcount == 0)
      { tConfigLocaldirsort* s = (tConfigLocaldirsort*)
          memory_allocate(sizeof(tConfigLocaldirsort), mapPermanent);
        s->next = config.lds; config.lds = s;
        strdup_word(s->path_pattern);
      }
      else if (wordcount == 1)
      { strdup_word(config.lds->sorting);
        check_localdirsort(config.lds->sorting);
      }
      else warn_section_wordcount();
      break;
     case cscLocalDirFormat:
      if (wordcount == 0)
      { tConfigLocaldirformat* s = (tConfigLocaldirformat*)
          memory_allocate(sizeof(tConfigLocaldirformat), mapPermanent);
        s->next = config.ldf; config.ldf = s;
        strdup_word(s->path_pattern);
      }
      else if (wordcount == 1) strdup_word(config.ldf->format);
      else warn_section_wordcount();
      break;
#endif
#if CONFIG_KEYMAPS
     case cscKeymap:
      if (current_keymap_kind == kkUnknown)
      { warn_section(cscKeymap, _("bad keymap kind; skipping section"));
        csc = cscUnknown; inside_comment = truE;
      }
      else if (wordcount == 0)
      { __dealloc(current_keymap_keystr); strdup_word(current_keymap_keystr); }
      else if (wordcount == 1)
      { if (current_keymap_keystr != NULL)
        { const char* errstr;
          unsigned char regres; /* registration result */
          copy_word(strbuf);
          regres = ( (current_keymap_kind == kkLineInput) ?
            keymap_lineinput_key_register(current_keymap_keystr, strbuf) :
            keymap_command_key_register(current_keymap_keystr, strbuf) );
          switch (regres)
          { case 1: errstr = _("bad key identifier"); break;
            case 2: errstr = _("repeatedly defined key"); break;
            case 3: errstr = _("bad command identifier"); break;
            default: errstr = NULL;
          }
          if (errstr != NULL) warn_section(cscKeymap, errstr);
        }
      }
      else warn_section_wordcount();
      break;
#endif
#if OPTION_TLS && CONFIG_FTP
     case cscFtpsMethod:
      if (wordcount == 0)
      { tConfigFtpTlsMethod* cftm = (tConfigFtpTlsMethod*)
          memory_allocate(sizeof(tConfigFtpTlsMethod), mapPermanent);
        cftm->next = config.ftp_tls_method; config.ftp_tls_method = cftm;
        strdup_word_tolower(cftm->hosts_pattern);
        interpret_portnumber(cftm->hosts_pattern, &(cftm->hosts_portnumber), 0,
          truE);
      }
      else if (wordcount == 1)
      { tFtpTlsMethod ftm;
        if (is_word("authssl")) ftm = ftmAuthSsl;
        else if (is_word("authtls")) ftm = ftmAuthTls;
        else if (is_word("authtls-dataclear")) ftm = ftmAuthTlsDataclear;
        else if (is_word("tls")) ftm = ftmTls;
        else
        { ftm = ftmAutodetect;
          if (!is_word("autodetect"))
            warn_section(cscFtpsMethod, _(strBadValue));
        }
        config.ftp_tls_method->ftm = ftm;
      }
      else warn_section_wordcount();
      break;
#endif
     default:
       warn_section(csc,
         _("no subsections possible; skipping remainder of section"));
      csc = cscUnknown;
      inside_comment = truE;
      break;
    }
  }
  out:
  wordstart = NULL;
  wordcount++;
}

static void __init cleanup_on_newline(void)
{
#if CONFIG_FTP
  dealloc(ftp_login_user); dealloc(ftp_login_password);
#endif
#if CONFIG_KEYMAPS
  dealloc(current_keymap_keystr);
#endif
}

static void __init rtconfig_parse(const char* buf, size_t count)
{ /* prepare... */
  wordstart = NULL; wordcount = 0; csc = cscUnknown;
  inside_comment = is_line_indented = falsE; linecount = 1; ptr = buf;
  /* ...and parse */
  while (count-- > 0)
  { const char ch = *ptr++;
    if (IS_NEWLINE(ch))
    { if (!inside_comment) handle_word();
      inside_comment = is_line_indented = falsE;
      wordcount = 0; linecount++;
      cleanup_on_newline();
    }
    else if (!inside_comment)
    { if (IS_WHITESPACE(ch))
      { if ( (wordcount == 0) && (wordstart == NULL) ) is_line_indented = truE;
        handle_word();
      }
      else
      { if (wordstart == NULL)
        { if (ch == '#') inside_comment = truE;
          else wordstart = ptr - 1;
        }
      }
    }
  }
  handle_word();
  cleanup_on_newline();
}

static one_caller void __init rtconfig_handle(void)
/* handles the (built-in/external) run-time configuration */
{ static const char strRtc[] = N_("Run-time configuration: ");
  char path[1024], *spfbuf_path, *spfbuf;
  int fd;
  struct stat statbuf;
  off_t filesize;
  void* filebuf;

#if OPTION_BIRTCFG
  if (config.flags & cfRtBuiltin)
  { sprint_safe(strbuf, _("%sparsing the built-in configuration%s"), _(strRtc),
      " (<a href=\"about:birtcfg\">about:birtcfg</a>)");
    store_initmsg(strbuf);
    rtconfig_parse(strBirtcfg, strlen(strBirtcfg));
  }
#endif

  /* Map the config file into memory... */

  if (!(config.flags & cfRtExternal)) goto no_external_rtcfg;
  my_spf(path, 1024, &spfbuf_path, strPercsPercs, config.path, "config");
  fd = my_open(spfbuf_path, O_RDONLY);
  if (fd == -1) /* no (readable) config file */
  { if ( (errno == ENOENT) && (!(config.flags & cfUserDefinedConfigPath)) )
    { /* The error is "file does not exist", and the user didn't ask for a
         non-default configuration directory on the command-line; this is the
         case if new users didn't yet setup a config file, and we don't wanna
         annoy users with such an (in this case pointless) error message. */
    }
    else
    { my_spf(strbuf, STRBUF_SIZE, &spfbuf,
        _("%scan't open \"%s\" (error #%d, %s)"), _(strRtc), spfbuf_path,
        errno, my_strerror(errno));
      store_initmsg(spfbuf);
      my_spf_cleanup(strbuf, spfbuf);
    }
    goto finish;
  }
  if (my_fstat(fd, &statbuf) != 0)
  { my_spf(strbuf, STRBUF_SIZE, &spfbuf,
      _("%scan't get system information about \"%s\" (error #%d, %s)"),
      _(strRtc), spfbuf_path, errno, my_strerror(errno));
    fail_cleanup:
    store_initmsg(spfbuf);
    my_spf_cleanup(strbuf, spfbuf);
    goto cleanup_file;
  }
  if (!S_ISREG(statbuf.st_mode)) /* could be /dev/zero :-) */
  { my_spf(strbuf, STRBUF_SIZE, &spfbuf, _("%s\"%s\" isn't a regular file"),
      _(strRtc), spfbuf_path);
    goto fail_cleanup;
  }
  filesize = statbuf.st_size;
  if (filesize <= 0)
  { my_spf(strbuf, STRBUF_SIZE, &spfbuf, _("%sfile \"%s\" is empty"),
      _(strRtc), spfbuf_path);
    goto fail_cleanup;
  }
  filebuf = my_mmap(filesize, fd);
  if (filebuf == MAP_FAILED)
  { my_spf(strbuf, STRBUF_SIZE, &spfbuf,
      _("%scan't read file \"%s\" (error #%d, %s)"), _(strRtc), spfbuf_path,
      errno, my_strerror(errno));
    goto fail_cleanup;
  }
  my_madvise_sequential(filebuf, filesize);
    /* (That's probably overkill since most config files will be small...) */

  /* ...and parse it */

  my_spf(strbuf, STRBUF_SIZE, &spfbuf, _("%sreading file \"%s\""), _(strRtc),
    spfbuf_path);
  store_initmsg(spfbuf);
  my_spf_cleanup(strbuf, spfbuf);
  rtconfig_parse(filebuf, filesize);
  my_munmap(filebuf, filesize);

  cleanup_file:
  my_close(fd);
  finish:
  my_spf_cleanup(path, spfbuf_path);
  no_external_rtcfg:
  list_reverse(config.http_version, tConfigProtocolVersion);
#if CONFIG_JUMPS
  list_reverse(config.jumps, tConfigJump);
#endif
  list_reverse(config.http_proxies, tConfigProxy);
#if OPTION_TLS
  list_reverse(config.https_proxies, tConfigProxy);
#endif
#if CONFIG_LOCALDIR > 1
  list_reverse(config.lds, tConfigLocaldirsort);
  list_reverse(config.ldf, tConfigLocaldirformat);
#endif
#if OPTION_COOKIES
  list_reverse(config.http_cookies, tConfigCookie);
#endif
#if OPTION_TLS && OPTION_COOKIES
  list_reverse(config.https_cookies, tConfigCookie);
#endif
#if CONFIG_FTP && OPTION_TLS
  list_reverse(config.ftp_tls_method, tConfigFtpTlsMethod);
#endif
#if OPTION_LOCAL_CGI
  list_reverse(config.local_cgi, tConfigLocalCgi);
#endif
#if OPTION_EXECEXT & EXECEXT_SHELL
  list_reverse(config.execext_shell, tExecextParam);
#endif

  /* Setup cachepath */

#if CONFIG_DISK_CACHE
  if (config.diskcachesize > 0)
  { tBoolean usable = falsE; /* assume failure */
    char* cachepath;
    my_spf(NULL, 0, &cachepath, strPercsPercs, config.path, "cache1/");
      /* ("1" is the current cache format version number) */
    if (my_stat(cachepath, &statbuf) == 0)
    { if (S_ISDIR(statbuf.st_mode)) usable = truE;
      /* "else": someone messed up the contents of the directory ".retawq/" */
    }
    else
    { if ( (config.flags & cfMayWriteDiskCache) &&
           (mkdir(cachepath, S_IRWXU) == 0) )
        usable = truE;
    }
    if (usable)
    { config.flags |= cfUseDiskCache;
      config.cachepath = my_spf_use(cachepath);
    }
    else my_spf_cleanup(NULL, cachepath);
  }
#endif
}

#endif /* #if CONFIG_RTCONFIG */

static one_caller void __init initialize_config(void)
/* initializes the run-time configuration */
{ const char *langstr = strEn, *unamestr;
  char *temp, *temp2;
  char ch;

  /* Set some default configuration values */

  if (config.path == NULL)
  { my_spf(NULL, 0, &temp, strPercsPercs, get_homepath(), ".retawq/");
    config.path = my_spf_use(temp);
  }
  config.home_uri = my_strdup("http://retawq.sourceforge.net/");
  config.ramcachesize = 100 * KILO;
  config.redirections = 10;

#if TGC_IS_GRAPHICS
  config.width = 400; config.height = 400;
#elif TGC_IS_CURSES
  config.char_yes = str2char_(strYes); config.char_no = str2char_(strNo);
#endif
  config.char_file = str2char_(strFileUc);
  config.char_dir = str2char_(strDirectoryUc);
  ch = str2char_(strLink); config.char_link = my_toupper(ch);

#if CONFIG_CONSOLE
  config.console_backspace = 8;
#endif
#if (MIGHT_USE_SCROLL_BARS) && ( (CONFIG_TG == TG_XCURSES) || (TGC_IS_GRAPHICS) )
    config.flags |= cfUseScrollBars;
#endif

  /* Handle the run-time configuration */

#if CONFIG_RTCONFIG
  rtconfig_handle();
#endif

  /* Setup the default session path */

#if CONFIG_SESSIONS
  my_spf(NULL, 0, &temp, strPercsPercs, config.path, "session");
  config.session_default = my_spf_use(temp);
#endif

  /* Setup language stuff */

#if OPTION_I18N
  if ( (current_locale != NULL) && (lookup_language_code() >= 0) )
  { /* valid, known, usable locale... */
    langstr = current_locale;
  }
#endif
  if (config.languages == NULL) config.languages = my_strdup(langstr);

  /* Setup user_agent */

  unamestr = strEmpty; /* default */
#if HAVE_UNAME && CONFIG_RTCONFIG
  if (allow_uname_in_user_agent > 0)
  { struct utsname uts;
    if (uname(&uts) == 0)
    { const char *usys = uts.sysname, *urel = uts.release, *umac = uts.machine;

      /* check pointers */
      if ( (usys == NULL) || (*usys == '\0') ) goto no_uname;
      if ( (allow_uname_in_user_agent >= 3) &&
           ( (umac == NULL) || (*umac == '\0') ) )
        allow_uname_in_user_agent = 2;
      if ( (allow_uname_in_user_agent >= 2) &&
           ( (urel == NULL) || (*urel == '\0') ) )
        allow_uname_in_user_agent = 1;

      /* build the string */
      if (allow_uname_in_user_agent >= 3)
        my_spf(strbuf, STRBUF_SIZE, &temp, "; %s %s; %s", usys, urel, umac);
      else if (allow_uname_in_user_agent == 2)
        my_spf(strbuf, STRBUF_SIZE, &temp, "; %s %s", usys, urel);
      else /* allow_uname_in_user_agent == 1 */
        my_spf(strbuf, STRBUF_SIZE, &temp, "; %s", usys);
      unamestr = my_spf_use(temp);
      no_uname: {}
    }
  }
#endif

#if CONFIG_JAVASCRIPT
#define strJs ", js"
#else
#define strJs ""
#endif

#if CONFIG_CSS
#define strCss ", css"
#else
#define strCss ""
#endif

  my_spf(NULL, 0, &temp2, "retawq/" RETAWQ_VERSION " [%s] (%s" strJs strCss
    strTls "%s)", langstr, strTG, unamestr);
  config.user_agent = my_spf_use(temp2);
  if (unamestr != strEmpty) my_spf_cleanup(strbuf, unamestr);
}

one_caller void __init initialize(int argc, const char** argv)
/* initializes most parts of the program */
{ struct stat statbuf;
#if NEED_FD_REGISTER
  fd_register_init(); /* (must be done _very_ early) */
#endif
  if (my_fstat_unregistried(0, &statbuf) == 0)
  { fd_stdin = 0; fd_register(&fd_stdin, fdkOther);
    fd_keyboard_input = fd_stdin; __lfdmbs |= 1;
  }
  if (my_fstat_unregistried(1, &statbuf) == 0)
  { fd_stdout = 1; fd_register(&fd_stdout, fdkOther); __lfdmbs |= 2; }
  if (my_fstat_unregistried(2, &statbuf) == 0)
  { fd_stderr = 2; fd_register(&fd_stderr, fdkOther); __lfdmbs |= 4; }
#if CONFIG_DEBUG
  { int fd = my_create("debug.txt", O_CREAT | O_TRUNC | O_WRONLY,
      S_IRUSR | S_IWUSR);
    if (fd < 0) fatal_error(errno, "can't create debugging file");
    debugfd = fd;
  }
  make_fd_cloexec(debugfd);
  debugmsg("retawq " RETAWQ_VERSION " general debugging file (<http://retawq.sourceforge.net/>)\n");
  sprint_safe(strbuf, "__lfdmbs: %d\n", __lfdmbs);
  debugmsg(strbuf);
#endif

  precheck_commandline(argc, argv);

  fd_observe_init();
#if CONFIG_TG == TG_GTK
  gtk_init(&argc, &argv); /* (must be done _before_ process_commandline()) */
#endif
#if OPTION_I18N
  initialize_i18n(); /* (do this early, but _after_ gtk_init()) */
#endif

  my_memclr_var(config);
#if CONFIG_RTCONFIG
  config.flags |= cfRtAll;
#endif
  process_commandline(argc, argv);

#if TGC_IS_GRAPHICS
  if (is_environed)
  {
#if CONFIG_TG == TG_X
    xws_display = XOpenDisplay(NULL);
    if (xws_display == NULL) fatal_error(0, _(strCantOpenXws));
    need_tglib_cleanup = truE;
#endif
  }
  else
#elif CONFIG_TG == TG_XCURSES
  if (is_environed) { /* no isatty() requirements with xcurses window */ }
  else
#endif
  { if ( (is_promptable) &&
         ( (!my_isatty(fd_stdin)) || (!my_isatty(fd_stdout)) ) )
      fatal_error(0, _("need to be launched on a terminal"));
  }

#if (!CONFIG_ASYNC_DNS)
  /* show a warning once when compiled and every time when run */
#if CAN_ISSUE_WARNINGS
#warning "With the current configuration, all DNS hostname lookups will block the whole program, and it will seem to hang. Please consider setting OPTION_THREADING to 1."
#endif
  show_warning(_("with the current configuration, DNS hostname lookups will \"block\" the whole program, and it will seem to hang"));
#endif

  initialize_config();

  if (is_environed)
  {
#if (MIGHT_DO_TERMIOS) && (CONFIG_TG == TG_BICURSES)
    prepare_terminal();
#endif
#if TGC_IS_CURSES
    initialize_curses();
#elif (MIGHT_USE_COLORS) && (CONFIG_TG == TG_X)
    if ( (!(config.flags & cfColorsOff)) &&
         (XDisplayCells(xws_display, DefaultScreen(xws_display)) > 2) )
      use_colors = truE;
#elif CONFIG_TG == TG_GTK
    gdk_rgb_init();
#endif
  }
#if MIGHT_DO_TERMIOS && CONFIG_CONSOLE
  else if (program_mode == pmConsole) prepare_terminal();
#endif

#if CAN_HANDLE_SIGNALS
  initialize_signals(); /* (must be called _after_ curses init) */
#endif

  parser_initialize();
#if CONFIG_JAVASCRIPT
  javascript_initialize();
#endif
  i18n_cleanup
  resource_initialize(); /* (do this quite late - it might create threads) */
#if TGC_IS_CURSES
  if (is_environed)
  { /* (last step, to get rid of gdb's stupid thread announcements...) */
    (void) clear();
  }
#endif
}
