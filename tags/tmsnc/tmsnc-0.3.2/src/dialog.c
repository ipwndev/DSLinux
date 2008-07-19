
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

#include "common.h"
#include "dialog.h"

void
UI_dialog_put_buffer(win, buffer, ptr)
     WINDOW *win;
     char *buffer;
     int ptr;
{
    int x, width, offset;

    width = DIALOG_WIDTH - 10;
    offset = (ptr / width) * width;
    x = (ptr % width) % width;

    wattron(win, A_UNDERLINE);
    mvwhline(win, 4, 4, ' ', width);
    mvwaddnstr(win, 4, 4, &buffer[offset], width);
    wattroff(win, A_UNDERLINE);
    wmove(win, 4, x + 4);
    wrefresh(win);
}

void
UI_dialog_destroy(l_win, l_pan)
     WINDOW *l_win;
     PANEL *l_pan;
{

    /** Hide window **/
    bottom_panel(l_pan);
    hide_panel(l_pan);
    update_panels();
    doupdate();

    /** Free memory **/
    del_panel(l_pan);
    delwin(l_win);
    refresh();

    /** Reset timeout **/
    timeout(20);
}

PANEL *
UI_dialog_new()
{

    /** Init **/
    WINDOW *l_win;
    PANEL *l_pan;
    int y, x;

    getmaxyx(stdscr, y, x);
    l_win =
        newwin(DIALOG_HEIGHT, DIALOG_WIDTH, (y - DIALOG_HEIGHT) / 2,
               (x - DIALOG_WIDTH) / 2);
    l_pan = new_panel(l_win);

    /** Draw **/
    top_panel(l_pan);
    show_panel(l_pan);
    box(l_win, 0, 0);
    update_panels();
    doupdate();

    /** Make getch() blocking **/
    timeout(-1);

    return l_pan;
}

int
UI_dialog_getstr(str, ret, retsize)
     char *str;
     char *ret;
     size_t retsize;
{
    WINDOW *d_win;
    PANEL *d_pan;
    int c, i, dialog_ptr = 0, str_length = 0;
    char field_buf[DIALOGBUFSIZE];

    field_buf[0] = 0x0;

    d_pan = UI_dialog_new();
    d_win = panel_window(d_pan);

    if (strlen(str) >= (DIALOG_WIDTH - 1))
        str[DIALOG_WIDTH - 2] = 0x0;

    /*
     * initial draw 
     */
    wattron(d_win, A_BOLD);
    mvwaddstr(d_win, 2, (DIALOG_WIDTH - strlen(str)) / 2, str);
    wattroff(d_win, A_BOLD);
    UI_dialog_put_buffer(d_win, field_buf, 0);

    while ((c = getch()) != '\n') {
        switch (c) {
        case KEY_LEFT:
            if (dialog_ptr > 0)
                dialog_ptr--;
            break;
        case KEY_RIGHT:
            if (dialog_ptr < str_length)
                dialog_ptr++;
            break;
        case KEY_BACKSPACE:
        case 127:
            if (dialog_ptr > 0) {
                dialog_ptr--;
                str_length--;
                for (i = dialog_ptr; i < str_length; i++)
                    field_buf[i] = field_buf[i + 1];
                field_buf[i] = 0x0;
            }
            break;
        case -1:
        case '\r':
        case '\t':
        case KEY_RESIZE:
        case KEY_UP:
        case KEY_DOWN:
            break;
        default:
            if (str_length >= DIALOGBUFSIZE - 1)
                break;

            for (i = str_length; i > dialog_ptr && i > 0; i--)
                field_buf[i] = field_buf[i - 1];

            field_buf[dialog_ptr] = c;
            field_buf[str_length + 1] = 0x0;
            dialog_ptr++;
            str_length++;
            break;
        }
        UI_dialog_put_buffer(d_win, field_buf, dialog_ptr);
        wrefresh(d_win);
    }

    UI_dialog_destroy(d_win, d_pan);

    if (str_length == 0 && dialog_ptr == 0)
        return -1;

    strncpy(ret, field_buf, retsize);
    return 0;
}

int
UI_dialog_getch(str)
     char *str;
{
    WINDOW *d_win;
    PANEL *d_pan;
    int c;

    d_pan = UI_dialog_new();
    d_win = panel_window(d_pan);

    if (strlen(str) >= (DIALOG_WIDTH - 1))
        str[DIALOG_WIDTH - 2] = 0x0;
    wattron(d_win, A_BOLD);
    mvwaddstr(d_win, (DIALOG_HEIGHT / 2), (DIALOG_WIDTH - strlen(str)) / 2,
              str);
    wattroff(d_win, A_BOLD);
    wrefresh(d_win);

    c = getch();

    UI_dialog_destroy(d_win, d_pan);

    return c;
}
