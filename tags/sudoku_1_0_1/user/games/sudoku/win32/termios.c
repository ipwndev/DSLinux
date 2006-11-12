/* Dumb termios implementation for Win32 port
 */

#include "termios.h"

int
tcgetattr( int fd, struct termios * t )
{
    return 0;
}

int
tcsetattr( int fd, int mode, struct termios * t )
{
    return 0;
}

