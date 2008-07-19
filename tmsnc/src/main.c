
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

#include "main.h"
#include "common.h"

void
UI_init_ncurses(void)
{
    config *cf = UI_get_config();

    initscr();
    start_color();
    use_default_colors();
    timeout(20);
    noecho();
    keypad(stdscr, TRUE);

    init_pair(1, cf->color.c1, -1);
    init_pair(2, cf->color.c2, -1);
    init_pair(3, cf->color.c3, -1);
    init_pair(4, cf->color.c4, -1);
    init_pair(5, cf->color.c5, -1);
    init_pair(6, cf->color.c6, -1);

    init_pair(7, -1, cf->color.c1);
    init_pair(8, -1, cf->color.c2);
}

int
main(argc, argv)
     int argc;
     char *argv[];
{
    struct passwd *passwd_entry;
    struct ui ui;
    struct menu menu;
    char buf[1024], t_buf[16];
    MSN_session session;
    config *cf = UI_get_config();
    time_t tm, tm_stamp = (time_t)0;
    int c, r, focus = 2, quit = 0;

    t_buf[0] = 0x0;

    MSN_init(&session);

#ifdef DEBUG
    debug_init();
#endif

    /*** Check the arguments ***/
    while ((c = getopt(argc, argv, "huvVl:i:")) != -1)
        switch (c) {
        case 'v':
        case 'V':
            printf("Text-based MSN Client (TMSNC) v.%s\n", VERSION);
            exit(0);
            break;
        case 'u':
            UI_check_for_updates();
            exit(0);
            break;
        case 'h':
            UI_usage();
            exit(0);
            break;
        case 'l':
            strncpy(session.me.addr, optarg, ADDR_LEN - 1);
            break;
        case 'i':
            /*
             * temporarily use t_buf 
             */
            strncpy(t_buf, optarg, sizeof(t_buf) - 1);
            break;
        case '?':
            if (optopt == 'c')
                fprintf(stderr, "Option -%c requires an argument.\n",
                        optopt);
            else if (isprint(optopt))
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf(stderr, "Unknown option character `\\x%x'.\n",
                        optopt);
            return 1;
        default:
            abort();
        }

    /*** Print info ***/
    printf("Starting TMSNC\nVersion: %s\nProcess ID: %d\nTerminal emulation: %s\n",
           VERSION, getpid(), getenv("TERM"));

    /*** Create and/or load config ***/
    passwd_entry = getpwuid(getuid());
    strncpy(cf->homedir, passwd_entry->pw_dir, sizeof(cf->homedir) - 1);
    snprintf(buf, sizeof(buf) - 1, "%s/.tmsnc/tmsnc.conf", cf->homedir);
    if (access(buf, R_OK) != 0) {
        if (UI_createconfig(passwd_entry->pw_dir) < 0) {
            perror("createconfig");
            UI_err_exit("Couldn't create configuration file");
        }
    }
    if (UI_loadconfig(buf, &session) < 0) {
        perror("loadconfig");
        UI_err_exit("Couldn't load configuration file");
    }

    if (cf->version < 5)
        UI_err_exit("Your configuration file (~/.tmsnc/tmsnc.conf) is outdated\n"
                    "Please delete it to generate a new one\n");

    if (t_buf[0] != 0x0)
        strncpy(cf->initial_status, t_buf, sizeof(cf->initial_status) - 1);

#ifdef HAVE_ICONV
    if (iconv_init(cf->codeset) != 0) {
        fprintf(stderr, "iconv: Character encoding translation not possible\n");
        return -1;
    }
#endif

    /*** Initialize ncurses ***/
    UI_init_ncurses();

    /*** Setup signal handlers */
    UI_setup_signals();

    /*** Initiate the MSN session ***/
    if (UI_login(&session, "messenger.hotmail.com", 1863) < 0)
        UI_err_exit("Couldn't initiate MSN session");

    /*** Set initial presence status ***/
    if (MSN_set_status(&session, cf->initial_status) < 0)
        UI_err_exit("Couldn't set initial presence status");

    /*** Load aliases ***/
    if (UI_loadaliases(passwd_entry->pw_dir, &session) < 0)
        UI_err_exit("Couldn't load aliases");

    /*** Make socket-descriptor non-blocking ***/
    fcntl(session.sd, F_SETFL, O_NONBLOCK);

    /*** Setup mouse-mask ***/
    if(cf->use_mouse)
        mousemask(ALL_MOUSE_EVENTS, NULL);

    /*** Init UI ***/
    UI_ui_init(&ui);
    UI_menu_init(&menu);
    UI_conv_nullify(&ui);
    ui.main.buf[0] = 0;
    ui.num_conversations = 0;

    /*** Draw UI ***/
    UI_ui_draw(&ui);
    UI_menu_draw(&menu);
    UI_bar_draw(&session);
    UI_put_msg("Successfully logged in");
    UI_draw_list(&ui, &session);
    UI_set_focus(&ui, &menu,  focus);

    /*** Enter main loop ***/
    while (!quit) {
        if (UI_is_killed()) {
            quit++;
            continue;
        }
        if (UI_is_resized()) {

            /** Destroy **/
            for (c = 0; c < ui.num_conversations; c++)
                UI_win_destroy(&ui.conv[c].win);
            UI_ui_destroy(&ui);
            UI_menu_destroy(&menu);

            /** Restart **/
            while (1) {
                endwin();
                initscr();
                erase();
                refresh();

                /** Make sure the terminal is large enough **/
                getmaxyx(stdscr, ui.y, ui.x);
                if (ui.x < 72 || ui.y < 15)
                    UI_dialog_getch("Terminal too small, must be at least 72x15");
                else
                    break;
            }

            /** Rebuild **/
            UI_ui_init(&ui);
            UI_ui_draw(&ui);
            UI_bar_draw(&session);
            UI_put_msg("Received SIGWINCH, terminal resized");
            UI_menu_init(&menu);
            UI_menu_draw(&menu);
            for (c = 0; c < ui.num_conversations; c++) {
                UI_win_init(&ui.conv[c].win);
                UI_win_draw(&ui.conv[c].win, ui.conv[c].last_contact);
            }
            UI_draw_list(&ui, &session);
            UI_draw_conv(&ui);
            UI_remove_focus(&ui, &menu, FOCUS_MENU);
            UI_set_focus(&ui, &menu, focus);
        }

        /*** Handle keyboard events ***/
        if ((c = getch()) != ERR) {
            //Start of iveqy
            if (c == 16) {      /* ctrl+P */
                UI_handle_power_functions(&ui, &session);
                continue;
            }
            if (c == 27 || c == 5) {      /* alt or ctrl-e */
                UI_handle_altkey(&ui, &menu, &focus);
                continue;
            }
            //End of iveqy
            if (c == '\t' || c == KEY_F(8)) {
                /*
                 * rotate focus clockwise 
                 */

                UI_remove_focus(&ui, &menu, focus);
                focus = (focus >= FOCUS_MAIN) ? FOCUS_MENU : focus + 1;
                UI_set_focus(&ui, &menu,  focus);
                continue;
            }

            if (c == KEY_MOUSE) {
                /* 
                 * handle mouse events
                 */
                if (UI_handle_mouse(&ui, &menu, &session, &focus) == -1) {
		    quit++;
		}
                continue;
            }
            if (c == KEY_F(7)) {
                /*
                 * rotate focus anti-clockwise 
                 */
                UI_remove_focus(&ui, &menu, focus);
                focus = (focus <= FOCUS_MENU) ? FOCUS_MAIN : focus - 1;
                UI_set_focus(&ui, &menu, focus);
                continue;
            }
            if (c == KEY_F(6)) {
                if (ui.c_conv != NULL) {
                    UI_close_conversation(&ui, ui.c_conv);
                }
                continue;
            }
            if (focus == FOCUS_MENU) {
                if ((c = UI_menu_handle_key(&menu, c)) == cmd_quit) {
                    if (UI_dialog_getch
                        ("Are you sure you want to quit? (y/n)") == 'y')
                        quit++;
                } else {
                    r = UI_do_command(&session, &ui, c, buf,
                                      sizeof(buf) - 1);
                    if (r < 0) {
                        wattron(ui.main.swin, COLOR_PAIR(1) | A_BOLD);
                        UI_win_addstr(&ui, &ui.main, "[%s] ",
                                      UI_get_timestr(t_buf, sizeof(t_buf) - 1));
                        wattroff(ui.main.swin, COLOR_PAIR(1) | A_BOLD);

                        UI_win_addstr(&ui, &ui.main, "Error: %s\n", buf);
                    }
                }
            } else if (focus == FOCUS_LIST) {
                c = UI_list_handle_key(&session, &ui, c);
                UI_draw_list(&ui, &session);
                if (c >= 0) {
                    /*
                     * Start conversation 
                     */
                    r = UI_start_conversation(&ui,
                                              &session,
                                              session.contact[c]->addr, 0);
                    if (r < 0) {
                        UI_play_sound(SOUND_ERROR);
                        wattron(ui.main.swin, COLOR_PAIR(1) | A_BOLD);
                        UI_win_addstr(&ui, &ui.main, "[%s] ",
                                      UI_get_timestr(t_buf, sizeof(t_buf) - 1));
                        wattroff(ui.main.swin, COLOR_PAIR(1) | A_BOLD);
                        UI_win_addstr(&ui, &ui.main,
                                      "Error: Cannot start conversation\n");
                    } else {
                        UI_remove_focus(&ui, &menu, focus);
                        focus = FOCUS_MAIN;
                        UI_set_focus(&ui, &menu, focus);
                    }
                    UI_draw_conv(&ui);
                }
            } else if (focus == FOCUS_CONV) {
                c = UI_conv_handle_key(c);
                if (c >= 0 && ui.num_conversations != 0) {
                    UI_current_conversation(&ui, &(ui.conv[c]));
                    ui.conv[c].new_message = 0;
                }
                UI_draw_conv(&ui);
            } else if (focus == FOCUS_MAIN) {
                UI_main_handle_key(&ui, &session, c);
            }
        }

        /*** Handle server messages ***/
        c = MSN_server_handle(&session, buf, sizeof(buf) - 1);
        if (c != 0) {
            wattron(ui.main.swin, COLOR_PAIR(2));
            UI_win_addstr(&ui, &ui.main, "[%s]",
                          UI_get_timestr(t_buf, sizeof(t_buf) - 1));
            wattroff(ui.main.swin, COLOR_PAIR(2));

            UI_put_msg(buf);
            UI_win_addstr(&ui, &ui.main, " %s\n", buf);
        }
        UI_handle_server_code(&ui, &session, c, buf);

        /*** Handle conversations ***/
        for (r = 0; r < ui.num_conversations; r++) {
            c = MSN_conversation_handle(ui.conv[r].sd,
                                        &ui.conv[r].num_ppl, buf,
                                        sizeof(buf) - 1,
					&session);

            UI_handle_conversation_code(&ui, &session, &(ui.conv[r]), c,
                                        buf);
        }

        /*** Update current media ***/
        if (cf->display_csong == 1) {
            time(&tm);
            if ((long)tm > (long)tm_stamp + 15) {
                UI_update_current_song(&session);
                tm_stamp = tm;
            }
        }
    }                           /* end main-loop */

    UI_dumpaliases(passwd_entry->pw_dir, &session);

    /** Destroy interface **/
    UI_menu_destroy(&menu);
    UI_ui_destroy(&ui);

#ifdef HAVE_ICONV
    iconv_destroy();
#endif

#ifdef DEBUG
    debug_destroy();
#endif

    endwin();
    close(session.sd);
    printf("Logged out from MSN server, quitting TMSNC\n"
             "If you find TMSNC useful, please consider a small donation\n");
    return 0;
}
