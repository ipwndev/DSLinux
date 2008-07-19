
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
#include "menu.h"

int
UI_menu_cntlines(menu_items)
     struct menu_item menu_items[];
{
    int i;

    for (i = 0; menu_items[i].flags != END; i++);
    return i;
}

void
UI_menu_init(menu)
     struct menu *menu;
{
    int i, offset;

    getmaxyx(stdscr, menu->y, menu->x);

    menu->colw = 28;
    menu->titlew = 14;
    menu->c_col = 0;
    menu->c_row = 0;
    menu->hide = 1;

    menu->menu[0] = file_menu;
    menu->menu[1] = action_menu;
    menu->menu[2] = contact_menu;
    menu->menu[3] = status_menu;

    for (i = 0, offset = 2; i < MENU_NO; i++, offset += menu->titlew) {
        menu->window[i] =
            newwin(UI_menu_cntlines(menu->menu[i]) + 1, menu->colw, 1, offset);
        wbkgd(menu->window[i], A_REVERSE);
        box(menu->window[i], 0, 0);
        menu->panel[i] = new_panel(menu->window[i]);
        hide_panel(menu->panel[i]);
    }

    update_panels();
    doupdate();
}

void
UI_menu_destroy(menu)
     struct menu *menu;
{
    int i;

    for (i = 0; i < MENU_NO; i++) {
        del_panel(menu->panel[i]);
        delwin(menu->window[i]);
    }

    update_panels();
    doupdate();
}

int
UI_menu_process_cmd(menu, cmd)
     struct menu *menu;
     enum engine_cmd cmd;
{
    int ret = 0;

    /** Process commands **/
    switch (cmd) {
    case cmd_unfocus:
        menu->hide = 1;
        UI_hide_panel(menu->panel[menu->c_col]);
        menu->c_col = -1;
        break;
    case cmd_choose:
        if (menu->c_row == 0) {
            menu->hide = (menu->hide) ? 0 : 1;
            if (menu->hide)
                UI_hide_panel(menu->panel[menu->c_col]);
            else
                UI_show_panel(menu->panel[menu->c_col]);
            ret++;
        } else {
            ret = menu->menu[menu->c_col][menu->c_row].action;
            menu->c_row = 0;
            menu->hide = 1;
            UI_hide_panel(menu->panel[menu->c_col]);
        }
        break;
    case cmd_up:
        if (menu->c_row > 0) {
            menu->c_row--;
            ret++;
        }
        break;
    case cmd_down:
        if (menu->c_row < UI_menu_cntlines(menu->menu[menu->c_col]) - 1 && !menu->hide) {
            menu->c_row++;
            ret++;
        }
        break;
    case cmd_left:
        if (menu->c_col > 0) {
            UI_hide_panel(menu->panel[menu->c_col]);
            menu->c_col--;
            if (menu->c_row >= UI_menu_cntlines(menu->menu[menu->c_col]))
                menu->c_row = UI_menu_cntlines(menu->menu[menu->c_col]) - 1;
            ret++;
        }
        break;
    case cmd_right:
        if (menu->c_col < MENU_NO - 1) {
            UI_hide_panel(menu->panel[menu->c_col]);
            menu->c_col++;
            if (menu->c_row >= UI_menu_cntlines(menu->menu[menu->c_col]))
                menu->c_row = UI_menu_cntlines(menu->menu[menu->c_col]) - 1;
            ret++;
        }
        break;
    default:
        break;
    }

    return ret;
}

int
UI_menu_draw(menu)
     struct menu *menu;
{
    int i, offset;
    config *cf = UI_get_config();

    /** Draw top row of menu **/
    attrset(A_REVERSE);
    mvhline(0, 0, ' ', menu->x);
    for (i = 0, offset = 2; i < MENU_NO; i++, offset += menu->titlew) {
        if (menu->c_col == i) {
            attrset(COLOR_PAIR(8));
            mvaddch(0, offset, '|');
            mvaddch(0, offset + menu->titlew - 1, '|');
        } else
            attrset(A_REVERSE);

        mvhline(0, offset + 1, ' ', menu->titlew - 2);
        mvaddstr(0, offset + 1, (char *)menu->menu[i][0].text);
    }
    attrset(0);

    if (menu->c_col >= 0 && menu->c_col < MENU_NO)
        for (i = 1; i < UI_menu_cntlines(menu->menu[menu->c_col]); i++) {
            if (menu->c_row == i) {
                wattron(menu->window[menu->c_col], A_BOLD);
                mvwaddstr(menu->window[menu->c_col], i, 1, "->");
            } else {
                mvwaddstr(menu->window[menu->c_col], i, 1, "  ");
            }
            mvwaddstr(menu->window[menu->c_col],
                      i, 3, (char *)menu->menu[menu->c_col][i].text);
            if (menu->c_row == i)
                wattroff(menu->window[menu->c_col], A_BOLD);
        }
    wattrset(menu->window[menu->c_col], 0);

    if (!menu->hide) {

        /** Draw menu-items of selected menu **/
        UI_show_panel(menu->panel[menu->c_col]);
        wrefresh(menu->window[menu->c_col]);

        if (menu->c_row > 0 && cf->cursor_follow_marker)
            wmove(menu->window[menu->c_col], menu->c_row, 1);

        wrefresh(menu->window[menu->c_col]);
    }
    if (cf->cursor_follow_marker && (menu->hide || menu->c_row == 0)) {
        move(0, menu->c_col * menu->titlew + 3);
        refresh();
    }
    return 0;
}

int
UI_menu_handle_key(menu, c)
     struct menu *menu;
     int c;
{
    switch (c) {
    case KEY_UP:
        c = UI_menu_process_cmd(menu, cmd_up);
        break;
    case KEY_DOWN:
        c = UI_menu_process_cmd(menu, cmd_down);
        break;
    case KEY_LEFT:
        c = UI_menu_process_cmd(menu, cmd_left);
        break;
    case KEY_RIGHT:
        c = UI_menu_process_cmd(menu, cmd_right);
        break;
    case '\n':
    case '\r':
        c = UI_menu_process_cmd(menu, cmd_choose);
        break;
    default:
        c = -1;
        break;
    }

    if (c > 0)
        UI_menu_draw(menu);

    return c;
}
