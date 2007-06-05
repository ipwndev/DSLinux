// Headers

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <stdarg.h>

// Constants

const char *FILE_HEADER = "!!8-Bit!!";

// Why aren't these in a standard header?
#define FALSE 0
#define TRUE 1

// Globals
#ifndef PATH_MAX
	#define PATH_MAX	256
#endif

#define BUFSIZE 65536

void decode(FILE *inFile);

void doFile(char *inName)
{
	FILE *in;
//	int errno;

	if (!(in = fopen(inName, "rb")))
		fprintf(stderr,"ERROR: Can't open %s - %s", inName, strerror(errno));

	decode(in);

	fclose(in);
}

void decode(FILE *inFile)
{
	unsigned char *dict[256], *inBuf, *outBuf, *in, *out, *inEnd, *outEnd;
	int c, i, dictLen[256];

// Check header

	inBuf = (char *) malloc(BUFSIZE);
	if (!inBuf)
		fprintf(stderr,"ERROR: Not enough memory for input buffer");
	outBuf = (char *) malloc(BUFSIZE);
	if (!outBuf)
		fprintf(stderr,"ERROR: Not enough memory for input buffer");

	errno = 0;

	fread(inBuf, 1, strlen(FILE_HEADER), inFile);
	if (errno)
		fprintf(stderr,"ERROR: Can't read header - %s", strerror(errno));

	if (strncmp(inBuf, FILE_HEADER, strlen(FILE_HEADER)))
		fprintf(stderr,"ERROR: Not a TCR compressed file");

// Read dictionary

	for (i = 0; i < 256; ++i)
	{	if ((dictLen[i] = getc(inFile)) < 0)
			fprintf(stderr,"ERROR: Can't read entry %i length - %s", i, strerror(errno));
		if (dictLen[i] > 255)
			fprintf(stderr,"ERROR: Invalid entry %i length %i - %s", i, dictLen[i], strerror(errno));
		if (!(dict[i] = (char *) malloc(dictLen[i])))
			fprintf(stderr,"ERROR: Out of memory for entry %i", i);
		if (fread(dict[i], 1, dictLen[i], inFile) < dictLen[i])
			if (errno)
				fprintf(stderr,"ERROR: Can't read entry %i - %s", i, strerror(errno));
			else
				fprintf(stderr,"ERROR: Malformed TCR file - too short");
	}

// Decode file

	in = inEnd = inBuf;
	out = outBuf;
	outEnd = outBuf + BUFSIZE;

	for (;;)
	{	if (in >= inEnd)
		{	int inLen;
			inLen = fread(inBuf, 1, BUFSIZE, inFile);
			if (inLen < 0)
				fprintf(stderr,"ERROR: Can't read - %s", strerror(errno));
			else if (inLen == 0)
				break;
			in = inBuf;
			inEnd = inBuf + inLen;
		}

		c = *in++;
		if (dictLen[c] == 0)
		{	fprintf(stderr,"WARNING: unused code %i found...\r\n", c);
			continue;
		}

		if (out + dictLen[c] > outEnd)
		{	if (fwrite(outBuf, 1, out - outBuf, stdout) < out - outBuf)
				fprintf(stderr,"ERROR: Can't write - %s", strerror(errno));
			out = outBuf;
		}
		memcpy(out, dict[c], dictLen[c]);
		out += dictLen[c];
	}

	if (fwrite(outBuf, 1, out - outBuf, stdout) < out - outBuf)
		fprintf(stderr,"ERROR: Can't write - %s", strerror(errno));

// Free dictionary

	for (i = 255; i >= 0; --i)
		free(dict[i]);
	free(outBuf);
	free(inBuf);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Process arg as filename

int main(int argc, char *argv[])
{
	int diserror=0;

	if (argc >= 1)
		diserror=1;
	

	if(diserror < 1)
	{
		fprintf(stderr, "tcrdecomp <tcr-file> \n - Decompresses <tcr-file>\n");
		exit(0);
	}


	while (--argc)
			doFile(*++argv);
	exit(0);
}
