
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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>

FILE *debug_fd;

FILE *
debug_init(void)
{
    debug_fd = fopen("DEBUG.txt", "w");
    return debug_fd;
}

int
debug_log(char *fmt, ...)
{
    char *ptr;
    va_list ap;

    va_start(ap, fmt);
    vasprintf(&ptr, fmt, ap);
    va_end(ap);
    fprintf(debug_fd, "%s\n", ptr);
    //write(debug_fd, ptr, strlen(ptr));
    fflush(debug_fd);
    free(ptr);
    return 0;
}

void
debug_destroy(void)
{
    fclose(debug_fd);
}
