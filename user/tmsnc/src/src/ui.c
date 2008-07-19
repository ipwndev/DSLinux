
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

#include "ui.h"
#include "common.h"

void
UI_win_set_title(win, str)
     struct win *win;
     char *str;
{
    int i;

    /*
     * clear 
     */
    for (i = 1; i < win->x - 2; i++)
        mvwaddch(win->win, 1, i, ' ');

    wattron(win->win, A_BOLD | A_UNDERLINE);
    mvwaddnstr(win->win, 1, 2, str, win->x - 4);
    wattroff(win->win, A_BOLD | A_UNDERLINE);

    update_panels();
    doupdate();
}

void
UI_win_init(win)
     struct win *win;
{
    int y, x;

    getmaxyx(stdscr, y, x);

    win->y = y - 3;
    win->x = x - LIST_WIDTH;
    win->win = newwin(win->y, win->x, 1, 0);
    win->swin = derwin(win->win, win->y - 8, win->x - 4, 3, 2);
    idlok(win->swin, TRUE);
    scrollok(win->swin, TRUE);

    win->field_win = derwin(win->win, 3, win->x - 4, win->y - 4, 2);

    win->field_buf[0] = 0;
    win->field_ptr = 0;
    win->field_length = 0;

}

int
UI_win_draw(win, title)
     struct win *win;
     char *title;
{
    int y, x;

    getmaxyx(win->win, y, x);
    box(win->win, 0, 0);
    mvwhline(win->win, y - 5, 1, 0, x - 2);
    UI_win_set_title(win, title);
    wprintw(win->swin, "%s", win->buf);

    update_panels();
    doupdate();
    return 0;
}

void
UI_bar_draw(session)
     MSN_session *session;
{
    int y, x;

    getmaxyx(stdscr, y, x);

    attron(A_REVERSE);
    mvhline(y - 2, 0, ' ', x);
    mvprintw(y - 2, 1, "Friendly name: %s (%s)",
             session->me.name, MSN_status2str(session->me.status));
    attroff(A_REVERSE);
    update_panels();
    doupdate();
}

void
UI_put_msg(msg)
     char *msg;
{
    int y, x;
    char buf[512];
    getmaxyx(stdscr, y, x);
    attron(A_REVERSE);
    mvhline(y - 1, 0, ' ', x);
    snprintf(buf, sizeof(buf) - 1, "Notifications: %s", msg);
    mvaddnstr(y - 1, 1, buf, x-2);
    attroff(A_REVERSE);
    update_panels();
    doupdate();
}

void
UI_win_destroy(win)
     struct win *win;
{
    delwin(win->win);
    delwin(win->swin);
    delwin(win->field_win);
    refresh();
}

void
UI_ui_init(ui)
     struct ui *ui;
{
    getmaxyx(stdscr, ui->y, ui->x);

    UI_win_init(&ui->main);
    UI_win_draw(&ui->main, "Console");

    ui->main_panel = new_panel(ui->main.win);

    ui->list_win =
//        newwin(ui->y - (ui->y / 3) - 3, LIST_WIDTH, 1, ui->x - LIST_WIDTH);
        newwin(ui->y - CONV_HEIGHT - 3, LIST_WIDTH, 1, ui->x - LIST_WIDTH);
    ui->list_panel = new_panel(ui->list_win);

    ui->conv_win =
//        newwin((ui->y / 3), LIST_WIDTH, ui->y - (ui->y / 3) - 2, ui->x - LIST_WIDTH);
        newwin(CONV_HEIGHT, LIST_WIDTH, ui->y - CONV_HEIGHT - 2, ui->x - LIST_WIDTH);
    ui->conv_panel = new_panel(ui->conv_win);
}

void
UI_ui_destroy(ui)
     struct ui *ui;
{
    del_panel(ui->main_panel);
    del_panel(ui->list_panel);
    del_panel(ui->conv_panel);
    delwin(ui->main.swin);
    delwin(ui->main.win);
    delwin(ui->list_win);
    delwin(ui->conv_win);
    refresh();
}

int
UI_ui_draw(ui)
     struct ui *ui;
{
    box(ui->main.win, 0, 0);
    UI_win_set_title(&ui->main, "Console");
    wprintw(ui->main.swin, "%s", ui->main.buf);
    update_panels();

    top_panel(ui->list_panel);
    box(ui->list_win, 0, 0);
    wattron(ui->list_win, A_BOLD | A_UNDERLINE);
    mvwaddstr(ui->list_win, 1, 1, "Contact list");
    wattroff(ui->list_win, A_BOLD | A_UNDERLINE);
    update_panels();

    top_panel(ui->conv_panel);
    box(ui->conv_win, 0, 0);
    wattron(ui->conv_win, A_BOLD | A_UNDERLINE);
    mvwaddstr(ui->conv_win, 1, 1, "Conversations");
    wattroff(ui->conv_win, A_BOLD | A_UNDERLINE);
    update_panels();

    UI_current_conversation(ui, NULL);

    return 0;
}

void
UI_hide_panel(panel)
     PANEL *panel;
{
    bottom_panel(panel);
    hide_panel(panel);
    update_panels();
    doupdate();
}

void
UI_show_panel(panel)
     PANEL *panel;
{
    top_panel(panel);
    show_panel(panel);
    update_panels();
    doupdate();
}

void
UI_current_conversation(ui, conv)
     struct ui *ui;
     struct conv *conv;
{
    if (conv==NULL)
        replace_panel(ui->main_panel, ui->main.win);
    else
        replace_panel(ui->main_panel, conv->win.win);
    ui->c_conv = conv;
    update_panels();
    doupdate();
}

int
UI_win_addstr(struct ui *ui, struct win *win, char *fmt, ...)
{
    char buf[2048];
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
    va_end(ap);

    if (WINBUFSIZE <= (strlen(win->buf) + strlen(buf))) {
        strncpy(win->buf, &win->buf[strlen(buf)], WINBUFSIZE - 1);
        win->buf[strlen(buf)] = 0x0;
    }
    strncat(win->buf, buf, WINBUFSIZE - 1 - strlen(buf) - strlen(win->buf));
    waddstr(win->swin, buf);

    if (ui->c_conv == NULL) {
        if(win == &(ui->main))
        {
            wrefresh(ui->main.swin);
        }
    } else if (&(ui->c_conv->win) == win) {
        wrefresh(win->swin);
    }
    return 0;
}

void
UI_set_focus(ui, menu, focus)
     struct ui *ui;
     struct menu *menu;
     int focus;
{
    int i;

    switch (focus) {
    case FOCUS_MENU:
        menu->c_col = 0;
        menu->c_row = 0;
        menu->hide  = 0;
        UI_menu_draw(menu);
        break;
    case FOCUS_LIST:
        wattron(ui->list_win, A_BOLD | COLOR_PAIR(3));
        box(ui->list_win, 0, 0);
        wattroff(ui->list_win, A_BOLD | COLOR_PAIR(3));
        wrefresh(ui->list_win);
        break;
    case FOCUS_CONV:
        wattron(ui->conv_win, A_BOLD | COLOR_PAIR(3));
        box(ui->conv_win, 0, 0);
        wattroff(ui->conv_win, A_BOLD | COLOR_PAIR(3));
        wrefresh(ui->conv_win);
        break;
    case FOCUS_MAIN:
        wattron(ui->main.win, A_BOLD | COLOR_PAIR(3));
        box(ui->main.win, 0, 0);
        wattroff(ui->main.win, A_BOLD | COLOR_PAIR(3));

        for (i = 0; i < ui->num_conversations; i++) {
            wattron(ui->conv[i].win.win, A_BOLD | COLOR_PAIR(3));
            box(ui->conv[i].win.win, 0, 0);
            wattroff(ui->conv[i].win.win, A_BOLD | COLOR_PAIR(3));
        }
        update_panels();
        move(2, 0);
        doupdate();
        break;
    default:
        break;
    }
}

void
UI_remove_focus(ui, menu, focus)
     struct ui *ui;
     struct menu *menu;
     int focus;
{
    int i;

    switch (focus) {
    case FOCUS_MENU:
        UI_menu_process_cmd(menu, cmd_unfocus);
        UI_menu_draw(menu);
        break;
    case FOCUS_LIST:
        box(ui->list_win, 0, 0);
        wrefresh(ui->list_win);
        break;
    case FOCUS_CONV:
        box(ui->conv_win, 0, 0);
        wrefresh(ui->conv_win);
        break;
    case FOCUS_MAIN:
        box(ui->main.win, 0, 0);

        for (i = 0; i < ui->num_conversations; i++)
            box(ui->conv[i].win.win, 0, 0);

        update_panels();
        doupdate();
        break;
    default:
        break;
    }
}

void
UI_win_put_buffer(win, buffer, h, w, ptr)
     WINDOW *win;
     char *buffer;
     int h;
     int w;
     int ptr;
{
    int a, y, x, area, offset;

    area = h * w;
    offset = (ptr / area) * area;
    a = ptr % area;
    y = a / w;
    x = a % w;

    werase(win);
    waddnstr(win, &buffer[offset], area);
    wmove(win, y, x);
    wrefresh(win);
}
