
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

#include "sig.h"
#include "common.h"

int has_received_sigwinch = 0;
int has_received_sighup = 0;
int has_received_sigint = 0;

void
sighup(int sig)
{
    has_received_sighup = 1;
}

void
sigint(int sig)
{
    if (has_received_sigint)
        UI_err_exit("Forced quit, settings not saved\n");

    if (UI_dialog_getch("Are you sure you want to quit? (y/n)") == 'y')
        has_received_sigint = 1;
}

void
sigwinch(int sig)
{
    has_received_sigwinch = 1;
}

void
UI_manual_resize()
{
    sigwinch(0);
}

int
UI_is_resized()
{
    int i = has_received_sigwinch;

    has_received_sigwinch = 0;
    return i;
}

int
UI_is_killed()
{
    int i;

    i = (has_received_sighup || has_received_sigint);
    has_received_sighup = 0;
    has_received_sigint = 0;
    return i;
}

void
UI_setup_signals()
{
    signal(SIGHUP, sighup);
    signal(SIGINT, sigint);
    signal(SIGWINCH, sigwinch);
}
