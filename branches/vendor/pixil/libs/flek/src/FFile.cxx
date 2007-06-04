#include <Flek/FFile.H>
#include <FL/filename.H>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>		// realpath
#include <stdio.h>		// tmpnam
#include <sys/stat.h>

void
lo_to_hi(ulong * buffer, int len)
{
//  return;
    char *cbuffer = (char *) buffer;
    register ulong value;
    for (register int i = 0; i < len; i++, cbuffer += 4) {
	value = buffer[i];
	cbuffer[0] = ((value >> 24) & 0xff);
	cbuffer[1] = ((value >> 16) & 0xff);
	cbuffer[2] = ((value >> 8) & 0xff);
	cbuffer[3] = ((value & 0xff));
    }
}

void
FFile::open(char *filename, FFileMode mode)
{
    char *m = 0;

    switch (mode) {
    case FFileNull:
	m = "";
	break;
    case FFileRead:
	m = "r";
	break;
    case FFileReadPlus:
	m = "r+";
	break;
    case FFileWrite:
	m = "w";
	break;
    case FFileWritePlus:
	m = "w+";
	break;
    case FFileAppend:
	m = "a";
	break;
    case FFileAppendPlus:
	m = "a+";
	break;
    }

    Fd = fopen(filename, m);

    if (!Fd)
	Error = 1;
    else
	Error = 0;
}
