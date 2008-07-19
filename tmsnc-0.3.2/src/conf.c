
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

#include "conf.h"
#include "common.h"

// This being glooobal
config cf;

int
UI_loadaliases(dir, session)
     char *dir;
     MSN_session *session;
{
    /*
     * Read ~/.tmsnc/aliases.conf, and load its content 
     */

    char *left, *right, filenamebuf[512], linebuf[512];
    FILE *in = NULL;
    int cnt;

    snprintf(filenamebuf, sizeof(filenamebuf) - 1, "%s/.tmsnc/aliases.conf",
             dir);

    in = fopen(filenamebuf, "r");
    if (in == NULL)
        return 0;

    while (fgets(linebuf, sizeof(linebuf) - 1, in) != NULL) {
        linebuf[strlen(linebuf) - 1] = 0x0;

        /*
         * Skip lines prefixed with '#' 
         */
        if (linebuf[0] == '#')
            continue;

        /*
         * Skip lines we cannot parse 
         */
        if ((left = (char *)split(linebuf, ' ', 0)) == NULL)
            continue;

        if ((right = (char *)strchr(linebuf, ' ')) == NULL) {
            free(left);
            continue;
        }
        *right++;

        /*
         * Fill the array 
         */
        for (cnt = 0; cnt < session->num_contacts; cnt++)
            if (strcmp(session->contact[cnt]->addr, left) == 0) {
                strncpy(session->contact[cnt]->custom_name, right,
                        sizeof(session->contact[cnt]->custom_name) - 1);
                break;
            }
        free(left);
    }

    fclose(in);

    return 0;
}

int
UI_dumpaliases(dir, session)
     char *dir;
     MSN_session *session;
{
    /*
     * Write the aliases into ~/.tmsnc/aliases.conf 
     */

    char filenamebuf[512];
    FILE *out = NULL;
    int cnt;

    snprintf(filenamebuf, sizeof(filenamebuf) - 1, "%s/.tmsnc/aliases.conf",
             dir);

    out = fopen(filenamebuf, "w");
    if (out == NULL)
        return -1;

    fprintf(out,
            "# Aliases (custom nicks) file for TMSNC\n"
            "# Example:\n"
            "#joe@example.net My friend Joe\n"
            "# Please do not edit while tmsnc is running\n");

    for (cnt = 0; cnt < session->num_contacts; cnt++)
        if (session->contact[cnt]->custom_name[0] != 0)
            fprintf(out, "%s %s\n", session->contact[cnt]->addr,
                    session->contact[cnt]->custom_name);

    fclose(out);

    return 0;
}

int
UI_set_alias(contact, custom_name)
     MSN_contact *contact;
     char *custom_name;
{
    strncpy(contact->custom_name, custom_name, NAME_LEN - 1);
    return 0;
}

int
UI_delete_alias(contact)
     MSN_contact *contact;
{
    contact->custom_name[0] = 0x0;
    return 0;
}

char *
UI_get_contact_name(contact, use_nickname)
     MSN_contact *contact;
     int use_nickname;
{
    char *contact_name;

    if (contact->custom_name[0] != '\0') {
        contact_name = calloc(NAME_LEN, sizeof(char));
        if (contact_name == NULL)
            UI_err_exit("Cannot use alias for display");
        strncpy(contact_name, contact->custom_name, NAME_LEN - 1);
    } else {
        if (use_nickname == 1 && contact->name[0] != '\0') {
            contact_name = calloc(NAME_LEN, sizeof(char));
            strncpy(contact_name, contact->name, NAME_LEN - 1);
        } else
            contact_name = split(contact->addr, '@', 0);
        if (contact_name == NULL)
            UI_err_exit("Cannot split address or nickname for display");
    }
    return contact_name;
}

config *
UI_get_config(void) {
    return &cf;
}

int
UI_loadconfig(path, session)
     char *path;
     MSN_session *session;
{
    FILE *in = NULL;
    char *left, *right, buf[512];

    in = fopen(path, "r");
    if (in == NULL)
        return -1;

    /*
     * Set defaults 
     */
    strncpy(cf.initial_status, "online", 9);
    strncpy(session->format.font, "Arial", 14);
    strncpy(session->format.color, "000000", 7);
    session->format.effect = ' ';
    cf.version = 0;
    cf.away_message[0] = '\0';
    cf.pass[0] = '\0';
    cf.autologin = 0;
    cf.log = 0;
    cf.sound.player[0] = '\0';
    cf.sound.login[0] = '\0';
    cf.sound.newmsg[0] = '\0';
    cf.sound.error[0] = '\0';
    cf.codeset[0] = '\0';
    cf.display_csong = 0;
    cf.color.c1 = 1;
    cf.color.c2 = 2;
    cf.color.c3 = 3;
    cf.color.c4 = 4;
    cf.color.c5 = 5;
    cf.color.c6 = 6;
    cf.use_nickname = 1;
    cf.use_mouse = 0;
    cf.cursor_follow_marker = 0;

    while (fgets(buf, sizeof(buf) - 1, in) != NULL) {
        buf[strlen(buf) - 1] = 0x0;

        /*
         * Skip lines prefixed with '#' 
         */
        if (buf[0] == '#')
            continue;

        /*
         * Skip lines we cannot parse 
         */
        if ((left = (char *)split(buf, '=', 0)) == NULL)
            continue;

        if ((right = (char *)strchr(buf, '=')) == NULL) {
            free(left);
            continue;
        }
        *right++;

        if (strcmp(left, "initial_status") == 0)
            strncpy(cf.initial_status, right, 9);
        else if (strcmp(left, "font") == 0)
            strncpy(session->format.font, right, 14);
        else if (strcmp(left, "color") == 0 && strlen(right) == 6) {
            session->format.color[0] = right[4];
            session->format.color[1] = right[5];
            session->format.color[2] = right[2];
            session->format.color[3] = right[3];
            session->format.color[4] = right[0];
            session->format.color[5] = right[1];
        } else if (strcmp(left, "effect") == 0)
            session->format.effect = right[0];
        else if (strcmp(left, "away_message") == 0)
            strncpy(cf.away_message, right, 79);
        else if (strcmp(left, "login") == 0 && session->me.addr[0] == 0x0)
            strncpy(session->me.addr, right, ADDR_LEN - 1);
        else if (strcmp(left, "password") == 0)
            strncpy(cf.pass, right, 29);
        else if (strcmp(left, "sound_player") == 0)
            strncpy(cf.sound.player, right, 19);
        else if (strcmp(left, "sound_login") == 0)
            strncpy(cf.sound.login, right, 19);
        else if (strcmp(left, "sound_newmsg") == 0)
            strncpy(cf.sound.newmsg, right, 19);
        else if (strcmp(left, "sound_error") == 0)
            strncpy(cf.sound.error, right, 19);
        else if (strcmp(left, "codeset") == 0)
            strncpy(cf.codeset, right, 19);
        else if (strcmp(left, "display_current_song") == 0)
            cf.display_csong = atoi(right);
        else if (strcmp(left, "autologin") == 0)
            cf.autologin = atoi(right);
        else if (strcmp(left, "color_1") == 0)
            cf.color.c1 = atoi(right);
        else if (strcmp(left, "color_2") == 0)
            cf.color.c2 = atoi(right);
        else if (strcmp(left, "color_3") == 0)
            cf.color.c3 = atoi(right);
        else if (strcmp(left, "color_4") == 0)
            cf.color.c4 = atoi(right);
        else if (strcmp(left, "color_5") == 0)
            cf.color.c5 = atoi(right);
        else if (strcmp(left, "color_6") == 0)
            cf.color.c6 = atoi(right);
        else if (strcmp(left, "CONFIG_VERSION") == 0)
            cf.version = atoi(right);
        else if (strcmp(left, "use_nickname") == 0)
            cf.use_nickname = atoi(right);
        else if (strcmp(left, "use_mouse") == 0)
            cf.use_mouse = atoi(right);
        else if (strcmp(left, "log") == 0)
            cf.log = atoi(right);
        else if (strcmp(left, "cursor_follow_marker") == 0)
            cf.cursor_follow_marker = atoi(right);

        free(left);
    }

    fclose(in);
    return 0;
}

int
UI_createconfig(dir)
     char *dir;
{
    FILE *out;
    char buf[512];

    snprintf(buf, sizeof(buf) - 1, "%s/.tmsnc", dir);
    mkdir(buf, 0700);

    snprintf(buf, sizeof(buf) - 1, "%s/.tmsnc/logs", dir);
    mkdir(buf, 0700);

    snprintf(buf, sizeof(buf) - 1, "%s/.tmsnc/sounds", dir);
    mkdir(buf, 0700);

    snprintf(buf, sizeof(buf) - 1, "%s/.tmsnc/tmsnc.conf", dir);

    out = fopen(buf, "w");
    if (out == NULL)
        return -1;
    fprintf(out,
            "# Configure file for TMSNC\n"
            "CONFIG_VERSION=5\n\n"
            "# initial_status can be set to either\n"
            "# online, idle, brb, phone, lunch, away or hidden.\n"
            "initial_status=online\n\n"
            "# automatically log conversations to ~/.tmsnc/logs\n"
            "log=1\n\n"
            "# Format of outgoing messages\n"
            "# color must be in hexadecimal 6 digit form (RGB)\n"
            "# effect can be 'B' 'I' or 'U'\n"
            "font=Helvetica\n"
            "color=000000\n"
            "#effect=B\n\n"
            "# Character encoding of your terminal\n"
            "# see 'iconv -l' for a list of encodings\n"
            "# Leave commented for automatic detection\n"
            "#codeset=ISO-8859-1\n\n"
            "# Uncheck the following option if you want mouse-support\n"
            "#use_mouse=1\n\n"
            "# Uncheck the following option if you want the cursor to follow the marker\n"
            "#cursor_follow_marker=1\n\n"
            "# Uncomment to display current playing song\n"
            "# make sure that ~/.tmsnc/current_song.sh exists\n"
            "#display_current_song=1\n\n"
            "# Uncheck these options if you want sounds to be played for various events.\n"
            "# tmsnc looks in ~/.tmsnc/sounds for the sound-files.\n"
            "#sound_player=play\n"
            "#sound_login=login.wav\n"
            "#sound_newmsg=newmsg.wav\n"
            "#sound_error=error.wav\n\n"
            "# Show this message automatically when someone tries\n"
            "# to contact you while you are away\n"
            "#away_message=If you look at my status it says AWAY!\n\n"
            "# For automatic login, specify login/password here\n"
            "# WARNING: this file must be unreadable for everyone but you!\n"
            "#autologin=1\n"
            "#login=myname@passport.com\n"
            "#password=mypassword\n\n"
            "# Customizing the colors in TMSNC.\n"
            "# Each color has a unique number.\n"
            "# Here's the list of colors that you can use and their corresponding numbers.\n"
            "# Color        Number\n"
            "# term-default -1\n"
            "# black        %d\n"
            "# red          %d\n"
            "# green        %d\n"
            "# yellow       %d\n"
            "# blue         %d\n"
            "# magenta      %d\n"
            "# cyan         %d\n"
            "# white        %d\n\n"
            "# Uncomment to use other colors than the default ones.\n"
            "#color_1=3\n"
            "#color_2=2\n"
            "#color_3=6\n"
            "#color_4=5\n"
            "#color_5=7\n"
            "#color_6=1\n\n"
            "# To show your contacts' addresses instead of their nickname\n"
            "# set this variable to 0.\n"
            "#use_nickname=0\n", COLOR_BLACK, COLOR_RED, COLOR_GREEN,
            COLOR_YELLOW, COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE);
    fclose(out);

    chmod(buf, 0600);
    return 0;
}
