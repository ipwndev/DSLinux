// mcburn.h
// Header file for cdrecord support in Midnight Commander
// Copyright 2001 Bart Friederichs

// Changed for CVS

#ifndef __MCBURN_H
#define __MCBURN_H

#ifdef __PANEL_H

//#include "panel.h"

void do_burn (WPanel *panel);
void burn_config ();
void init_burn_config ();						/* initialize burner config dialog */
void init_burn ();									/* initialize burner dialog */
void load_mcburn_settings ();
void save_mcburn_settings ();
int scan_for_recorder(char *);
char *concatstrings(const char *, const char *);	/* returns a string that is the concatenation of s1 and s2 */
static const char* configfile = "/.mc/mcburn.conf";		/* watch it! $HOME_DIR will be prepended to this path */

#endif

#endif
