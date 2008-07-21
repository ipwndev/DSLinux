#ifndef _LCT_LOCAL_H
#define _LCT_LOCAL_H

/* convenience */
#include <stdio.h>
#include <sysexits.h>

#include <config.h>

#define LOCALEDIR "/usr/share/locale"

#ifdef HAVE_LOCALE_H
# include <libintl.h>
# define _(String) gettext (String)
# ifdef gettext_noop
#  define N_(String) gettext_noop (String)
# else
#  define N_(String) (String)
# endif
#else
/* rough approximation of the functions */
# define _(String) (String)
# define N_(String) (String)
# define textdomain(Domain)
# define bindtextdomain(Package, Directory)
#endif

#define badusage(ERRMSG) do { \
  if (ERRMSG) \
    fprintf (stderr, "%s: %s\n", progname, ERRMSG); \
  usage(); \
  exit(EX_USAGE); \
} while(0)

/* setup localization for a program */
#define setuplocale() do { \
  setlocale (LC_ALL, ""); \
  bindtextdomain (PACKAGE, LOCALEDIR); \
  textdomain (PACKAGE); \
} while (0)

#define OPT(option, desc) printf("\t" option "%s\n", desc)

#define OPTIONS_ARE() printf(_("valid options are:\n"))
#define COMMANDS_ARE() printf(_("valid commands are:\n"))
#define HELPDESC _("display this help text and exit")
#define VERSIONDESC _("display version information and exit")

#endif /* _LCT_LOCAL_H */
