/* 
 * Print WFC settings stored in Nintendo DS firmware to stdout.
 *
 * (C)reated in 2006 by Stefan Sperling <stsp@stsp.in-berlin.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * Based on information snooped from source code of sgstair's wifi lib
 * (http://akkit.org/dswifi)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

typedef int bool;
#define true 1
#define false 0

#define WFC_SETTINGS 		0x03FA00
#define ESSID_OFFSET		0x40
#define WEP_OFFSET		0x80
#define IP_OFFSET		0xC0
#define GW_OFFSET		(IP_OFFSET + 4)
#define DNS1_OFFSET		(GW_OFFSET + 4)
#define DNS2_OFFSET		(DNS1_OFFSET + 4)
#define MASK_OFFSET		0xD0
#define WEP_MODE_OFFSET		0xE6
#define STATUS_OFFSET		0xE7

#define WEP_MODE_OFF		0
#define WEP_MODE_40BIT		1
#define WEP_MODE_128BIT		2

#define FIRMWARE_FILE "/dev/firmware"

#define DEFAULT_CONFIG 0
static int config = DEFAULT_CONFIG;
static FILE* f = NULL;

void usage()
{
	fprintf(stderr, "wfcdump - dump WFC settings from firmware\n");
	fprintf(stderr, "Usage: wfcdump [options]\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-c n\tnumber of WFC configuration to use "
			"(1, 2 or 3).\n");
	fprintf(stderr, "\t        Default is %i.\n", DEFAULT_CONFIG + 1);
	fprintf(stderr, "\t-h\tprint help\n");
}

void die()
{
	if (f != NULL)
		fclose(f);
	perror("wfcdump");
	exit(1);
}

/* Print n in dotted quad notation. If reverse is set,
 * print bytes in reverse order. */
void print_dotquad(unsigned long n, bool reverse)
{
	if (reverse)
		printf("%lu.%lu.%lu.%lu", (n >> 24) & 0xff, (n >> 16) & 0xff,
				(n >> 8) & 0xff, n & 0xff);
	else
		printf("%lu.%lu.%lu.%lu", n & 0xff, (n >> 8) & 0xff,
				(n >> 16) & 0xff, (n >> 24) & 0xff);
}

unsigned long read_ulong(int offset)
{
	int base = WFC_SETTINGS + ((config) << 8);
	if (fseek(f, base + offset, SEEK_SET) != 0)
		die();
	unsigned long out = 0;
	if (fread(&out, sizeof(out), 1, f) < 1)
		die();
	return out;
}

void print_ip_item(int offset)
{
	unsigned long item = read_ulong(offset);
	if (item != 0)
		print_dotquad(item, false);
}

unsigned long calc_mask(unsigned long bits)
{
	int i;
	unsigned long mask = 0;
	for (i = 0; i < bits; i++)
		mask |= 1 << (31 - i);
	return mask;
}

void print_mask()
{
	unsigned long bits = read_ulong(MASK_OFFSET);
	if (bits != 0)
		print_dotquad(calc_mask(bits), true);
}

void print_essid()
{
	char essid[32];
	int base, i;

	for (i = 0; i < sizeof(essid); i++)
		essid[i] = '\0';

	base = WFC_SETTINGS + ((config) << 8);
	if (fseek(f, base + ESSID_OFFSET, SEEK_SET) != 0)
		die();
	if (fgets(essid, sizeof(essid), f) == NULL)
		die();
	putchar('"');
	for ( i = 0 ; essid[i] ; i++ )
	{
	    if ( essid[i] == '\\' )
		printf("\\\\");
	    else if ( essid[i] == '\"' )
		printf("\\\"");
	    else
		putchar(essid[i]);
	}
	putchar('"');
}

void print_wep()
{
	int base, i, mode, len;

	base = WFC_SETTINGS + ((config) << 8);
	if (fseek(f, base + WEP_MODE_OFFSET, SEEK_SET) != 0)
		die();

	len = 0;
	mode = (fgetc(f) & 0x0F);
	
	switch (mode) {
	case WEP_MODE_OFF:
		return;
	case WEP_MODE_40BIT:
		len = 5;
		break;
	case WEP_MODE_128BIT:
		len = 13; /* 128 bit is actually 104 bit */
		break;
	default:
		fprintf(stderr, "wfcdump: unknown wep mode %i read from "
				"firmware\n", mode);
		errno = EINVAL;
		die();
	}

	if (fseek(f, base + WEP_OFFSET, SEEK_SET) != 0)
		die();
	putchar('"');
	for (i = 0; i < len; i++)
		printf("%02x", fgetc(f));
	putchar('"');
}


int main(int argc, char *argv[])
{
	int ch;

	while ((ch = getopt(argc, argv, "c:h")) != -1) {
		switch (ch) {
		case 'c':
			config = atoi(optarg) - 1;
			if (config < 0 || config > 2) {
				fprintf(stderr, "wfcdump: Invalid WFC "
					"configuration number %i\n",
					config + 1);
				exit(1);
			}
			break;
		case 'h':
			usage();
			exit(0);
		default:
			usage();
			exit(1);
		}
	}

	if ((f = fopen(FIRMWARE_FILE, "r")) == NULL) {
		perror("wfcdump: " FIRMWARE_FILE);
		exit(1);
	}

	if (read_ulong(STATUS_OFFSET) != 0) {
		fprintf(stderr, "wfcdump: configuration %i not configured\n",
				config + 1);
		exit(1);
	}
	
	printf("essid=");
	print_essid();
	printf("\nwepkey=");
	print_wep();
	printf("\nip=");
	print_ip_item(IP_OFFSET);
	printf("\ngateway=");
	print_ip_item(GW_OFFSET);
	printf("\ndns1=");
	print_ip_item(DNS1_OFFSET);
	printf("\ndns2=");
	print_ip_item(DNS2_OFFSET);
	printf("\nnetmask=");
	print_mask();
	printf("\n");

	if (f != NULL)
		fclose(f);
	exit(0);
}

