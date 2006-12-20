#ifndef _CLOCKSTUFF_H
#define _CLOCKSTUFF_H

#include <time.h>
#include <sys\timeb.h>

#include "ntp_machine.h"
#include "ntp_syslog.h"

/* Windows NT versions of gettimeofday and settimeofday
 *
 * ftime() has internal DayLightSavings related BUGS
 * therefore switched to GetSystemTimeAsFileTime()
 */

/* 100ns intervals between 1/1/1601 and 1/1/1970 as reported by
 * SystemTimeToFileTime()
 */

#define FILETIME_1970     0x019db1ded53e8000
#define HECTONANOSECONDS  10000000ui64
#endif
