
/*
 * TMSNC - Textbased MSN Client Copyright (C) 2004 The IR Developer Group
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the IR Public Domain License as published by the IR Group;
 * either version 1.6 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the IR Public Domain License along with
 * this program; if not, write to sanoix@gmail.com.
 */

#include <string.h>
#include <stdlib.h>

enum list_show_mode {
	list_show_default,
	list_show_addr,
	list_show_nick,
	list_show_psm,
	list_show_END
};

int list_offset = 0, list_pointer = 0;
int conv_offset = 0, conv_pointer = 0;
int list_show = list_show_default; 
