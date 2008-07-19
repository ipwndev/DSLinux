
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
 * this program; if not, write to sanoix@gmail.com
 */

#include "config.h"
#if defined(USE_NCURSES) && !defined(RENAMED_NCURSES)
#include <ncurses.h>
#else
#include <curses.h>
#endif
#include <form.h>
#include <panel.h>
#include <pthread.h>

#include "core.h"

#define FOCUS_MENU    1
#define FOCUS_LIST    2
#define FOCUS_CONV    3
#define FOCUS_MAIN    4

#define SOUND_LOGIN   1
#define SOUND_NEWMSG  2
#define SOUND_ERROR   3

typedef struct _config {
    short autologin;
    char initial_status[10];
    char codeset[20];
    char homedir[120];
    char away_message[80];
    char pass[30];
    int filter;
    int log;
    int version;
    int cursor_follow_marker;
    int display_csong;
    struct _color {
        int c1, c2, c3, c4, c5, c6;
    } color;
    struct _sound {
        char player[20];
        char login[20];
        char newmsg[20];
        char error[20];
    } sound;
    int use_nickname;
    int use_mouse;
} config;

enum engine_cmd {
    cmd_noop,
    cmd_left,
    cmd_right,
    cmd_up,
    cmd_down,
    cmd_choose,
    cmd_unfocus,
};

enum menu_commands {
    cmd_none,
    cmd_login,
    cmd_console,
    cmd_quit,
    cmd_changenick,
    cmd_invite,
    cmd_setfilter,
    cmd_setpsm,
    cmd_redraw,
    cmd_addcontact,
    cmd_removecontact,
    cmd_blockcontact,
    cmd_unblockcontact,
    cmd_setcustomnick,
    cmd_unsetcustomnick,
    cmd_manual,
    cmd_statusonline,
    cmd_statusaway,
    cmd_statusidle,
    cmd_statusbusy,
    cmd_statusbrb,
    cmd_statusphone,
    cmd_statuslunch,
    cmd_statushidden,
};

struct menu_item {
    unsigned char *text;
    enum menu_commands action;
    int flags;
};

#define MENU_NO 4

struct menu {
    struct menu_item *menu[MENU_NO];
    PANEL *panel[MENU_NO];
    WINDOW *window[MENU_NO];
    int c_col;
    int c_row;
    int y;
    int x;
    int colw;
    int titlew;
    int hide;
};

#define WINBUFSIZE   2048
#define FIELDBUFSIZE 512

struct win {
    WINDOW *win;
    WINDOW *swin;
    char buf[WINBUFSIZE];
    WINDOW *field_win;
    char field_buf[FIELDBUFSIZE];
    int field_ptr;
    int field_length;
    int y;
    int x;
};

#define MAX_CONV 30
#define CONTACT_LEN 64

struct conv {
    int sd;
    FILE *logfd;
    char **ppl;
    int num_ppl;
    int new_message;
    char last_contact[CONTACT_LEN];
    int csc;
    struct win win;
};

struct ui {
    WINDOW *conv_win;
    PANEL *conv_panel;

    WINDOW *list_win;
    PANEL *list_panel;

    PANEL *main_panel;

    struct win main;
    struct conv *c_conv;
    struct conv conv[MAX_CONV];
    int num_conversations;
    int y;
    int x;
};

#define LIST_WIDTH    30
#define CONV_HEIGHT	7	//gfhuang

void UI_init_ncurses();
void UI_menu_init(struct menu *);
int UI_menu_draw(struct menu *);
void UI_menu_destroy(struct menu *);
int UI_menu_process_cmd(struct menu *, enum engine_cmd);
int UI_menu_handle_key(struct menu *, int);
int UI_menu_cntlines(struct menu_item []);
void UI_ui_init(struct ui *);
int UI_ui_draw(struct ui *);
void UI_ui_destroy(struct ui *);
void UI_usage(void);
void UI_play_sound(int);
int UI_err_exit(char *fmt, ...);
int UI_check_for_updates(void);
int UI_is_resized(void);
int UI_is_killed(void);
void UI_setup_signals(void);
int UI_loadconfig(char *, MSN_session *);
int UI_createconfig(char *);
int UI_login(MSN_session *, char *, int);
void UI_show_panel(PANEL *);
void UI_hide_panel(PANEL *);
void UI_current_conversation(struct ui *, struct conv *);
int UI_win_addstr(struct ui *, struct win *, char *, ...);
char *UI_get_timestr(char *, size_t);
void UI_set_focus(struct ui *, struct menu *, int);
void UI_remove_focus(struct ui *, struct menu *, int);
int UI_list_handle_key(MSN_session *, struct ui *, int);
void UI_bar_draw(MSN_session *);
void UI_put_msg(char *);
void UI_sort_list(MSN_session *);
void UI_sort_conv(struct ui *);
void UI_erase_listwin(WINDOW *);
int UI_conv_handle_key(int);
void UI_draw_list(struct ui *, MSN_session *);
void UI_draw_conv(struct ui *);
void UI_win_init(struct win *);
int UI_win_draw(struct win *, char *);
int iconv_init(char *);
void UI_conv_nullify(struct ui *);
void UI_win_destroy(struct win *);
int UI_do_command(MSN_session *, struct ui *, int, char *, size_t);
int UI_start_conversation(struct ui *, MSN_session *, char *, int);
int UI_main_handle_key(struct ui *, MSN_session *, int);
int UI_handle_server_code(struct ui *, MSN_session *, int, char *);
int UI_handle_conversation_code(struct ui *, MSN_session *,
                                struct conv *, int, char *);
int UI_handle_mouse(struct ui *, struct menu *, MSN_session *, int *);
void iconv_destroy();
int UI_loadaliases(char *, MSN_session *);
int UI_dumpaliases(char *, MSN_session *);
char *UI_get_contact_name(MSN_contact *, int);
int UI_dialog_getstr(char *, char *, size_t);
int UI_dialog_getch(char *);
int UI_set_alias(MSN_contact *, char *);
int UI_delete_alias(MSN_contact *);
int UI_get_list_pointer();
int UI_handle_hotkey(struct ui *, MSN_session *, config *);
void UI_close_conversation(struct ui *, struct conv *);
int UI_set_filter(char *);
void UI_filter_translate(char *, int);
void UI_manual_resize();
void UI_win_put_buffer(WINDOW *, char *, int, int, int);
int UI_update_current_song(MSN_session *);
config *UI_get_config(void);

//Start of iveqy
int UI_handle_power_functions(struct ui *, MSN_session *);
int UI_handle_altkey(struct ui *, struct menu *, int *);
//End of iveqy
