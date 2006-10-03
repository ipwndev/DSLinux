
#ifndef MENU_H
#define MENU_H

#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#define MAXMENUS 2
#define MAXMENUSIZE 5

typedef struct _menubuttontype
{
    char *name;
    Widget *dialog;
}
menubuttontype;

typedef struct _submenutype
{
    char *name;
    int size;
    menubuttontype button[MAXMENUSIZE];
    Widget pulldown, pshell;
}
submenutype;

typedef struct _menutype
{
    int size;
    submenutype submenu[MAXMENUS];
}
menutype;

extern Widget toplevel, base, menubar, field;
extern Widget aboutbox, rulesbox, storybox;
extern Widget warpbox, quitbox, newgamebox, pausebox;
extern Widget scorebox, highscorebox;
extern Widget endgamebox, enternamebox;

menutype menu = { 2,
    {
     {"Game", 5,
      {
       {"New game", &newgamebox},
       {"Pause game", &pausebox},
       {"Warp to level...", &warpbox},
       {"View high scores", &highscorebox},
       {"Quit game", &quitbox},
       }
      },
     {"Info", 3,
      {
       {"Story of xBill", &storybox},
       {"Rules", &rulesbox},
       {"About xBill", &aboutbox},
       }
      }
     }
};

#endif
