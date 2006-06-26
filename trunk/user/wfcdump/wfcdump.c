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

#define WFC_SETTINGS 		0x03FA00
#define SSID_OFFSET		0x40
#define WEP_KEY_OFFSET		0x80
#define IP_SETTINGS_OFFSET	0xC0
#define	NETMASK_OFFSET		0xD0

#define FIRMWARE_FILE "/dev/firmware"
//#define FIRMWARE_FILE "./fw.bin"

int main(int argc, char *argv[])
{
	FILE *f;
	char ssid[32];
	int i, j, offset;
	unsigned long k, n;

	f = fopen(FIRMWARE_FILE, "r");
	if (f == NULL) {
		perror("wfcdump: " FIRMWARE_FILE);
		return 1;
	}

	for (i = 0; i < 3; i++) {
		offset = WFC_SETTINGS + (i << 8);

		printf("WFC Configuration %i:\n", i);

		if (fseek(f, offset + SSID_OFFSET, SEEK_SET) != 0)
			goto out;
		for (j = 0; j < sizeof(ssid); j++)
			ssid[j] = '\0';
		if (fgets(ssid, sizeof(ssid), f) == NULL)
			goto out;
		printf("[%i]ssid\t%s\n", i, ssid);

		if (fseek(f, offset + WEP_KEY_OFFSET, SEEK_SET) != 0)
			goto out;
		printf("[%i]wep\t", i);
		for (j = 0; j < 16; j++)
			printf("%x", fgetc(f));
		printf("\n");

		if (fseek(f, offset + IP_SETTINGS_OFFSET, SEEK_SET) != 0)
			goto out;
		
		if (fread(&k, sizeof(k), 1, f) < 1)
			goto out;
		printf("[%i]ip\t%lu.%lu.%lu.%lu\n", i, k & 0xff,
				(k >> 8) & 0xff, (k >> 16) & 0xff,
				(k >> 24) & 0xff);
		
		if (fread(&k, sizeof(k), 1, f) < 1)
			goto out;
		printf("[%i]gw\t%lu.%lu.%lu.%lu\n", i, k & 0xff,
				(k >> 8) & 0xff, (k >> 16) & 0xff,
				(k >> 24) & 0xff);
		
		if (fread(&k, sizeof(k), 1, f) < 1)
			goto out;
		printf("[%i]dns1\t%lu.%lu.%lu.%lu\n", i, k & 0xff,
				(k >> 8) & 0xff, (k >> 16) & 0xff,
				(k >> 24) & 0xff);
		
		if (fread(&k, sizeof(k), 1, f) < 1)
			goto out;
		printf("[%i]dns2\t%lu.%lu.%lu.%lu\n", i, k & 0xff,
				(k >> 8) & 0xff, (k >> 16) & 0xff,
				(k >> 24) & 0xff);

		if (fseek(f, offset + NETMASK_OFFSET, SEEK_SET) != 0)
			goto out;

		/* read number of bits set in netmask */
		if (fread(&k, sizeof(k), 1, f) < 1)
			goto out;

		/* construct netmask */
		n = 0;
		for (j = 0; j < k; j++)
			n |= 1 << (31 - j);

		printf("[%i]mask\t%lu.%lu.%lu.%lu\n", i, (n >> 24) & 0xff,
				(n >> 16) & 0xff, (n >> 8) & 0xff,
				n & 0xff);

		(i == 2) ? : printf("\n");
	}

	fclose(f);
	return 0;
out:
	perror("wfcdump");
	fclose(f);
	return 1;
}

