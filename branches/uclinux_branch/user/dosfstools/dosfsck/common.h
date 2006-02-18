/* common.h  -  Common functions */

/* Written 1993 by Werner Almesberger */


#ifndef _COMMON_H
#define _COMMON_H

void die(char *msg,...) __attribute((noreturn));

/* Displays a prinf-style message and terminates the program. */

void pdie(char *msg,...) __attribute((noreturn));

/* Like die, but appends an error message according to the state of errno. */

void *alloc(int size);

/* mallocs SIZE bytes and returns a pointer to the data. Terminates the program
   if malloc fails. */

void *qalloc(void **root,int size);

/* Like alloc, but registers the data area in a list described by ROOT. */

void qfree(void **root);

/* Deallocates all qalloc'ed data areas described by ROOT. */

int min(int a,int b);

/* Returns the smaller integer value of a and b. */

char get_key(char *valid,char *prompt);

/* Displays PROMPT and waits for user input. Only characters in VALID are
   accepted. Terminates the program on EOF. Returns the character. */

#endif
