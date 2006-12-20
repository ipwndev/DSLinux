/* Dummy header for Win32 port
 */

#define ISIG 0
#define TCSANOW 0

struct termios
{
    int c_lflag;
};

int tcgetattr( int, struct termios * );
int tcsetattr( int, int, struct termios * );
