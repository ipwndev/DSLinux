/* Dummy header for Win32 port
 */

#include <direct.h>
#include <io.h>
#include <process.h>

#define PATH_MAX    _MAX_PATH

#define getcwd _getcwd
#define getpid _getpid
#define isatty _isatty
#define pclose _pclose
#define popen  _popen

typedef int sigset_t;
int sigemptyset( sigset_t * );

struct sigaction
{
    void (*sa_handler)( int );
    sigset_t sa_mask;
    int sa_flags;
};

void sigaction( int, struct sigaction *, int );

