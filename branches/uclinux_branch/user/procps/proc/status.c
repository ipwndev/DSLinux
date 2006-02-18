/***********************************************************************\
*   Copyright (C) 1992-1998 by Michael K. Johnson, johnsonm@redhat.com *
*                                                                      *
*      This file is placed under the conditions of the GNU Library     *
*      General Public License, version 2, or any later version.        *
*      See file COPYING for information on distribution conditions.    *
\***********************************************************************/


#include "proc/procps.h"
#include "proc/readproc.h"

char * status(proc_t* task) {
    static char buf[4] = "   ";

    buf[0] = task->state;
    if (task->rss == 0 && task->state != 'Z')
        buf[1] = 'W';
    else
        buf[1] = ' ';
    if (task->nice < 0)
	buf[2] = '<';
    else if (task->nice > 0)
	buf[2] = 'N';
    else
	buf[2] = ' ';

    return(buf);
}
