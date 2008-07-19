
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

#include "misc.h"
#include "common.h"

void
UI_usage()
{
    printf("Usage: tmsnc [OPTION(s)]\n"
             "TMSNC is a textbased MSN client with an NCurses interface\n\n"
             "OPTION can be one or more of the following:\n"
             "-h               print this help\n"
             "-l <username>    specify your username\n"
             "-u               check wether a newer version of TMSNC is available or not\n"
             "-i <status>      set initial status where <status> is 'online', 'away', 'brb',\n"
             "                 'busy', 'idle', 'phone', 'lunch' or 'hidden'.\n"
             "-v               display version information\n\n"
             "Please report bugs to sanoi[at]freeshell.org\n");
}

void
UI_play_sound(sound)
     int sound;
{
    FILE *devnull;
    char buffer[256];
    int c;
    config *cf = UI_get_config();

    if (sound == SOUND_LOGIN)
        snprintf(buffer, sizeof(buffer), "%s/.tmsnc/sounds/%s",
                 cf->homedir, cf->sound.login);
    else if (sound == SOUND_NEWMSG)
        snprintf(buffer, sizeof(buffer), "%s/.tmsnc/sounds/%s",
                 cf->homedir, cf->sound.newmsg);
    else if (sound == SOUND_ERROR)
        snprintf(buffer, sizeof(buffer), "%s/.tmsnc/sounds/%s",
                 cf->homedir, cf->sound.error);
    if ((c = fork()) == 0) {
        /*
         * redirect stdout and stderr to /dev/null 
         */
        devnull = fopen("/dev/null", "w");
        dup2(fileno(devnull), fileno(stdout));
        dup2(fileno(devnull), fileno(stderr));
        execlp(cf->sound.player, cf->sound.player, buffer, NULL);
        fclose(devnull);
        exit(0);
    }
    wait(&c);
}

int
UI_err_exit(char *fmt, ...)
{
    va_list ap;

    endwin();

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    exit(-1);
}

int
UI_update_current_song(session)
     MSN_session *session;
{
    FILE *cmd;
    char buf[256], csong[256], *ptr;
    
    if ((cmd = popen(CSONG_SCRIPT_PATH, "r")) == NULL)
        return -1;
    if (fgets(csong, sizeof(csong) - 1, cmd) == NULL)
        return -1;
    pclose(cmd);
    if ((ptr = strchr(csong, '\n')) != NULL)
        *ptr = '\0';
    snprintf(buf, sizeof(buf) - 1,
             "\\0Music\\01\\0{0}\\0%s\\0\\0", csong);

    return MSN_set_current_media(buf, session);
}

char *
UI_get_timestr(ret, size)
     char *ret;
     size_t size;
{
    struct tm *ptr;
    time_t tm;

    tm = time(NULL);
    ptr = localtime(&tm);
    strftime(ret, size, "%H:%M", ptr);
    return ret;
}

int
UI_check_for_updates(void)
{
    int sd;
    char buf[512];

    if ((sd = tcp_connect("tmsnc.sourceforge.net", 80)) < 0)
        UI_err_exit("Couldn't connect to tmsnc.sf.net");

    if (send(sd,
             "GET /version.txt HTTP/1.1\r\n"
             "Host: tmsnc.sourceforge.net\r\n\r\n", 59, 0) < 0)
        UI_err_exit("Couldn't send HTTP request");

    while (getline(buf, sizeof(buf) - 1, sd) > 0) {
        buf[strlen(buf) - 1] = 0x0;
        if (strncmp(buf, "Version", 7) == 0) {
            if (strcmp(&buf[9], VERSION) == 0) {
                printf("You have the newest version installed\n");
                exit(0);
            } else {
                printf("Newer version (%s) available at "
                       "http://tmsnc.sf.net\n", &buf[9]);
                exit(0);
            }
        }
    }

    UI_err_exit("Unknown error");
    return 0;
}

void
UI_conv_nullify(ui)
     struct ui *ui;
{
    int i;

    for (i = 0; i < MAX_CONV; i++) {
        ui->conv[i].sd = 0;
        ui->conv[i].csc = 0;
        ui->conv[i].num_ppl = 0;
        ui->conv[i].new_message = 0;
        ui->conv[i].logfd = NULL;
        ui->conv[i].last_contact[0] = 0x0;
        ui->conv[i].ppl = NULL;
        ui->conv[i].win.buf[0] = 0x0;
    }
}

int
UI_start_conversation(ui, session, address, mode)
     struct ui *ui;
     MSN_session *session;
     char *address;
     int mode;
{

    /*
     * mode =  0     start conversation mode != 0     accept conversation
     */
    time_t tm;
    struct passwd *pw;
    char buf[256];

    int ret, temp_sd;

    temp_sd =
        (mode == 0) ? MSN_conversation_initiate(session, address, NULL) : mode;
    if (temp_sd < 0)
        return -1;

    for (ret = 0; ret < MAX_CONV; ret++) {
        if (strcmp(ui->conv[ret].last_contact, address) == 0
            && ui->conv[ret].num_ppl <= 1) {
            close(ui->conv[ret].sd);
            break;
        }
    }

    if (ret == MAX_CONV) {
        for (ret = 0; ret < MAX_CONV; ret++) {
            if (ui->conv[ret].sd == 0) {
                UI_win_init(&ui->conv[ret].win);
                UI_current_conversation(ui, &(ui->conv[ret]));
                ui->conv[ret].ppl = (char **)calloc(1, sizeof(char *));
                UI_win_draw(&ui->conv[ret].win, address);
                ui->num_conversations += 1;
                if (UI_get_config()->log) {
                    time(&tm);
                    pw = getpwuid(getuid());
                    snprintf(buf, sizeof(buf) - 1, "%s/.tmsnc/logs/%s",
                             pw->pw_dir, address);
                    if ((ui->conv[ret].logfd = fopen(buf, "a")) != NULL)
                        fprintf(ui->conv[ret].logfd,
                                "############ NEW CONVERSATION ############\n%s\n",
                                asctime(localtime(&tm)));
                }
                if (mode != 0)
                    UI_play_sound(SOUND_NEWMSG);

                break;
            }
        }
    }
    ui->conv[ret].sd = temp_sd;

    ui->conv[ret].csc = 2;
    ui->conv[ret].num_ppl = 0;
    ui->conv[ret].last_contact[0] = 0x0;

    return ret;
}

void
UI_close_conversation(ui, c)
     struct ui *ui;
     struct conv *c;
{
    int i;

    UI_current_conversation(ui, NULL);
    UI_win_destroy(&c->win);

    close(c->sd);
    if (c->logfd != NULL)
        fclose(c->logfd);

    for(i=1; i <= c->num_ppl; i++)
      free(c->ppl[i-1]);
    free(c->ppl);

    c->sd = 0;
    c->csc = 0;
    c->num_ppl = 0;
    c->new_message = 0;
    c->logfd = NULL;
    c->last_contact[0] = 0x0;
    c->win.buf[0] = 0x0;
    c = NULL;
    UI_sort_conv(ui);
    ui->num_conversations -= 1;
    UI_draw_conv(ui);
    ui->c_conv = NULL;
}
