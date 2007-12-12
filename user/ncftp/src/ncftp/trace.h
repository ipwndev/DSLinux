/* trace.h
 *
 * Copyright (c) 1992-2004 by Mike Gleason.
 * All rights reserved.
 * 
 */

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	define kTraceFileName				"trace.txt"
#else
#	define kTraceFileName				"trace"
#endif

/* trace.c */
void Trace(const int, const char *const, ...)
#if (defined(__GNUC__)) && (__GNUC__ >= 2)
__attribute__ ((format (printf, 2, 3)))
#endif
;
void ErrorHook(const FTPCIPtr UNUSED(cipUnused), char *msg);
void DebugHook(const FTPCIPtr, char *);
void SetDebug(int);
void UseTrace(void);
void OpenTrace(void);
void CloseTrace(void);
