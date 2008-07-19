#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <config.h>

#include "tty.h"
#include "dialog.h"

#include "dlg.h"
#include "widget.h"
#include "wtools.h"

#include "i18n.h"
#include "util.h"
#include "main.h"
#include "menu.h"
#include "panel.h"

#define NEED_EDIT
#include "dlglist.h"

typedef struct _MC_DLG_List MC_DLG_List;

struct _MC_DLG_List {
  void *data;
  MC_DLG_List *next;
  MC_DLG_List *prev;
};

typedef struct {
    Dlg_head   *dlg;
    DlgTypes	dlg_type;
    char       *filename;
    char       *full_filename;
    union {
	struct {
	    int		*move_dir_p;
	    WView	*wview;
	} viewer_data;
	struct {
	    WEdit	*wedit;
	    WButtonBar	*edit_bar;
	    WMenu	*edit_menubar;
	} editor_data;
    } u;
} MC_Dialog;


/* List of (background) dialogs: filemanagers, editors, viewers */
MC_DLG_List *mc_dialogs = NULL;
/* Currently active dialog  */
MC_Dialog *current_mcdlg = NULL;
/* File manager dialog - there can be only one */
MC_Dialog *manager_mcdlg = NULL;
/* Is there any dialogs that we have to run after returning to the manager
   from another dialog */
int dialog_is_pending = 0;

int exiting_from_midnight = 0;

/* Internal functions */


extern void run_editor( Dlg_head *_edit_dlg,  WEdit *_wedit, WButtonBar *_edit_bar, WMenu *_edit_menubar );
extern void run_viewer( Dlg_head *our_dlg, int *move_dir_p, WView *wview );


///##################################################################

#define mc_list_previous(list)	((list) ? (((MC_DLG_List *)(list))->prev) : NULL)
#define mc_list_next(list)	((list) ? (((MC_DLG_List *)(list))->next) : NULL)

static MC_DLG_List* mc_list_first (MC_DLG_List *list){ while(list->prev) list=list->prev; return list;}
static MC_DLG_List* mc_list_last  (MC_DLG_List *list){ while(list->next) list=list->next; return list;}


MC_DLG_List*
mc_list_nth (MC_DLG_List *list, unsigned int n)
{
  while ((n-- > 0) && list)
    list = list->next;

  return list;
}

MC_DLG_List*
mc_list_alloc (void)
{
  MC_DLG_List *list = xmalloc (sizeof (MC_DLG_List), "MC_DLG_List");
//  MC_DLG_List *list = malloc (sizeof (MC_DLG_List));

      list->data = NULL;
      list->next = NULL;
      list->prev = NULL;
  
  return list;
}

void
mc_list_free (MC_DLG_List *list)
{
  if (list)
    {
      list->data = list->next = NULL;  
      free (list);
    }
}

MC_DLG_List*
mc_list_append (MC_DLG_List *list, void *data)
{
  MC_DLG_List *new_list;
  MC_DLG_List *last;
  
  new_list = mc_list_alloc ();
  new_list->data = data;
  
  if (list)
    {
      last = mc_list_last (list);
      last->next = new_list;
      new_list->prev = last;

      return list;
    }
  else
    return new_list;
}

MC_DLG_List*
mc_list_prepend (MC_DLG_List *list, void *data)
{
  MC_DLG_List *new_list;
  
  new_list = mc_list_alloc ();
  new_list->data = data;
  
  if (list)
    {
      if (list->prev)
	{
	  list->prev->next = new_list;
	  new_list->prev = list->prev;
	}
      list->prev = new_list;
      new_list->next = list;
    }
  
  return new_list;
}


MC_DLG_List*
mc_list_insert (MC_DLG_List *list, void *data, int position)
{
  MC_DLG_List *new_list;
  MC_DLG_List *tmp_list;
  
  if (position < 0)
    return mc_list_append (list, data);
  else if (position == 0)
    return mc_list_prepend (list, data);
  
  tmp_list = mc_list_nth (list, position);
  if (!tmp_list)
    return mc_list_append (list, data);
  
  new_list = mc_list_alloc ();
  new_list->data = data;
  
  if (tmp_list->prev)
    {
      tmp_list->prev->next = new_list;
      new_list->prev = tmp_list->prev;
    }
  new_list->next = tmp_list;
  tmp_list->prev = new_list;
  
  if (tmp_list == list)
    return new_list;
  else
    return list;
}

MC_DLG_List*
mc_list_find (MC_DLG_List *list, void *data)
{
  while (list)
    {
      if (list->data == data)
	break;
      list = list->next;
    }
  return list;
}

unsigned int
mc_list_length (MC_DLG_List *list)
{
  unsigned int length;
  
  length = 0;
  while (list)
    {
      length++;
      list = list->next;
    }
  return length;
}

int
mc_list_position (MC_DLG_List *list, MC_DLG_List *link)
{
  int i;

  i = 0;
  while (list)
    {
      if (list == link)
	return i;
      i++;
      list = list->next;
    }

  return -1;
}

MC_DLG_List*
mc_list_remove (MC_DLG_List *list, void *data)
{
  MC_DLG_List *tmp;
  
  tmp = list;
  while (tmp)
    {
      if (tmp->data != data)
	tmp = tmp->next;
      else
	{
	  if (tmp->prev)
	    tmp->prev->next = tmp->next;
	  if (tmp->next)
	    tmp->next->prev = tmp->prev;
	  
	  if (list == tmp)
	    list = list->next;
	  
	  if (tmp) {
	    if (tmp->data) free( tmp->data );
	    tmp->data = NULL;  
	  }
	  
	  break;
	}
    }
  return list;
}

void dbg_message (char *format, ...)
{
    va_list args;

    FILE *fd = fopen("/tmp/mcdebug.log", "a");
    va_start (args, format);
    vfprintf( fd, format, args );
    fprintf( fd, "\n" );
    va_end (args);
    fclose(fd);
}

///##################################################################

static int dlglist_count_dialogs()
{
    return mc_list_length( mc_dialogs );
}

static MC_DLG_List* dlglist_find_node( MC_Dialog *mcdlg )
{
    MC_DLG_List *cur_node =
	mc_list_find( mc_dialogs, (void *) mcdlg );
    if (!cur_node)
	printf( "dlglist_find_specific_node: can't find node!" );
    return cur_node;
}

static MC_DLG_List* dlglist_find_cur_node()
{
    return dlglist_find_node( current_mcdlg );
}

static int dlglist_find_node_num( MC_Dialog *mcdlg )
{
    MC_DLG_List *node;

    if (! dlglist_count_dialogs())
	return -1;

    node = dlglist_find_node( mcdlg );
    return mc_list_position( mc_dialogs, node );
}

static int dlglist_find_cur_node_num()
{
    return dlglist_find_node_num( current_mcdlg );
}

/* External functions */

/* dlglist_select_dialog() helper, but can be useful in other modules */
unsigned char get_hot_key( int n )
{
    return (n <= 9) ? '0' + n : 'a' + n - 10;
}

void dlgdbg_ensure_right_dlg_type( DlgTypes dlg_type )
{
    if (dlg_type != DLG_MANAGER && dlg_type != DLG_VIEWER &&
	dlg_type != DLG_EDITOR) {
	printf( "dlglist_ensure_right_dlg_type: wrong dlg_type = %d!",
		 dlg_type );
    }
}

/* The return value should be free'ed */
static char* dlglist_get_dialog_title( MC_Dialog *mcdlg )
{
    char buf[256];

    dlgdbg_ensure_right_dlg_type( mcdlg->dlg_type );

    switch (mcdlg->dlg_type) {
	case DLG_MANAGER:
	    return strdup( _("File Manager") );
	case DLG_VIEWER:
	    sprintf( buf, _("View: %s"), mcdlg->filename );
	    return strdup( buf );
	case DLG_EDITOR: {
	    int modified =
		mcdlg->u.editor_data.wedit
		? mcdlg->u.editor_data.wedit->modified
		: 0;

	    sprintf( buf, _("Edit: %s%s"), mcdlg->filename, modified ? " (*)" : "" );
	    return strdup( buf );
	}
    }
    return NULL;
}


static void dlglist_set_dialog_xterm_title( MC_Dialog *mcdlg )
{
    extern int xterm_flag;
    extern int xterm_title;
    
    if(xterm_flag && xterm_title) 
    {
	char buf[256];

	dlgdbg_ensure_right_dlg_type( mcdlg->dlg_type );

	switch (mcdlg->dlg_type) {
	    case DLG_MANAGER:
		sprintf( buf, "MC: %s", name_trunc (mcdlg->full_filename, 200) );
//		update_xterm_title (strdup(buf));
		update_xterm_title (buf);
		return;
	    case DLG_VIEWER:
		sprintf( buf, "MC view: %s", name_trunc (mcdlg->full_filename, 200) );
		update_xterm_title (buf);
//		update_xterm_title (strdup(buf));
		return;
	    case DLG_EDITOR: {
		int modified =
		    mcdlg->u.editor_data.wedit
		    ? mcdlg->u.editor_data.wedit->modified
		    : 0;

		sprintf( buf, "MC edit: %s%s", name_trunc (mcdlg->full_filename, 200), modified ? " (*)" : "" );
		update_xterm_title (buf);
//		update_xterm_title (strdup(buf));
		return;
	    }
	}
    }
    return;
}


#ifdef MC_DEBUG
void dlgdbg_dump_dialogs()
{
    int dlgcount = dlglist_count_dialogs();
    char *dlgtitle =
	dlgcount
	? dlglist_get_dialog_title( current_mcdlg )
	: NULL;

    dbg_message( "dlgdbg_dump_dialogs: %d dialogs; current is: %s",
		dlgcount, dlgtitle );

    free( dlgtitle );
}
#else
#define dlgdbg_dump_dialogs()
#endif

void dlglist_add_dialog(
    Dlg_head *dlg, DlgTypes dlg_type,
    const char *filename )
{
    MC_Dialog *mcdlg;
    int current_dlg_num = dlglist_find_cur_node_num();

    dlgdbg_ensure_right_dlg_type( dlg_type );

    mcdlg = (MC_Dialog*) malloc( sizeof(MC_Dialog) );
    memset (mcdlg, 0, sizeof (MC_Dialog));

    mcdlg->dlg		= dlg;
    mcdlg->dlg_type	= dlg_type;
    mcdlg->filename	= filename ? strdup( filename ) : NULL;
    mcdlg->full_filename =
	filename ? concat_dir_and_file( cpanel->cwd, filename ) : NULL;

#ifdef MC_DEBUG
    {
	char *dlgtitle = dlglist_get_dialog_title( mcdlg );
	dbg_message( "dlglist_add_dialog: %s (%s)", dlgtitle, mcdlg->full_filename );
	free( dlgtitle );
    }
#endif

    current_mcdlg = mcdlg;
    if (dlg_type == DLG_MANAGER)
	manager_mcdlg = mcdlg;
    mc_dialogs = mc_list_insert( mc_dialogs, mcdlg, current_dlg_num + 1 );
    dlglist_set_dialog_xterm_title( mcdlg );
//    update_xterm_title (dlglist_get_dialog_title( mcdlg ));
}

void dlglist_add_viewer(
    Dlg_head *dlg, const char *filename,
    int *move_dir_p, WView *wview )
{
    dlglist_add_dialog( dlg, DLG_VIEWER, filename );
    current_mcdlg->u.viewer_data.move_dir_p = move_dir_p;
    current_mcdlg->u.viewer_data.wview = wview;
}

void dlglist_add_editor(
    Dlg_head *edit_dlg, const char *filename,
    WEdit *wedit, WButtonBar *edit_bar, WMenu *edit_menubar )
{
    dlglist_add_dialog( edit_dlg, DLG_EDITOR, filename );
    current_mcdlg->u.editor_data.wedit = wedit;
    current_mcdlg->u.editor_data.edit_bar = edit_bar;
    current_mcdlg->u.editor_data.edit_menubar = edit_menubar;
}

void dlglist_dialog_switch( Direction direction, int dlgnum )
{
    MC_Dialog *next_mcdlg, *cur_mcdlg;
    MC_DLG_List *next_node;
    MC_DLG_List *cur_node = dlglist_find_cur_node();

    if (direction == DIR_NEXT) {
	next_node = mc_list_next( cur_node );
	if (!next_node)
	    next_node = mc_list_first( mc_dialogs );
    } else if (direction == DIR_PREV) {
	next_node = mc_list_previous( cur_node );
	if (!next_node)
	    next_node = mc_list_last( mc_dialogs );
    } else {
	next_node = mc_list_nth( mc_dialogs, dlgnum );
	if (!next_node)
	    return;
    }

    if (next_node == cur_node)
	return;

    cur_mcdlg = (MC_Dialog*) cur_node->data;
    next_mcdlg = (MC_Dialog*) next_node->data;

    dlglist_set_dialog_xterm_title( next_mcdlg );
//    update_xterm_title (dlglist_get_dialog_title( next_mcdlg ));

#ifdef MC_DEBUG
    {
	char *dlgtitle1 = dlglist_get_dialog_title( cur_mcdlg );
	char *dlgtitle2 = dlglist_get_dialog_title( next_mcdlg );
	dbg_message( "dlglist_dialog_switch: from '%s' to '%s'",
		   dlgtitle1, dlgtitle2 );
	free( dlgtitle1 );
	free( dlgtitle2 );
    }
#endif

    current_mcdlg = next_mcdlg;

    if (cur_mcdlg->dlg_type != DLG_MANAGER) {
	/* "soft" stop of current dialog */
	cur_mcdlg->dlg->running = 0;
	cur_mcdlg->dlg->soft_exit = 1;
	if (next_mcdlg->dlg_type != DLG_MANAGER) {
	    /* mark next dialog as "pending" */
	    dialog_is_pending = 1;
	} else
	    do_refresh();

	/* now we will return back to manager */
    } else {
	/* Current dialog is DLG_MANAGER */
	dialog_is_pending = 1;
	dlglist_process_pending_dialogs();
    }
}

void dlglist_process_pending_dialogs()
{
#ifdef MC_DEBUG
    dbg_message( "dlglist_process_pending_dialogs: %d", dialog_is_pending );
#endif

    while (dialog_is_pending) {
	dialog_is_pending = 0;

	current_mcdlg->dlg->soft_exit = 0;

	if (current_mcdlg->dlg_type == DLG_VIEWER)
	{
	    run_viewer( current_mcdlg->dlg,
		      current_mcdlg->u.viewer_data.move_dir_p,
		      current_mcdlg->u.viewer_data.wview );
	}
	  else if (current_mcdlg->dlg_type == DLG_EDITOR)
	{
	    run_editor( current_mcdlg->dlg,
		      current_mcdlg->u.editor_data.wedit,
		      current_mcdlg->u.editor_data.edit_bar,
		      current_mcdlg->u.editor_data.edit_menubar );
	} else {
	    dbg_message( "dlglist_process_pending_dialog: "
			 "DLG_MANAGER can't be 'pending' dialog!" );
	}
    }

    do_refresh();

#ifdef MC_DEBUG
    dbg_message( "dlglist_process_pending_dialogs end" );
#endif
}


void dlglist_remove_current_dialog()
{
    MC_DLG_List *cur_node = dlglist_find_cur_node();
    MC_Dialog *mcdlg = cur_node->data;

#ifdef MC_DEBUG
    {
	char *dlgtitle = dlglist_get_dialog_title( mcdlg );
	dbg_message( "dlglist_remove_current_dialog: %s", dlgtitle );
	free( dlgtitle );
    }
#endif

    if ( mcdlg->dlg_type == DLG_MANAGER && dlglist_count_dialogs() > 1 ) {
	dlgdbg_dump_dialogs();
	dbg_message( "dlglist_remove_current_dialog: "
		     "attempt to remove DLG_MANAGER "
		     "while other dialogs are active!" );
    }

    if (mcdlg->filename) free( mcdlg->filename );
    if (mcdlg->full_filename) free( mcdlg->full_filename );

    mc_dialogs = mc_list_remove( mc_dialogs, mcdlg );

    current_mcdlg = manager_mcdlg;

    dlgdbg_dump_dialogs();
}


void dlglist_select_dialog()
{
    Listbox *listbox;
    int i;

    /* Find the longest dialog title */
    int max_title_len = 15;
    MC_DLG_List *cur_node;

    if (exiting_from_midnight) return;

#ifdef MC_DEBUG
    dbg_message( "dlglist_select_dialog: start" );
#endif
    
    cur_node = mc_list_first( mc_dialogs );
    while (cur_node) {
	char *dlgtitle;
	int len;
	MC_Dialog *cur_mcdlg = (MC_Dialog*) cur_node->data;
	dlgtitle = dlglist_get_dialog_title( cur_mcdlg );
	len = strlen(dlgtitle);
	free( dlgtitle );

	max_title_len = MAX( max_title_len, len );

	cur_node = mc_list_next( cur_node );
    }
    
    /* Create listbox */

    listbox = create_listbox_window(
	max_title_len + 6, dlglist_count_dialogs(),
	_(" Select Screen "), "[Screens List]");

    /* Fill it with dialog titles */

    cur_node = mc_list_first( mc_dialogs );
    for (i = 0; cur_node; i++) {
	char *dlgtitle, buffer[255];
	MC_Dialog *cur_mcdlg = (MC_Dialog*) cur_node->data;
	dlgtitle = dlglist_get_dialog_title( cur_mcdlg );
	sprintf( buffer, "%s", dlgtitle );
	free( dlgtitle );

	LISTBOX_APPEND_TEXT( listbox, get_hot_key(i), buffer, NULL );

	cur_node = mc_list_next( cur_node );
    }

    /* Run listbox */

    i = dlglist_find_cur_node_num();
    listbox_select_by_number( listbox->list, i );

    i = run_listbox( listbox );
    if (i < 0)
	return;

    /* Switch to selected dialog */
    
    dlglist_dialog_switch( DIR_NTH, i );

#ifdef MC_DEBUG
    dbg_message( "dlglist_select_dialog: finish" );
#endif
}

void dlglist_switch_to_next_dlg()
{
    if (exiting_from_midnight) return;
#ifdef MC_DEBUG
    dbg_message( "dlglist_switch_to_next_dlg" );
#endif
    dlglist_dialog_switch( DIR_NEXT, 0 );
}

void dlglist_switch_to_prev_dlg()
{
    if (exiting_from_midnight) return;
#ifdef MC_DEBUG
    dbg_message( "dlglist_switch_to_prev_dlg" );
#endif
    dlglist_dialog_switch( DIR_PREV, 0 );
}

/* Return -1 if dialog is not found */
int dlglist_get_dlg_num_if_it_is_active(
    DlgTypes dlg_type, const char *filename )
{
    char *full_filename;
    MC_DLG_List *cur_node;
    int result = -1;

    if (dlg_type != DLG_VIEWER && dlg_type != DLG_EDITOR)
	return -1;

    full_filename = concat_dir_and_file( cpanel->cwd, filename );

    cur_node = mc_list_first( mc_dialogs );
    while (cur_node) {
	MC_Dialog *cur_mcdlg = (MC_Dialog*) cur_node->data;
	if (cur_mcdlg->dlg_type == dlg_type &&
	    strcmp( cur_mcdlg->full_filename, full_filename ) == 0)
	{
	    result = dlglist_find_node_num( cur_mcdlg );
	    goto fin;
	}

	cur_node = mc_list_next( cur_node );
    }
fin:
    free( full_filename );
    return result;
}

void dlglist_before_exit_from_manager()
{
    MC_DLG_List *cur_node;

#ifdef MC_DEBUG
    dbg_message( "dlglist_before_exit_from_manager: start" );
#endif

    exiting_from_midnight = 1;

    dlgdbg_dump_dialogs();

    cur_node = mc_list_first( mc_dialogs );
    while (cur_node) {
	MC_Dialog *cur_mcdlg = (MC_Dialog*) cur_node->data;
	MC_DLG_List *next_node = mc_list_next( cur_node );

	
	if (cur_mcdlg->dlg_type == DLG_VIEWER) {

	    /* Kill any viewers */

	    current_mcdlg = cur_mcdlg;

#ifdef MC_DEBUG
	    {
		char *dlgtitle = dlglist_get_dialog_title( cur_mcdlg );
		dbg_message( "dlglist_before_exit_from_manager: killing '%s'",
			     dlgtitle );
		free( dlgtitle );
	    }
#endif

	    finish_view( current_mcdlg->dlg,
			 current_mcdlg->u.viewer_data.move_dir_p,
			 current_mcdlg->u.viewer_data.wview );

	} else if (cur_mcdlg->dlg_type == DLG_EDITOR) {

	    current_mcdlg = cur_mcdlg;

	    if (! current_mcdlg->u.editor_data.wedit->modified) {
		/* Kill unmodified editors */
#ifdef MC_DEBUG
	    {
		char *dlgtitle = dlglist_get_dialog_title( cur_mcdlg );
		dbg_message( "dlglist_before_exit_from_manager: killing '%s'",
			     dlgtitle );
		free( dlgtitle );
	    }
#endif
		finish_edit(
		    current_mcdlg->dlg,
		    current_mcdlg->u.editor_data.wedit,
		    current_mcdlg->u.editor_data.edit_bar,
		    current_mcdlg->u.editor_data.edit_menubar );
	    } else {
		/* Activate modified editor, so user can save file */
#ifdef MC_DEBUG
		{
		    char *dlgtitle = dlglist_get_dialog_title( cur_mcdlg );
		    dbg_message( "dlglist_before_exit_from_manager: "
			         "activating unsaved '%s'",
			         dlgtitle );
		    free( dlgtitle );
		}
#endif

		beep();
		dialog_is_pending = 1;
		dlglist_process_pending_dialogs();
	    }

	}

	cur_node = next_node;
    }

    current_mcdlg = manager_mcdlg;

#ifdef MC_DEBUG
    dbg_message( "dlglist_before_exit_from_manager: finish" );
#endif
}
