#ifndef __PRINTTEXT_H
#define __PRINTTEXT_H

#include "fe-windows.h"
#include "formats.h"

void printformat_module(const char *module, void *server, const char *target, int level, int formatnum, ...);
void printformat_module_window(const char *module, WINDOW_REC *window, int level, int formatnum, ...);
void printformat_module_dest(const char *module, TEXT_DEST_REC *dest, int formatnum, ...);

void printformat_module_args(const char *module, void *server, const char *target, int level, int formatnum, va_list va);
void printformat_module_window_args(const char *module, WINDOW_REC *window, int level, int formatnum, va_list va);
void printformat_module_dest_args(const char *module, TEXT_DEST_REC *dest, int formatnum, va_list va);

void printtext(void *server, const char *target, int level, const char *text, ...);
void printtext_string(void *server, const char *target, int level, const char *text);
void printtext_string_window(WINDOW_REC *window, int level, const char *text);
void printtext_window(WINDOW_REC *window, int level, const char *text, ...);
void printtext_multiline(void *server, const char *target, int level, const char *format, const char *text);
void printtext_dest(TEXT_DEST_REC *dest, const char *text, ...);

/* only GUI should call these - used for printing text to somewhere else
   than windows */
void printtext_gui(const char *text);
void printformat_module_gui(const char *module, int formatnum, ...);
void printformat_module_gui_args(const char *module, int formatnum, va_list va);

void printtext_init(void);
void printtext_deinit(void);

/* printformat(...) = printformat_format(MODULE_NAME, ...)

   Could this be any harder? :) With GNU C compiler and C99 compilers,
   use #define. With others use either inline functions if they are
   supported or static functions if they are not..
 */
#if defined (__GNUC__) && !defined (__STRICT_ANSI__)
/* GCC */
#  define printformat(server, target, level, formatnum...) \
	printformat_module(MODULE_NAME, server, target, level, ##formatnum)
#  define printformat_window(window, level, formatnum...) \
	printformat_module_window(MODULE_NAME, window, level, ##formatnum)
#  define printformat_dest(dest, formatnum...) \
	printformat_module_dest(MODULE_NAME, dest, ##formatnum)
#  define printformat_gui(formatnum...) \
	printformat_module_gui(MODULE_NAME, ##formatnum)
#elif defined (_ISOC99_SOURCE)
/* C99 */
#  define printformat(server, target, level, formatnum, ...) \
	printformat_module(MODULE_NAME, server, target, level, formatnum, __VA_ARGS__)
#  define printformat_window(window, level, formatnum, ...) \
	printformat_module_window(MODULE_NAME, window, level, formatnum, __VA_ARGS__)
#  define printformat_dest(dest, formatnum, ...) \
	printformat_module_dest(MODULE_NAME, dest, formatnum, __VA_ARGS__)
#  define printformat_gui(formatnum, ...) \
	printformat_module_gui(MODULE_NAME, formatnum, __VA_ARGS__)
#else
/* inline/static */
#ifdef G_CAN_INLINE
G_INLINE_FUNC
#else
static
#endif
void printformat(void *server, const char *target, int level, int formatnum, ...)
{
	va_list va;

	va_start(va, formatnum);
	printformat_module_args(MODULE_NAME, server, target, level, formatnum, va);
	va_end(va);
}

#ifdef G_CAN_INLINE
G_INLINE_FUNC
#else
static
#endif
void printformat_window(WINDOW_REC *window, int level, int formatnum, ...)
{
	va_list va;

	va_start(va, formatnum);
	printformat_module_window_args(MODULE_NAME, window, level, formatnum, va);
	va_end(va);
}

#ifdef G_CAN_INLINE
G_INLINE_FUNC
#else
static
#endif
void printformat_dest(TEXT_DEST_REC *dest, int formatnum, ...)
{
	va_list va;

	va_start(va, formatnum);
	printformat_module_dest_args(MODULE_NAME, dest, formatnum, va);
	va_end(va);
}

#ifdef G_CAN_INLINE
G_INLINE_FUNC
#else
static
#endif
void printformat_gui(int formatnum, ...)
{
	va_list va;

	va_start(va, formatnum);
	printformat_module_gui_args(MODULE_NAME, formatnum, va);
	va_end(va);
}
#endif

#endif
