#include <config.h>

#ifdef HAVE_CHARSETS

#include <stdio.h>
#include <stdlib.h>
#include "dlg.h"
#include "dialog.h"
#include "widget.h"
#include "wtools.h"
#include "charsets.h"

#define ENTRY_LEN 35

unsigned char get_hotkey( int n )
{
    return (n <= 9) ? '0' + n : 'a' + n - 10;
}

int select_charset( int current_charset, int seldisplay )
{
    int i, menu_lines = n_codepages + 1;
 
    /* Create listbox */
    Listbox* listbox =
	create_listbox_window ( ENTRY_LEN + 2, menu_lines,
				" Choose codepage ",
				"[Codepages Translation]");

    if (!seldisplay)
	LISTBOX_APPEND_TEXT( listbox, '-', "< No translation >", NULL );

    /* insert all the items found */
    for (i = 0; i < n_codepages; i++) {
	struct codepage_desc cpdesc = codepages[i];
	LISTBOX_APPEND_TEXT( listbox, get_hotkey(i), cpdesc.name, NULL );
    }


    if (seldisplay)
	LISTBOX_APPEND_TEXT( listbox, get_hotkey(n_codepages), "Other 8 bit", NULL );
	
    /* Select the default entry */

    i = (seldisplay) ? ( (current_charset < 0) ? n_codepages : current_charset ) : ( current_charset + 1 );

    listbox_select_by_number( listbox->list, i );

    i = run_listbox( listbox );

    return (seldisplay) ? ( (i >= n_codepages) ? -1 : i ) : ( i - 1 );
}

int
do_select_codepage (void)
{
    char *errmsg;
    extern int source_codepage;
    extern int display_codepage;
    if (display_codepage > 0) {
	source_codepage = select_charset (source_codepage, 0);
	errmsg =
	    init_translation_table (source_codepage, display_codepage);
	if (errmsg) {
	    message (1, "ERROR", "%s", errmsg);
	    return -1;
	}
    } else {
	message (1, _("Warning"),
		 _("To use this feature select your codepage in\n"
		   "Setup / Display Bits dialog!\n"
		   "Do not forget to save options."));
	return -1;
    }
    return 0;
}

#endif /* !HAVE_CHARSETS */
