/*
 * Replacement gettext library header file. Include this within config.h,
 * like this:
 *
 *   #ifdef ENABLE_NLS
 *   # include "library/gettext.h"
 *   #else
 *   # define _(String) (String)
 *   # define N_(String) (String)
 *   #endif
 *
 * Copyright 2008 Andrew Wood, distributed under the Artistic License 2.0.
 */

#ifndef _LIBRARY_GETTEXT_H
#define _LIBRARY_GETTEXT_H 1

#ifdef HAVE_GETTEXT
# ifdef HAVE_LIBINTL_H
#  include <libintl.h>
# endif
# ifdef HAVE_LOCALE_H
#  include <locale.h>
# endif
# define _(String)	gettext (String)
# define N_(String)	gettext_noop (String)
#else
# define _(String)	minigettext (String)
# define N_(String)	minigettext_noop (String)
# define setlocale	minisetlocale
# define bindtextdomain	minibindtextdomain
# define textdomain	minitextdomain
# ifndef LC_ALL
#  define LC_ALL	""
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

char *minisetlocale(char *, char *);
char *minibindtextdomain(char *, char *);
char *minitextdomain(char *);
char *minigettext(char *);
char *minigettext_noop(char *);

#ifdef __cplusplus
}
#endif

#endif /* _LIBRARY_GETTEXT_H */

/* EOF */
