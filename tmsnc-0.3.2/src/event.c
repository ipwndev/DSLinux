
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

#include "event.h"
#include "common.h"

int
UI_handle_mouse(ui, menu, session, focus)
     struct ui *ui;
     struct menu *menu;
     MSN_session *session;
     int *focus;
{
    MEVENT mouse;
    int mx, my, wx, wy, r;
    char buf[1024], t_buf[16];

    if (getmouse(&mouse) == OK) {
          if (mouse.bstate & BUTTON1_CLICKED) {
              mx=mouse.x;
              my=mouse.y;
              getmaxyx(stdscr,wy,wx);

              //Menu item click
              if ((*focus==FOCUS_MENU)&&(mx<=4*menu->titlew)&&(my>0)) {
                  int j;
                  char e;
                  j=UI_menu_cntlines(menu->menu[menu->c_col]);
                  if(my<=j) {
                      menu->c_row=my-1;
                      if ((e = UI_menu_handle_key(menu, '\n')) == cmd_quit) {
                          if (UI_dialog_getch("Are you sure you want to quit? (y/n)") == 'y')
                              return -1;
                      } else {
                          r = UI_do_command(session, ui, e, buf, sizeof(buf) - 1);
                          if (r < 0) {
                              wattron(ui->main.swin, COLOR_PAIR(1) | A_BOLD);
                              UI_win_addstr(ui, &ui->main, "[%s] ",
                              UI_get_timestr(t_buf, sizeof(t_buf) - 1));
                              wattroff(ui->main.swin, COLOR_PAIR(1) | A_BOLD);
                              UI_win_addstr(ui, &ui->main, "Error: %s\n", buf);
                          }
                      }
                      UI_remove_focus(ui, menu, *focus);
                      *focus = FOCUS_MAIN;
                      UI_set_focus(ui, menu, *focus);
                      return 1;
                }
            }
            //Main click
            if ((mx < (wx-LIST_WIDTH))&&
                (my > 0)&&(my <= (wy-7))) {
                UI_remove_focus(ui, menu, *focus);
                *focus = FOCUS_MAIN;
                UI_set_focus(ui, menu, *focus);
                return 1;
            }
            //Menu click
            if (my == 0) {
                int i;
                UI_remove_focus(ui, menu, *focus);
                *focus = FOCUS_MENU;
                UI_set_focus(ui, menu, *focus);
                for(i=0;i<((mx-mx%menu->titlew)/menu->titlew);i++) {
                    UI_menu_process_cmd(menu, cmd_right);
                }
                UI_menu_process_cmd(menu, cmd_choose);
                UI_menu_draw(menu);
                return 1;
            }
            //Contact list click
            if ((mx >= wx-LIST_WIDTH)&&(my < 2*(wy-3)/3)) {
                UI_remove_focus(ui, menu, *focus);
                *focus = FOCUS_LIST;
                UI_set_focus(ui, menu, *focus);
                return 1;
            }
            //Conversation list click
            if((mx >= wx-LIST_WIDTH)&&(my >= 2*(wy-3)/3)) {
                UI_remove_focus(ui, menu, *focus);
                *focus = FOCUS_CONV;
                UI_set_focus(ui, menu, *focus);
                return 1;
            }
        }
    }
    return 0;
}

int
UI_handle_server_code(ui, session, c, str)
     struct ui *ui;
     MSN_session *session;
     int c;
     char *str;
{
    char *ptr[2] = { NULL, NULL }, t_buf[16];
    int r;

    switch (c) {
    case -2:                   /* fatal error */
        UI_play_sound(SOUND_ERROR);
        UI_err_exit("Quitting TMSNC: %s\n", str);
        break;
    case 2:                    /* initial login */
    case 6:                    /* someone logged out */
    case 11:                   /* added contact */
    case 12:                   /* removed contact */
    case 13:                   /* someone changed status */
        UI_draw_list(ui, session);
        break;
    case 8:                    /* we are invited to a conversation */
        if ((ptr[0] = split(str, ' ', 0)) == NULL ||
            (ptr[1] = split(str, ' ', 1)) == NULL)
            UI_err_exit("Cannot split conversation string");
        r = UI_start_conversation(ui, session, ptr[1], atoi(ptr[0]));
        free(ptr[0]);
        free(ptr[1]);

        if (r < 0) {
            wattron(ui->main.swin, COLOR_PAIR(1) | A_BOLD);
            UI_win_addstr(ui, &ui->main, "[%s] ",
                          UI_get_timestr(t_buf, sizeof(t_buf) - 1));
            wattroff(ui->main.swin, COLOR_PAIR(1) | A_BOLD);
            UI_win_addstr(ui, &ui->main,
                          "Error: Cannot accept conversation\n");
            UI_play_sound(SOUND_ERROR);
        }
        UI_draw_conv(ui);
        break;
    case 9:                    /* we changed screen-name */
        UI_bar_draw(session);
        break;
    case 14:                   /* someone logged in */
        UI_play_sound(SOUND_LOGIN);
        UI_draw_list(ui, session);
        break;
    default:
        break;
    }
    return 0;
}

int
UI_handle_conversation_code(ui, session, conv, c, message)
     struct ui *ui;
     MSN_session *session;
     struct conv *conv;
     int c;
     char *message;
{
    char t_buf[16], *tmp;
    int do_print_msg = 0, i, do_shift_left = 0;

    switch (c) {
    case 1:                    /* IRO */
    case 5:                    /* JOI */
        tmp = split(message, ' ', 0);
        conv->ppl = (char **)realloc(conv->ppl,
                                     (conv->num_ppl + 1) *
                                     sizeof(char *));
        conv->ppl[conv->num_ppl-1] = tmp;
        strncpy(conv->last_contact, tmp, CONTACT_LEN - 1);
        UI_draw_conv(ui);
        do_print_msg++;
        break;
    case 3:                    /* BYE (due to inactivity) */
    case 4:                    /* BYE */
        tmp = split(message, ' ', 0);
        for(i=0; i <= conv->num_ppl; i++) {
            if(strcmp(conv->ppl[i], tmp)==0)
            {
              free(conv->ppl[i]);
              do_shift_left = 1;
            }
            if(do_shift_left && i != conv->num_ppl)
              conv->ppl[i] = conv->ppl[i+1];
        }
        free(tmp);
        UI_draw_conv(ui);
        do_print_msg++;
        break;
    case -2:                   /* Connection lost */
        UI_close_conversation(ui, conv);

        wattron(ui->main.swin, COLOR_PAIR(1) | A_BOLD);
        UI_win_addstr(ui, &(ui->main), "[%s] ",
                      UI_get_timestr(t_buf, sizeof(t_buf) - 1));
        wattroff(ui->main.swin, COLOR_PAIR(1) | A_BOLD);

        UI_win_addstr(ui, &(ui->main),
                      "Notice: Connection to switchboard lost\n");
        break;
    case 0:                    /* do nothing */
        break;
    default:
        do_print_msg++;
        break;
    }

    if (do_print_msg) {
        if (ui->c_conv != conv) {
            conv->new_message = 1;
            UI_draw_conv(ui);
            UI_play_sound(SOUND_NEWMSG);
        }
        wattron(conv->win.swin, (c < 0) ? COLOR_PAIR(1) : COLOR_PAIR(2));
        UI_win_addstr(ui, &(conv->win), "[%s]",
                      UI_get_timestr(t_buf, sizeof(t_buf) - 1));
        wattroff(conv->win.swin, (c < 0) ? COLOR_PAIR(1) : COLOR_PAIR(2));

        UI_win_addstr(ui, &(conv->win), " %s\n", message);
        if (conv->logfd != NULL)
            fprintf(conv->logfd, "[%s] %s\n", t_buf, message);
    }
    return 0;
}

int
UI_main_handle_key(ui, session, c)
     struct ui *ui;
     MSN_session *session;
     int c;
{
    char t_buf[16];
    int width, height, i;
    struct win *c_win = NULL;

    if (ui->c_conv == NULL)
        c_win = &(ui->main);
    else
        c_win = &(ui->c_conv->win);

    width = c_win->x - 4;
    height = 3;

    switch (c) {
    case KEY_LEFT:
        if (c_win->field_ptr > 0)
            c_win->field_ptr--;
        break;
    case KEY_RIGHT:
        if (c_win->field_ptr < c_win->field_length)
            c_win->field_ptr++;
        break;
    case KEY_BACKSPACE:
    case 127:                  /* Backspace in terminal.app */
        if (c_win->field_ptr > 0) {
            c_win->field_ptr--;
            c_win->field_length--;
            for (i = c_win->field_ptr; i < c_win->field_length; i++)
                c_win->field_buf[i] = c_win->field_buf[i + 1];
            c_win->field_buf[i] = 0x0;
        }
        break;
    case '\n':
    case '\r':
        /*
         * Break on empty buffer 
         */
        if (c_win->field_buf[0] == 0x0)
            break;

        if (ui->c_conv == NULL) {
            /*
             * we 're in the console 
             */
            UI_win_addstr(ui, &ui->main,
                          "No commands implemeted at the moment. Sorry :)\n");

            /*
             * Clean the input field 
             */
            c_win->field_buf[0] = 0x0;
            c_win->field_ptr = 0;
            c_win->field_length = 0;
            werase(c_win->field_win);
            break;
        }
        if (ui->c_conv->num_ppl <= 0) {
            for (i = 0; i < session->num_contacts &&
                 strcmp(session->contact[i]->addr,
                        ui->c_conv->last_contact) == 0; i++);
            if (i == session->num_contacts)
                return -1;
            if (session->contact[i]->status != 7) {
		// sent msg crash after long left conversation bug, by gfhuang
		ui->conv->sd = MSN_conversation_initiate(session, ui->c_conv->last_contact, NULL);
		if(ui->conv->sd < 0) return -1;
		ui->conv->csc = 2;
	
//                MSN_conversation_call(ui->c_conv->sd, &(ui->c_conv->csc),
//                                      ui->c_conv->last_contact);

                /*
                 * to give the server some time to process
                 * our invitation
                 */
//                sleep(1);
            }
        }
        /*
         * Run the buffer through a talkfilter if available 
         */
        UI_filter_translate(c_win->field_buf, FIELDBUFSIZE - 1);

        /*
         * Send the message 
         */
        if (MSN_conversation_sendmsg(ui->c_conv->sd,
                                     &(ui->c_conv->csc),
                                     c_win->field_buf, session) == -1)
            return -1;

        wattron(c_win->swin, COLOR_PAIR(3));
        UI_win_addstr(ui, c_win, "[%s]",
                      UI_get_timestr(t_buf, sizeof(t_buf) - 1));
        wattroff(c_win->swin, COLOR_PAIR(3));

        UI_win_addstr(ui, c_win, " %s says:\n     %s\n",
                      session->me.name, c_win->field_buf);
        if (ui->c_conv->logfd != NULL) {
            fprintf(ui->c_conv->logfd, "[%s] %s says:\n     %s\n",
                    UI_get_timestr(t_buf, sizeof(t_buf) - 1),
                    session->me.name, c_win->field_buf);
        }
        c_win->field_buf[0] = 0x0;
        c_win->field_ptr = 0;
        c_win->field_length = 0;
        werase(c_win->field_win);
        break;
    case -1:
    case KEY_RESIZE:
    case KEY_UP:
    case KEY_DOWN:
        return -1;
        break;
    default:
        if (c_win->field_length >= FIELDBUFSIZE - 1)
            return -1;

        for (i = c_win->field_length; i > c_win->field_ptr && i > 0;
             i--)
            c_win->field_buf[i] = c_win->field_buf[i - 1];

        c_win->field_buf[c_win->field_ptr] = c;
        c_win->field_buf[c_win->field_length + 1] = 0x0;
        c_win->field_ptr++;
        c_win->field_length++;
        break;
    }
    UI_win_put_buffer(c_win->field_win,
                      c_win->field_buf, height, width,
                      c_win->field_ptr);
    wrefresh(c_win->field_win);
    return 0;
}

int
UI_do_command(session, ui, c, ret, retsize)
     MSN_session *session;
     struct ui *ui;
     int c;
     char *ret;
     size_t retsize;
{
    int r;
    char buf[512], r_buf[512], *ptr;

    switch (c) {
    case cmd_none:
        break;
    case cmd_statusonline:
        if (MSN_set_status(session, "online") < 0)
            UI_dialog_getch("Cannot set status to 'online'");
        else
            UI_bar_draw(session);
        break;
    case cmd_statusaway:
        if (MSN_set_status(session, "away") < 0)
            UI_dialog_getch("Cannot set status to 'away'");
        else
            UI_bar_draw(session);
        break;
    case cmd_statusidle:
        if (MSN_set_status(session, "idle") < 0)
            UI_dialog_getch("Cannot set status to 'idle'");
        else
            UI_bar_draw(session);
        break;
    case cmd_statusbusy:
        if (MSN_set_status(session, "busy") < 0)
            UI_dialog_getch("Cannot set status to 'busy'");
        else
            UI_bar_draw(session);
        break;
    case cmd_statusbrb:
        if (MSN_set_status(session, "brb") < 0)
            UI_dialog_getch("Cannot set status to 'be right back'");
        else
            UI_bar_draw(session);
        break;
    case cmd_statusphone:
        if (MSN_set_status(session, "phone") < 0)
            UI_dialog_getch("Cannot set status to 'on phone'");
        else
            UI_bar_draw(session);
        break;
    case cmd_statuslunch:
        if (MSN_set_status(session, "lunch") < 0)
            UI_dialog_getch("Cannot set status to 'on lunch'");
        else
            UI_bar_draw(session);
        break;
    case cmd_statushidden:
        if (MSN_set_status(session, "hidden") < 0)
            UI_dialog_getch("Cannot set status to 'hidden'");
        else
            UI_bar_draw(session);
        break;
    case cmd_changenick:
        if (UI_dialog_getstr("Enter new nickname", buf, sizeof(buf) - 1) == 0) {
            MSN_change_nick(buf, session);
        }
        break;
    case cmd_setpsm:
        if (UI_dialog_getstr("Enter personal message", buf, sizeof(buf) - 1) == 0) {
            MSN_set_personal_message(buf, session);
        }
        break;
    case cmd_setfilter:
#ifdef HAVE_TALKFILTERS
        if (UI_dialog_getstr
            ("Name of filter ('none' to disable)", buf,
             sizeof(buf) - 1) == 0)
            if (UI_set_filter(buf) < 0)
                UI_dialog_getch("No such filter, read the manual");
#else
        UI_dialog_getch("TMSNC was not compiled with talkfilter support");
#endif
        break;
    case cmd_invite:
        if (ui->c_conv != NULL) {
            MSN_conversation_call(ui->c_conv->sd,
                                  &ui->c_conv->csc,
                                  session->contact[UI_get_list_pointer()]->
                                  addr);
        } else
            UI_dialog_getch("This is not a conversation window st00pid.");
        break;
    case cmd_addcontact:
        if (UI_dialog_getstr("Add contact", buf, sizeof(buf) - 1) == 0) {
            if (MSN_add_contact(buf, session, 'a', r_buf, sizeof(r_buf) - 1) < 0)
                UI_dialog_getch(r_buf);
        }
        break;
    case cmd_removecontact:
        if (MSN_remove_contact
            (session->contact[UI_get_list_pointer()], session, r_buf,
             sizeof(r_buf) - 1) < 0)
            UI_dialog_getch(r_buf);
        break;
    case cmd_blockcontact:
        if (MSN_block_contact
            (session->contact[UI_get_list_pointer()]->addr, session, r_buf,
             sizeof(r_buf) - 1) < 0)
            UI_dialog_getch(r_buf);
        break;
    case cmd_unblockcontact:
        if (MSN_unblock_contact
            (session->contact[UI_get_list_pointer()]->addr, session, r_buf,
             sizeof(r_buf) - 1) < 0)
            UI_dialog_getch(r_buf);
        break;
    case cmd_console:
        UI_current_conversation(ui, NULL);
        break;
    case cmd_redraw:
        UI_manual_resize();
        break;
    case cmd_setcustomnick:
        ptr = UI_get_contact_name(session->contact[UI_get_list_pointer()],
                                  UI_get_config()->use_nickname);
        snprintf(r_buf, sizeof(r_buf) - 1, "New custom nickname for %s",
                 ptr);
        free(ptr);

        if (UI_dialog_getstr(r_buf, buf, sizeof(buf) - 1) == 0)
            UI_set_alias(session->contact[UI_get_list_pointer()], buf);
        UI_draw_list(ui, session);
        break;
    case cmd_unsetcustomnick:
        UI_delete_alias(session->contact[UI_get_list_pointer()]);
        UI_draw_list(ui, session);
        break;
    case cmd_manual:
        def_prog_mode();
        endwin();
        if ((r = fork()) == 0) {
            r = execlp("man", "man", "tmsnc", NULL);
            reset_prog_mode();
            refresh();
            if (r < 0) {
                strncpy(ret, strerror(errno), retsize);
                return -1;
            }
        } else if (r > 0)
            wait(0);
        break;
    default:
        break;
    }

    return 0;
}

//Start of iveqy
int
UI_handle_power_functions(ui, session)
     struct ui *ui;
     MSN_session *session;
{
    int i;
    char r_buf[512];

    UI_dialog_getstr("Enter command", r_buf, sizeof(r_buf) - 1);

    if (!strcasecmp("block all", r_buf)) {
        if (UI_dialog_getch("Block entire contact list? (y/n)") == 'y') {
            for (i = 0; i < session->num_contacts; i++)
                MSN_block_contact(session->contact[i]->addr, session, r_buf,
                                  sizeof(r_buf) - 1);
        }
    } else if (!strcasecmp("unblock all", r_buf)) {
        if (UI_dialog_getch("Unblock entire contact list? (y/n)") == 'y') {
            for (i = 0; i < session->num_contacts; i++)
                MSN_unblock_contact(session->contact[i]->addr, session,
                                    r_buf, sizeof(r_buf) - 1);
        }
    } else if (!strcasecmp("remove all", r_buf)) {
        if (UI_dialog_getch("Remove entire contact list? (y/n)") == 'y') {
            for (i = 0; i < session->num_contacts; i++)
                MSN_remove_contact(session->contact[i], session, r_buf,
                                  sizeof(r_buf) - 1);
        }
    } else if (!strcasecmp("whoami", r_buf)) {
        snprintf(r_buf, sizeof(r_buf) - 1, "Your email address is: %s",
                 session->me.addr);
        UI_dialog_getch(r_buf);
    } else if (!strcasecmp("export conlist", r_buf)) {
        if (UI_dialog_getstr
            ("Export contact list to file", r_buf, sizeof(r_buf) - 1) == 0) {
            FILE *outp;

            outp = fopen(r_buf, "w");

            if (outp == NULL)
                UI_dialog_getch("Error, trying to open file");
            else {
                for (i = 0; i < session->num_contacts; i++)
                    fprintf(outp, "%s\n", session->contact[i]->addr);
                fclose(outp);
                UI_dialog_getch("Export completeted, press any key");
            }
        }
    } else if (!strcasecmp("import conlist", r_buf)) {
        if (UI_dialog_getstr
            ("Import contact list to file", r_buf, sizeof(r_buf) - 1) == 0) {
            FILE *inp;
            char *ptr, buf[512];

            inp = fopen(r_buf, "r");

            while (fgets(buf, sizeof(buf) - 1, inp) != NULL) {
                if ((ptr = strchr(buf, '\n')) != NULL)
                    *ptr = '\0';
                MSN_add_contact(buf, session, 'a', r_buf, sizeof(r_buf) - 1);
            }
            fclose(inp);
        }
    }
    return 0;
}

int
UI_handle_altkey(ui, menu, focus)
     struct ui *ui;
     struct menu *menu;
     int *focus;
{
    int c;

    switch ((c = tolower(getch()))) {
    case 'a':
        UI_remove_focus(ui, menu, *focus);
        *focus = 1;
        UI_set_focus(ui, menu, *focus);
        break;
    case 's':
        UI_remove_focus(ui, menu, *focus);
        *focus = 2;
        UI_set_focus(ui, menu, *focus);
        break;
    case 'x':
        UI_remove_focus(ui, menu, *focus);
        *focus = 3;
        UI_set_focus(ui, menu, *focus);
        break;
    case 'z':
        UI_remove_focus(ui, menu, *focus);
        *focus = 4;
        UI_set_focus(ui, menu, *focus);
        break;
    case 'c':
        /*
         * close current conversation 
         */
        if (ui->c_conv != NULL) {
            UI_close_conversation(ui, ui->c_conv);
        }
        break;
    case 'w':
        if(ui->c_conv != NULL)
          for(c=0; c < ui->c_conv->num_ppl; c++)
            UI_win_addstr(ui, &ui->c_conv->win, "%s\n", ui->c_conv->ppl[c]);
        break; 
    default:
        if (c >= '0' && c <= '9') {
            c -= '0';
            if (c > ui->num_conversations)
                break;
            c--;

            if (c == -1) {      /* alt-0 */
                UI_current_conversation(ui, NULL);
            } else {
                UI_current_conversation(ui, &(ui->conv[c]));
                ui->conv[c].new_message = 0;

                UI_draw_conv(ui);
            }
        }
        break;
    }
    return 0;
}

//End of iveqy
