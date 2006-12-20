/* Dummy signal implementation for Win32 port
 */

#include "unistd.h"

int
sigemptyset( sigset_t * sigset )
{
    *sigset = 0;
    return 0;
}

void
sigaction( int sig, struct sigaction * action, int ignored )
{
    /* Nothing */
}
