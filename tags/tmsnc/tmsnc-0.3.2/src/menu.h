
/*
 * TMSNC - Textbased MSN Client Copyright (C) 2004 The IR Developer Group
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the IR Public Domain License as published by the IR Group;
 * either version 1.6 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the IR Public Domain License along with
 * this program; if not, write to sanoix@gmail.com.
 */

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define END       666

#define INIT_MENU_ITEM(text, action, flags)  \
{                                            \
  (unsigned char *) (text),                  \
  (action),                                  \
  (flags),                                   \
}

#define END_ITEM \
{                \
  NULL,          \
  cmd_none,      \
  END,           \
}

static struct menu_item file_menu[] = {
    INIT_MENU_ITEM("File", cmd_none, 0),
    INIT_MENU_ITEM("Manual", cmd_manual, 0),
    INIT_MENU_ITEM("Console", cmd_console, 0),
    INIT_MENU_ITEM("Quit", cmd_quit, 0),
    END_ITEM,
};

static struct menu_item action_menu[] = {
    INIT_MENU_ITEM("Actions", cmd_none, 0),
    INIT_MENU_ITEM("Change nick", cmd_changenick, 0),
    INIT_MENU_ITEM("Set personal message", cmd_setpsm, 0),
    INIT_MENU_ITEM("Invite principal", cmd_invite, 0),
    INIT_MENU_ITEM("Set talkfilter", cmd_setfilter, 0),
    INIT_MENU_ITEM("Redraw interface", cmd_redraw, 0),
    END_ITEM,
};

static struct menu_item contact_menu[] = {
    INIT_MENU_ITEM("Contacts", cmd_none, 0),
    INIT_MENU_ITEM("Add", cmd_addcontact, 0),
    INIT_MENU_ITEM("Remove", cmd_removecontact, 0),
    INIT_MENU_ITEM("Block", cmd_blockcontact, 0),
    INIT_MENU_ITEM("Un-block", cmd_unblockcontact, 0),
    INIT_MENU_ITEM("Set custom nick", cmd_setcustomnick, 0),
    INIT_MENU_ITEM("Un-set custom nick", cmd_unsetcustomnick, 0),
    END_ITEM,
};

static struct menu_item status_menu[] = {
    INIT_MENU_ITEM("Status", cmd_none, 0),
    INIT_MENU_ITEM("Online", cmd_statusonline, 0),
    INIT_MENU_ITEM("Away", cmd_statusaway, 0),
    INIT_MENU_ITEM("Idle", cmd_statusidle, 0),
    INIT_MENU_ITEM("Be right back", cmd_statusbrb, 0),
    INIT_MENU_ITEM("Busy", cmd_statusbusy, 0),
    INIT_MENU_ITEM("On phone", cmd_statusphone, 0),
    INIT_MENU_ITEM("On lunch", cmd_statuslunch, 0),
    INIT_MENU_ITEM("Hidden", cmd_statushidden, 0),
    END_ITEM,
};
