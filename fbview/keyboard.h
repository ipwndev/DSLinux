#ifndef ds_keyboard_h
#define ds_keyboard_h

#include <stdio.h>

#define KEY_ESC 27
#define KEY_UP 'w'
#define KEY_DOWN 's'
#define KEY_LEFT 'a'
#define KEY_RIGHT 'd'

#define JUMP_SIZE 2

extern void init_keyboard();
extern void close_keyboard();
extern int kbhit();
extern int readch();

#endif
