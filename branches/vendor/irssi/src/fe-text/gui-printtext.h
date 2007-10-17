#ifndef __GUI_PRINTTEXT_H
#define __GUI_PRINTTEXT_H

#include "gui-windows.h"
#include "textbuffer-view.h"
#include "formats.h"

extern int mirc_colors[];

void gui_printtext_init(void);
void gui_printtext_deinit(void);

void gui_register_indent_func(const char *name, INDENT_FUNC func);
void gui_unregister_indent_func(const char *name, INDENT_FUNC func);

void gui_set_default_indent(const char *name);
INDENT_FUNC get_default_indent_func(void);

void gui_printtext(int xpos, int ypos, const char *str);
void gui_printtext_after(TEXT_DEST_REC *dest, LINE_REC *prev, const char *str);

#endif
