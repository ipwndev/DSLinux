#ifndef MC_DLGLIST_H
#define MC_DLGLIST_H

#include <sys/stat.h>

#ifndef	WANT_WIDGETS
#define WANT_WIDGETS 1
#endif

#include "view.h"

#ifdef NEED_EDIT
#define MIDNIGHT 1
#include "../edit/edit-widget.h"
#include "../edit/edit.h"
#include "menu.h"
#endif

typedef enum {
    DLG_MANAGER, DLG_VIEWER, DLG_EDITOR
} DlgTypes;

typedef enum {
    DIR_PREV, DIR_NEXT, DIR_NTH
} Direction;

void dbg_message( char *format, ... );

void dlglist_add_viewer(
    Dlg_head *dlg, const char *filename, int *move_dir_p, WView *wview );

#ifdef NEED_EDIT
void dlglist_add_editor( Dlg_head *edit_dlg, const char *filename, WEdit *wedit, WButtonBar *edit_bar, WMenu *edit_menubar );
#endif

void dlglist_add_dialog( Dlg_head *dlg, DlgTypes dlg_type, const char *filename );
void dlglist_dialog_switch( Direction direction, int dlgnum );
void dlglist_switch_to_prev_dlg();
void dlglist_switch_to_next_dlg();
void dlglist_select_dialog();
void dlglist_remove_current_dialog();
void dlglist_before_exit_from_manager();
void dlglist_process_pending_dialogs();
int dlglist_get_dlg_num_if_it_is_active(
    DlgTypes dlg_type, const char *filename );

unsigned char get_hot_key( int n );

#endif /* MC_DLGLIST_H */
