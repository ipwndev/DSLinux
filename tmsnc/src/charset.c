
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

#include "charset.h"
#include "common.h"

#ifdef HAVE_ICONV
#include <iconv.h>

iconv_t from_cd;
iconv_t to_cd;
char *codeset = "ISO-8859-1";

int
iconv_init(cset)
   char *cset;
{
    if (cset[0] != '\0')
        codeset = cset;

    if ((from_cd = iconv_open(codeset, "UTF-8")) == (iconv_t) - 1)
        return -1;
    if ((to_cd = iconv_open("UTF-8", codeset)) == (iconv_t) - 1)
        return -1;

#ifdef DEBUG
    debug_log("Iconv codeset: %s\n", codeset);
#endif

    return 0;
}

void
iconv_destroy()
{
    iconv_close(from_cd);
    iconv_close(to_cd);
}

int
convert(cd, ibuf, isize, obuf, osize)
     iconv_t cd;
     char *ibuf;
     size_t isize;
     char *obuf;
     size_t osize;
{
    int rc = 0;
    char *iptr = ibuf, *optr = obuf;
    size_t oleft = osize, ileft = isize;

    if (strcmp(codeset, "UTF-8") == 0) {
        strncpy(obuf, ibuf, osize - 1);
        return 0;
    }
    memset(obuf, 0x0, osize);

    while (ileft > 0 && oleft > 0) {
        rc = iconv(cd, &iptr, &ileft, &optr, &oleft);
        if (rc == -1) {
            *iptr = '#';
            iptr++;
            ileft--;
        }
    }

    return rc;
}

int
convert_from_utf8(ibuf, isize, obuf, osize)
     char *ibuf;
     size_t isize;
     char *obuf;
     size_t osize;
{
    return convert(from_cd, ibuf, isize, obuf, osize);
}

int
convert_to_utf8(ibuf, isize, obuf, osize)
     char *ibuf;
     size_t isize;
     char *obuf;
     size_t osize;
{
    return convert(to_cd, ibuf, isize, obuf, osize);
}
#endif
