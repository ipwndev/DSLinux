
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

#include "list.h"
#include "common.h"

int
UI_get_list_pointer(void)
{
    return list_pointer;
}

void
UI_sort_list(session)
     MSN_session *session;
{
    MSN_contact *tmp_contact;
    int i, j;

    if (session->num_contacts > 1)
        for (i = 0; i < session->num_contacts - 1; i++)
            for (j = 0; j < session->num_contacts - 1 - i; j++) {

		//group offline together, by gfhuang
		if(7 == session->contact[j+1]->status &&
		   7 != session->contact[j]->status) continue;
		if(7 == session->contact[j]->status &&
		   7 != session->contact[j+1]->status) {
	                    tmp_contact = session->contact[j];
        	            session->contact[j] = session->contact[j + 1];
                	    session->contact[j + 1] = tmp_contact;
			continue;
		}

		int temp = session->contact[j+1]->ingroup -
				session->contact[j]->ingroup;
                if ((temp < 0)	//added by gfhuang sorted by first group?
		    ||
 		    ((0 == temp)
		     &&
		     (session->contact[j + 1]->status <
                     session->contact[j]->status))
                    ||
                    ((0 == temp)
		     &&
		     (session->contact[j + 1]->status ==
                      session->contact[j]->status)
                     &&
                     (strcmp
                      (session->contact[j + 1]->addr,
                       session->contact[j]->addr) < 0))) {
                    tmp_contact = session->contact[j];
                    session->contact[j] = session->contact[j + 1];
                    session->contact[j + 1] = tmp_contact;
                }
	    }
}

void
UI_sort_conv(ui)
     struct ui *ui;
{
    struct conv tmp_conv;
    int i, j;

    if (ui->num_conversations > 1)
        for (i = 0; i < ui->num_conversations - 1; i++)
            for (j = 0; j < ui->num_conversations - 1 - i; j++)
                if (ui->conv[j + 1].sd > 0 && ui->conv[j].sd <= 0) {
                    tmp_conv = ui->conv[j];
                    ui->conv[j] = ui->conv[j + 1];
                    ui->conv[j + 1] = tmp_conv;
                }
}

void
UI_erase_listwin(win)
     WINDOW *win;
{
    int y, x, i, j;

    getmaxyx(win, y, x);
    for (i = 3; i < y - 1; i++)
        for (j = 1; j < x - 1; j++)
            mvwaddch(win, i, j, ' ');
}

int
UI_list_handle_key(session, ui, c)
     MSN_session *session;
     struct ui *ui;
     int c;
{
    char buf[256], r_buf[256];
    int x, y;

    switch (c) {
    // KEY_PAGEDOWN, KEY_PAGEUP, added by gfhuang
    case KEY_NPAGE:
	getmaxyx(ui->list_win, y, x);
    	y -= 4;
	list_pointer += y;
	list_offset += y;
    	if (list_offset >= session->num_contacts)
        	list_offset = session->num_contacts - 1;
	break;
    case KEY_PPAGE:
	getmaxyx(ui->list_win, y, x);
    	y -= 4;
	list_pointer -= y;
	list_offset -= y;
    	if (list_offset < 0)
        	list_offset = 0;
	break;
	
    case KEY_DOWN:
        list_pointer++;
        break;
    case KEY_UP:
        list_pointer--;
        break;
    case 'd':                  /* delete */
        snprintf(buf, sizeof(buf) - 1, "Remove %s? (y/n)",
                 session->contact[list_pointer]->addr);
        if (UI_dialog_getch(buf) == 'y')
            if (MSN_remove_contact(session->contact[list_pointer],
                                   session, buf, sizeof(buf) - 1) != 0)
                UI_dialog_getch(buf);
        break;
    case 'a':                  /* add */
#ifdef DEBUG
        debug_log("Adding contact %s, list_num = %d\n",
                  session->contact[list_pointer]->addr,
                  session->contact[list_pointer]->listnum);
#endif
        if ((RL & session->contact[list_pointer]->listnum) > 0 && 
            (FL & session->contact[list_pointer]->listnum) == 0) {
            if (MSN_add_contact(session->contact[list_pointer]->addr,
                                session, 'a', r_buf, sizeof(r_buf) - 1) < 0)
                UI_dialog_getch(r_buf);
        } else if (UI_dialog_getstr("Add contact", buf, sizeof(buf) - 1) == 0) {
            if (MSN_add_contact(buf, session, 'a', r_buf, sizeof(r_buf) - 1) < 0)
                UI_dialog_getch(r_buf);
        }
        break;
    case 'b':                  /* block */
        snprintf(buf, sizeof(buf) - 1, "Block %s? (y/n)",
                 session->contact[list_pointer]->addr);
        if (UI_dialog_getch(buf) == 'y')
            if (MSN_block_contact(session->contact[list_pointer]->addr,
                                  session, buf, sizeof(buf) - 1) != 0)
                UI_dialog_getch(buf);
        break;
    case 'u':                  /* unblock */
        if (MSN_unblock_contact(session->contact[list_pointer]->addr,
                                session, buf, sizeof(buf) - 1) != 0)
            UI_dialog_getch(buf);
        break;
    case 'i':                  /* invite */
        if (ui->c_conv != NULL) {
            MSN_conversation_call(ui->c_conv->sd,
                                  &ui->c_conv->csc,
                                  session->contact[list_pointer]->addr);
        }
        break;
    case 'I':                  /* information */
        UI_win_addstr(ui, ((ui->c_conv == NULL) ? &(ui->main) : &(ui->c_conv->win)),
                      "\nAddress: %s\nName: %s\nCustom name: %s\nCurrent media: %s\n"
                      "Personal message: %s\nGUID: %s\nStatus: %s\nListnum: %d\n",
                      session->contact[list_pointer]->addr,
                      session->contact[list_pointer]->name,
                      session->contact[list_pointer]->custom_name,
                      session->contact[list_pointer]->cmedia,
                      session->contact[list_pointer]->psm,
                      session->contact[list_pointer]->guid,
                      MSN_status2str(session->contact[list_pointer]->status),
                      session->contact[list_pointer]->listnum);
        break;
    case 'f':		//list_mode by gfhuang
	++list_show;
	if(list_show >= list_show_END) list_show = list_show_default;
	break;
    case '\n':
    case '\r':
        return list_pointer;
        break;
    default:
        break;
    }
    return -1;
}

int
UI_conv_handle_key(c)
     int c;
{
    switch (c) {
    case KEY_DOWN:
        conv_pointer++;
        break;
    case KEY_UP:
        conv_pointer--;
        break;
    case '\n':
    case '\r':
        return conv_pointer;
        break;
    default:
        break;
    }
    return -1;
}

void
UI_draw_list(ui, session)
     struct ui *ui;
     MSN_session *session;
{
    int i, y, x, attr;
    char buf[256], *name;
    config *cf = UI_get_config();

    UI_erase_listwin(ui->list_win);
    getmaxyx(ui->list_win, y, x);
    y -= 4;

    if (session->num_contacts <= 0) {
        update_panels();
        doupdate();
        return;
    }
    if (list_pointer >= session->num_contacts)
        list_pointer = session->num_contacts - 1;
    else if (list_pointer < 0)
        list_pointer = 0;

    if (list_pointer >= (y + list_offset))
	list_offset = list_pointer - y + 1;		// by gfhuang, avoid page jump
//        list_offset += y;
    if (list_pointer < list_offset)
	list_offset = list_pointer;			//by gfhuang, avoid page jump
//        list_offset -= y;

    UI_sort_list(session);

    for (i = list_offset; i < session->num_contacts && i < (y + list_offset);
         i++) {
	// list_shown by gfhuang
	switch(list_show) {
	case list_show_addr:
		name = session->contact[i]->addr;
		break;
	case list_show_nick:
		name = session->contact[i]->name;
		break;
	case list_show_psm:
		name = session->contact[i]->psm;
		break;
	default:
	        name = UI_get_contact_name(session->contact[i], cf->use_nickname);
	}
	//end, by gfhuang

        if (name == NULL)
            UI_err_exit("Cannot get contact name");

        if ((RL & session->contact[i]->listnum) == 0 &&
            (FL & session->contact[i]->listnum) > 0)
            snprintf(buf, sizeof(buf) - 1, "[x]%s", name);		//gfhuang, hasn't added you
        else if ((RL & session->contact[i]->listnum) > 0 &&
                 (FL & session->contact[i]->listnum) == 0)
            snprintf(buf, sizeof(buf) - 1, "[?]%s", name);		//gfhuang, not added
        else if ((BL & session->contact[i]->listnum) > 0)
            snprintf(buf, sizeof(buf) - 1, "[b]%s", name);		//gfhuang, block
        else
            snprintf(buf, sizeof(buf) - 1, "%s (%s)",
                     name, MSN_status2str(session->contact[i]->status));
        if(list_show == list_show_default) free(name);

        switch (session->contact[i]->status) {
        case 0:                /* online = green */
            attr = COLOR_PAIR(2);
            break;
        case 1:                /* busy */
        case 4:                /* on phone */
        case 5:                /* on lunch */
            attr = COLOR_PAIR(5);       /* magenta */
            break;
        case 2:                /* idle = yellow */
            attr = COLOR_PAIR(3);
            break;
        case 3:                /* away */
        case 6:                /* brb */
            attr = COLOR_PAIR(6);       /* cyan */
            break;
        default:
            attr = COLOR_PAIR(0);       /* black */
            break;
        }
	//draw group line, added by gfhuang
	if((i != session->num_contacts) && (0 != session->contact[i]->ingroup - 
						 	session->contact[i+1]->ingroup))
		attr |= A_UNDERLINE;

        wattron(ui->list_win, attr);

        if (list_pointer == i)
            wattron(ui->list_win, A_REVERSE);

//        mvwhline(ui->list_win, (i % y) + 3, 1, ' ', x - 2);
//        mvwaddnstr(ui->list_win, (i % y) + 3, 1, buf, x - 3 /* 2 */); // by gfhuang, for half chinese
        mvwhline(ui->list_win, (i - list_offset) + 3, 1, ' ', x - 2);
        mvwaddnstr(ui->list_win, (i - list_offset) + 3, 1, buf, x - 3 /* 2 */); // by gfhuang, for half chinese

        wattrset(ui->list_win, 0);
    }

    if (cf->cursor_follow_marker)
        wmove(ui->list_win, (list_pointer - list_offset) + 3, 1);		//by gfhuang
//        wmove(ui->list_win, (list_pointer % y) + 3, 1);	

    wrefresh(ui->list_win);
}

void
UI_draw_conv(ui)
     struct ui *ui;
{
    int i, y, x;
    char *title;

    UI_erase_listwin(ui->conv_win);
    getmaxyx(ui->conv_win, y, x);
    y -= 4;

    if (ui->num_conversations <= 0) {
        update_panels();
        doupdate();
        return;
    }
    if (conv_pointer >= ui->num_conversations)
        conv_pointer = ui->num_conversations - 1;
    else if (conv_pointer < 0)
        conv_pointer = 0;
    if (conv_pointer >= (y + conv_offset))
        conv_offset += y;
    if (conv_pointer < conv_offset)
        conv_offset -= y;

    UI_sort_conv(ui);

    for (i = conv_offset; i < ui->num_conversations &&
         i < (y + conv_offset) && ui->conv[i].sd != 0; i++) {
        if (conv_pointer == i)
            wattron(ui->conv_win, A_REVERSE);

        if (ui->conv[i].new_message && conv_pointer == i)
            wattrset(ui->conv_win, COLOR_PAIR(2) | A_REVERSE);
        else if (conv_pointer == i)
            wattrset(ui->conv_win, A_REVERSE);
        else if (ui->conv[i].new_message)
            wattrset(ui->conv_win, COLOR_PAIR(2));

        mvwhline(ui->conv_win, (i % y) + 3, 1, ' ', x - 2);

        if (ui->conv[i].num_ppl>1)
            title = strdup("MSN Conversation");
        else if (ui->conv[i].last_contact[0] == 0x0)
            title = strdup("New Conversation");
        else
            title = split(ui->conv[i].last_contact, '@', 0);
        mvwaddnstr(ui->conv_win, (i % y) + 3, 1, title, x - 2);
        free(title);
        wattrset(ui->conv_win, 0);
    }

    if (UI_get_config()->cursor_follow_marker)
        wmove(ui->conv_win, (conv_pointer % y) + 3, 1);

    wrefresh(ui->conv_win);
}
