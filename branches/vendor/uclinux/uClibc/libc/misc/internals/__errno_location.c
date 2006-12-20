#include <errno.h>
#undef errno

weak_decl(__errno_location)
int * weak_const_function __errno_location (void)
{
    return &errno;
}

