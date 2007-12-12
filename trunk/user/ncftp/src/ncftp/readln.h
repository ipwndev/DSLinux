/* readln.h
 *
 * Copyright (c) 1992-2004 by Mike Gleason.
 * All rights reserved.
 * 
 */

#if (defined(WIN32) || defined(_WINDOWS)) && !defined(__CYGWIN__)
#	define kHistoryFileName "history.txt"
#else
#	define kHistoryFileName "history"
#endif

/* readln.c */
void GetScreenColumns(void);
void InitTermcap(void);
void InitReadline(void);
void ReCacheBookmarks(void);
char *Readline(char *);
void AddHistory(const char *const line);
void PrintStartupBanner(void);
void SetXtermTitle(const char *const fmt, ...)
#if (defined(__GNUC__)) && (__GNUC__ >= 2)
__attribute__ ((format (printf, 1, 2)))
#endif
;
void MakePrompt(char *, size_t);
void SaveHistory(void);
void LoadHistory(void);
void DisposeReadline(void);
