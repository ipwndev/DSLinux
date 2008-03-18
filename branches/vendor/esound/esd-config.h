#ifndef ESD_CONFIG_H
#define ESD_CONFIG_H

#include <string.h>

/* these are some defines to use in when reading
 * esd.conf */

#define LINEBUF_SIZE 1024

/* use strtok_r if available */
#ifdef HAVE_STRTOK_R
#define DO_STRTOK(S,DELIM) strtok_r(S,DELIM,strtok_state)
char *strtok_state[LINEBUF_SIZE];
#else
#define DO_STRTOK(S,DELIM) strtok(S,DELIM)
#endif

#endif

