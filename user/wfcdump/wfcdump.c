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

typedef int bool;
#define true 1
#define false 0

#define WFC_SETTINGS 		0x03FA00
#define SSID_OFFSET		0x40
#define WEP_OFFSET		0x80
#define IP_OFFSET		0xC0
#define GW_OFFSET		(IP_OFFSET + 4)
#define DNS1_OFFSET		(GW_OFFSET + 4)
#define DNS2_OFFSET		(DNS1_OFFSET + 4)
#define MASK_OFFSET		0xD0
#define WEP_MODE_OFFSET		0xE6
#define STATUS_OFFSET		0xE7

#define FIRMWARE_FILE "/dev/firmware"

#define DEFAULT_CONFIG 0
static int config = DEFAULT_CONFIG;
static FILE* f = NULL;

void usage()
{
	fprintf(stderr, "wfcdump - dump WFC settings from firmware\n");
	fprintf(stderr, "Usage: wfcdump [options] <items to print>\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-c n\tnumber of WFC configuration to use "
			"(1, 2 or 3).\n");
	fprintf(stderr, "\t        Default is %i.\n", DEFAULT_CONFIG + 1);
	fprintf(stderr, "\t-h\tprint help\n");
	fprintf(stderr, "Available items:\n");
	fprintf(stderr, "\tssid\tprint SSID\n");
	fprintf(stderr, "\twep\tprint WEP key\n");
	fprintf(stderr, "\tip\tprint IP address\n");
	fprintf(stderr, "\tgw\tprint gateway\n");
	fprintf(stderr, "\tdns1\tprint first DNS server\n");
	fprintf(stderr, "\tdns2\tprint second DNS server\n");
	fprintf(stderr, "\tmask\tprint netmask\n");
}

void die()
{
	if (f != NULL)
		fclose(f);
	perror("wfcdump");
	exit(1);
}

/* Prints n in quad dotted notation. If reverse is set,
 * reverse byte order before printing. */
void print_dotquad(unsigned long n, bool reverse)
{
	if (reverse)
		printf("%lu.%lu.%lu.%lu", (n >> 24) & 0xff, (n >> 16) & 0xff, (n >> 8) & 0xff,
				n & 0xff);
	else
		printf("%lu.%lu.%lu.%lu", n & 0xff, (n >> 8) & 0xff, (n >> 16) & 0xff,
				(n >> 24) & 0xff);
}

unsigned long read_ulong(int offset)
{
	int base = WFC_SETTINGS + ((config) << 8);
	if (fseek(f, base + offset, SEEK_SET) != 0)
		die();
	int out = 0;
	if (fread(&out, sizeof(out), 1, f) < 1)
		die();
	return out;
}

void print_ulong_item(int offset, char *name)
{
	unsigned long item = read_ulong(offset);
	print_dotquad(item, false);
	printf("\n");
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
	print_dotquad(calc_mask(bits), true);
	printf("\n");
}

void print_ssid()
{
	char ssid[32];
	int base, i;

	for (i = 0; i < sizeof(ssid); i++)
		ssid[i] = '\0';

	base = WFC_SETTINGS + ((config) << 8);
	if (fseek(f, base + SSID_OFFSET, SEEK_SET) != 0)
		die();
	if (fgets(ssid, sizeof(ssid), f) == NULL)
		die();
	printf("%s\n", ssid);
}

void print_wep()
{
	int base, i;

	base = WFC_SETTINGS + ((config) << 8);
	if (fseek(f, base + WEP_OFFSET, SEEK_SET) != 0)
		die();
	for (i = 0; i < 16; i++)
		printf("%02x", fgetc(f));
	printf("\n");
}


int main(int argc, char *argv[])
{
	int ch, i;

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

	if (argc <= optind)
		usage();

	if ((f = fopen(FIRMWARE_FILE, "r")) == NULL) {
		perror("wfcdump: " FIRMWARE_FILE);
		exit(1);
	}
	
	for (i = optind; i < argc; i++) {
		if (strcmp(argv[i], "ssid") == 0)
			print_ssid();
		else if (strcmp(argv[i], "wep") == 0)
			print_wep();
		else if (strcmp(argv[i], "ip") == 0)
			print_ulong_item(IP_OFFSET, "ip");
		else if (strcmp(argv[i], "gw") == 0)
			print_ulong_item(GW_OFFSET, "gw");
		else if (strcmp(argv[i], "dns1") == 0)
			print_ulong_item(DNS1_OFFSET, "dns1");
		else if (strcmp(argv[i], "dns2") == 0)
			print_ulong_item(DNS2_OFFSET, "dns2");
		else if (strcmp(argv[i], "mask") == 0)
			print_mask();
		else {
			fprintf(stderr, "Unknown item %s\n", argv[i]);
			exit(1);
		}
	}

	if (f != NULL)
		fclose(f);
	exit(0);
}

