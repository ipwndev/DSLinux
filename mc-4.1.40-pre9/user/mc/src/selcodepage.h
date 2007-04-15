#include <config.h>

#ifdef HAVE_CHARSETS
int do_select_codepage (void);
int select_charset( int current_charset, int seldisplay );
#endif /* !HAVE_CHARSETS */
