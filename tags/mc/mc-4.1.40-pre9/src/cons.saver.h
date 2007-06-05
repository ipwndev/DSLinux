#ifndef __CONS_SAVER_H
#define __CONS_SAVER_H

enum {
    CONSOLE_INIT = '1',
    CONSOLE_DONE,
    CONSOLE_SAVE,
    CONSOLE_RESTORE,
    CONSOLE_CONTENTS
};

extern signed char console_flag;

void show_console_contents (int starty, unsigned char begin_line, unsigned char end_line);
void handle_console (unsigned char action);

void show_rxvt_contents (int starty, unsigned char y1, unsigned char y2);
int look_for_rxvt_extensions (void);

/* Used only in the principal program */
extern int cons_saver_pid;


#endif				/* __CONS_SAVER_H */
