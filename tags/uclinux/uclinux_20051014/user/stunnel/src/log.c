/*
 *   stunnel       Universal SSL tunnel
 *   Copyright (c) 1998-2004 Michal Trojnara <Michal.Trojnara@mirt.net>
 *                 All Rights Reserved
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   In addition, as a special exception, Michal Trojnara gives
 *   permission to link the code of this program with the OpenSSL
 *   library (or with modified versions of OpenSSL that use the same
 *   license as OpenSSL), and distribute linked combinations including
 *   the two.  You must obey the GNU General Public License in all
 *   respects for all of the code used other than OpenSSL.  If you modify
 *   this file, you may extend this exception to your version of the
 *   file, but you are not obligated to do so.  If you do not wish to
 *   do so, delete this exception statement from your version.
 */

#include "common.h"
#include "prototypes.h"

static FILE *outfile=NULL; /* Logging to file disabled by default */

#if defined (USE_WIN32) || defined (__vms)

/* HANDLE evt=NULL; */

void log_open(void) { /* Win32 version */
#if 0
    AllocConsole();
    /* reopen stdin handle as console window input */
    freopen("CONIN$", "rb", stdin);
    /* reopen stout handle as console window output */
    freopen("CONOUT$", "wb", stdout);
    /* reopen stderr handle as console window output */
    freopen("CONOUT$", "wb", stderr);
    printf("Close this window to exit stunnel\n\n");
#endif
    if(options.output_file)
        outfile=fopen(options.output_file, "a");
    if(outfile)
        return; /* It was possible to open a log file */
    /* TODO: Register NT EventLog source here */
    /* evt=RegisterEventSource(NULL, "stunnel"); */
    if(options.output_file)
        log(LOG_ERR, "Unable to open output file: %s", options.output_file);
}

void log_close(void) {
    if(outfile)
        fclose(outfile);
#if 0
    else
        FreeConsole();
#endif
}

#else /* USE_WIN32, __vms */

void log_open(void) { /* Unix version */
    int fd;

    if(options.output_file) { /* 'output' option specified */
        fd=open(options.output_file, O_CREAT|O_WRONLY|O_APPEND, 0640);
        if(fd>=0) { /* file opened or created */
            fcntl(fd, F_SETFD, FD_CLOEXEC);
            outfile=fdopen(fd, "a");
            if(outfile)
                return; /* no need to setup syslog */
        }
    }
    if(options.option.syslog) {
#ifdef __ultrix__
        openlog("stunnel", LOG_PID);
#else
        openlog("stunnel", LOG_CONS | LOG_NDELAY | LOG_PID, options.facility);
#endif /* __ultrix__ */
    }
    if(options.output_file)
        log(LOG_ERR, "Unable to open output file: %s", options.output_file);
}

void log_close(void) {
    if(outfile) {
        fclose(outfile);
        return;
    }
    if(options.option.syslog)
        closelog();
}

#endif /* USE_WIN32, __vms */

void log(int level, const char *format, ...) {
    va_list arglist;
    char text[STRLEN], timestamped[STRLEN];
    FILE *out;
    time_t gmt;
    struct tm *timeptr;
#ifdef HAVE_LOCALTIME_R
    struct tm timestruct;
#endif

    if(level>options.debug_level)
        return;
    va_start(arglist, format);
#ifdef HAVE_VSNPRINTF
    vsnprintf(text, STRLEN, format, arglist);
#else
    vsprintf(text, format, arglist);
#endif
    va_end(arglist);
#if !defined (USE_WIN32) && !defined (__vms)
    if(!outfile && options.option.syslog) {
        syslog(level, "%s", text);
        return;
    }
#endif /* USE_WIN32, __vms */
    out=outfile?outfile:stderr;
    time(&gmt);
#ifdef HAVE_LOCALTIME_R
    timeptr=localtime_r(&gmt, &timestruct);
#else
    timeptr=localtime(&gmt);
#endif
#ifdef HAVE_SNPRINTF
    snprintf(timestamped, STRLEN,
#else
    sprintf(timestamped,
#endif
        "%04d.%02d.%02d %02d:%02d:%02d LOG%d[%lu:%lu]: %s",
        timeptr->tm_year+1900, timeptr->tm_mon+1, timeptr->tm_mday,
        timeptr->tm_hour, timeptr->tm_min, timeptr->tm_sec,
        level, stunnel_process_id(), stunnel_thread_id(), text);
#ifdef USE_WIN32
    win_log(timestamped); /* Always log to the GUI window */
    if(outfile) /* to the file - only if it exists */
#endif
    {
        fprintf(out, "%s\n", timestamped);
        fflush(out);
    }
}

void log_raw(const char *format, ...) {
    va_list arglist;
    char text[STRLEN];
    FILE *out;

    va_start(arglist, format);
#ifdef HAVE_VSNPRINTF
    vsnprintf(text, STRLEN, format, arglist);
#else
    vsprintf(text, format, arglist);
#endif
    va_end(arglist);
    out=outfile?outfile:stderr;
#ifdef USE_WIN32
    win_log(text);
#else
    fprintf(out, "%s\n", text);
    fflush(out);
#endif
}

/* End of log.c */
