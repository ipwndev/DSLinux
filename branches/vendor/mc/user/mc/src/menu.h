#ifndef __MENU_H
#define __MENU_H

#include <signal.h>
#include "dlg.h"
#include "widget.h"

#define NO_CLOCK      0
#define HOUR_MIN_SEC  1
#define HOUR_MIN      2
/*
int get_clock_type(void);
void clock_init(void);
void clock_cancel(void);
void clock_resume(void);
void clock_interrupt_handler(int sig);
*/

void show_clock(void);

typedef void (*callfn) ();
/* FIXME: We have to leave this type ambiguous, because `callfn' is
   used both for functions that take an argument and ones that don't.
   That ought to be cleared up. */

typedef struct {
    char first_letter;
    char *text;
    int  hot_key;
    callfn call_back;
} menu_entry;

typedef struct {
    char   *name;
    int    count;
    int    max_entry_len;
    int    selected;
    menu_entry *entries;
    int    start_x;		/* position relative to menubar start */
} sMenu;
typedef sMenu *Menu;

Menu create_menu (char *name, menu_entry *entries, int count);
void destroy_menu (Menu menu);

extern int menubar_visible;

/* The button bar menu */
typedef struct {
    Widget widget;

    int    active;		/* If the menubar is in use */
    int    dropped;		/* If the menubar has dropped */
    Menu   *menu;		/* The actual menus */
    int    items;
    int    selected;		/* Selected menu on the top bar */
    int    subsel;		/* Selected entry on the submenu */
    int    max_entry_len;	/* Cache value for the columns in a box */
    int    previous_selection;	/* Selected widget before activating menu */
} WMenu;

WMenu *menubar_new (int y, int x, int cols, Menu menu [], int items);

#endif /* __MENU_H */

