
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

#include "login.h"

void
callback(i, str)
     int i;
     char *str;
{
    werase(statuswin);
    if (i != -1) {
        wattron(statuswin, A_REVERSE | COLOR_PAIR(4));
        mvwaddnstr(statuswin, 1, 3,
                   "                                                     ",
                   i * 5);
        wattroff(statuswin, A_REVERSE | COLOR_PAIR(4));
    } else
        mvwaddstr(statuswin, 1, 31, "ERROR");

    mvwaddch(statuswin, 1, 2, '[');
    mvwaddch(statuswin, 1, 65, ']');
    mvwaddstr(statuswin, 3, (70 - (strlen(str))) / 2, str);
    wrefresh(statuswin);
}

void
draw_border(win)
     WINDOW *win;
{
    int y, x;
    char buf[128];

    snprintf(buf, sizeof(buf) - 1, "T M S N C - Version %s", VERSION);
    getmaxyx(win, y, x);
    wattron(win, A_REVERSE | COLOR_PAIR(1));
    mvwhline(win, 0, 0, ' ', x);
    mvwhline(win, y - 1, 0, ' ', x);
    mvwvline(win, 0, 0, ' ', y);
    mvwvline(win, 0, x - 1, ' ', y);
    mvwaddstr(win, 0, (x - strlen(buf)) / 2, buf);
    wattroff(win, A_REVERSE | COLOR_PAIR(1));
    wrefresh(win);
}

void
setup_fields(wins, addr, pass)
     struct login_windows *wins;
     char *addr;
     char *pass;
{
    wins->field[0] = new_field(1, 46, 2, 2, 4, 0);
    wins->field[1] = new_field(1, 46, 4, 2, 4, 0);
    wins->field[2] = NULL;

    /*
     * Set field options 
     */
    field_opts_off(wins->field[0], O_AUTOSKIP);
    field_opts_off(wins->field[0], O_BLANK);
    set_field_back(wins->field[0], A_UNDERLINE);
    field_opts_off(wins->field[1], O_PUBLIC);
    field_opts_off(wins->field[1], O_AUTOSKIP);
    field_opts_off(wins->field[1], O_BLANK);
    set_field_back(wins->field[1], A_UNDERLINE);
    set_field_buffer(wins->field[0], 0, addr);
    set_field_buffer(wins->field[1], 0, pass);
}

void
destroy_fields(wins)
     struct login_windows *wins;
{
    free_field(wins->field[0]);
    free_field(wins->field[1]);
}

void
draw_login(wins, max_y, max_x)
     struct login_windows *wins;
     int max_y;
     int max_x;
{
    wins->main = newwin(11, 70, (max_y - 11) / 2, (max_x - 70) / 2);
    wins->main_panel = new_panel(wins->main);
    wins->formwin = derwin(wins->main, 5, 48, 1, 20);
    statuswin = derwin(wins->main, 4, 68, 6, 1);
    top_panel(wins->main_panel);
    refresh();
    doupdate();

    /*
     * Create the form and post it 
     */
    wins->form = new_form(wins->field);
    set_form_win(wins->form, wins->formwin);
    set_form_sub(wins->form, derwin(wins->formwin, 5, 48, 0, 0));
    post_form(wins->form);
    wrefresh(wins->formwin);

    wattron(wins->main, A_BOLD);
    mvwaddstr(wins->main, 3, 2, "Username:");
    mvwaddstr(wins->main, 5, 2, "Password:");
    wattroff(wins->main, A_BOLD);
    draw_border(wins->main);
    wrefresh(wins->main);

    refresh();
}

void
destroy_login(wins)
     struct login_windows *wins;
{
    del_panel(wins->main_panel);
    delwin(wins->main);
    unpost_form(wins->form);
    free_form(wins->form);
    delwin(wins->formwin);
}

int
handle_form(ch, form)
     int ch;
     FORM *form;
{
    switch (ch) {
    case KEY_DOWN:
    case '\t':                 /* Tab */
        form_driver(form, REQ_NEXT_FIELD);
        form_driver(form, REQ_END_LINE);
        break;
    case KEY_UP:
        form_driver(form, REQ_PREV_FIELD);
        form_driver(form, REQ_END_LINE);
        break;
    case KEY_LEFT:
        form_driver(form, REQ_PREV_CHAR);
        break;
    case KEY_RIGHT:
        form_driver(form, REQ_NEXT_CHAR);
        break;
    case KEY_BACKSPACE:
    case 127:                  /* Backspace in terminal.app */
        form_driver(form, REQ_PREV_CHAR);
        //Simulate backspace
        form_driver(form, REQ_DEL_CHAR);
        break;
    case '\n':
        if (field_index(current_field(form)) == 0) {
            form_driver(form, REQ_NEXT_FIELD);
            form_driver(form, REQ_END_LINE);
        } else {
            form_driver(form, REQ_LAST_FIELD);
            //Submit form
            return 1;
        }
        break;
    case -1:
    case KEY_RESIZE:
        break;
    default:
        form_driver(form, ch);
        break;
    }
    return 0;
}

int
UI_login(session, dispatch, port)
     MSN_session *session;
     char *dispatch;
     int port;
{
    int y, x;
    struct login_windows wins;
    char pass[80], *ptr;
    config *cf = UI_get_config();

    getmaxyx(stdscr, y, x);
    setup_fields(&wins, session->me.addr, cf->pass);
    draw_login(&wins, y, x);

    if (session->me.addr[0] != 0x0)
        set_current_field(wins.form, wins.field[1]);
    else
        set_current_field(wins.form, wins.field[0]);

    while (session->sd < 1) {
        if (cf->autologin == 0)
            while (handle_form(getch(), wins.form) == 0) {
                if (UI_is_killed()) {
                    destroy_login(&wins);
                    destroy_fields(&wins);
                    endwin();
                    exit(0);
                }
                if (UI_is_resized()) {
                    destroy_login(&wins);
                    endwin();
                    initscr();
                    erase();
                    refresh();
                    getmaxyx(stdscr, y, x);
                    draw_login(&wins, y, x);
                }
                wrefresh(wins.formwin);
            }

        strncpy(session->me.addr,
                field_buffer(wins.field[0], 0), sizeof(session->me.addr) - 1);

        strncpy(pass, field_buffer(wins.field[1], 0), sizeof(pass) - 1);

        if ((ptr = strchr(session->me.addr, ' ')) != NULL)
            *ptr = 0x0;
        if ((ptr = strchr(pass, ' ')) != NULL)
            *ptr = 0x0;

        /*
         * session->sd gets updated after running this function 
         */
        MSN_init_session(dispatch, port, session, pass, &callback);

        /*
         * zero the password field 
         */
        set_field_buffer(wins.field[1], 0, "\0");
    }

    //eggs, by gfhuang
    strncpy(session->me.psm, "Textbased MSN Client: http://tmsnc.sourceforge.net/", PSM_LEN-1);
    session->me.cmedia[0] = 0x0;
    MSN_send_uux(session);

    if (MSN_load_userlist(session, callback) < 0)
        UI_err_exit("Couldn't retrieve the list of users");

    destroy_login(&wins);
    destroy_fields(&wins);
    clear();
    return 0;
}
