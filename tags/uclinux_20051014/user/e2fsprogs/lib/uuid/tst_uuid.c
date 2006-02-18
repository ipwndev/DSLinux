/*
 * tst_uuid.c --- test program from the UUID library
 *
 * Copyright (C) 1996, 1997, 1998 Theodore Ts'o.
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

#include <stdio.h>

#include "uuid.h"

int
main(int argc, char **argv)
{
	uuid_t		buf, tst;
	char		str[100];
	struct timeval	tv;
	time_t		time_reg;
	unsigned char	*cp;
	int i;
	int failed = 0;
	int type, variant;

	uuid_generate(buf);
	uuid_unparse(buf, str);
	printf("UUID generate = %s\n", str);
	printf("UUID: ");
	for (i=0, cp = (unsigned char *) &buf; i < 16; i++) {
		printf("%02x", *cp++);
	}
	printf("\n");
	type = uuid_type(buf); 	variant = uuid_variant(buf);
	printf("UUID type = %d, UUID variant = %d\n", type, variant);
	if (variant != UUID_VARIANT_DCE) {
		printf("Incorrect UUID Variant; was expecting DCE!\n");
		failed++;
	}
	printf("\n");

	uuid_generate_random(buf);
	uuid_unparse(buf, str);
	printf("UUID random string = %s\n", str);
	printf("UUID: ");
	for (i=0, cp = (unsigned char *) &buf; i < 16; i++) {
		printf("%02x", *cp++);
	}
	printf("\n");
	type = uuid_type(buf); 	variant = uuid_variant(buf);
	printf("UUID type = %d, UUID variant = %d\n", type, variant);
	if (variant != UUID_VARIANT_DCE) {
		printf("Incorrect UUID Variant; was expecting DCE!\n");
		failed++;
	}
	if (type != 4) {
		printf("Incorrect UUID type; was expecting "
		       "4 (random type)!\n");
		failed++;
	}
	printf("\n");
	
	uuid_generate_time(buf);
	uuid_unparse(buf, str);
	printf("UUID string = %s\n", str);
	printf("UUID time: ");
	for (i=0, cp = (unsigned char *) &buf; i < 16; i++) {
		printf("%02x", *cp++);
	}
	printf("\n");
	type = uuid_type(buf); 	variant = uuid_variant(buf);
	printf("UUID type = %d, UUID variant = %d\n", type, variant);
	if (variant != UUID_VARIANT_DCE) {
		printf("Incorrect UUID Variant; was expecting DCE!\n");
		failed++;
	}
	if (type != 1) {
		printf("Incorrect UUID type; was expecting "
		       "1 (time-based type)!\\n");
		failed++;
	}
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	time_reg = uuid_time(buf, &tv);
	printf("UUID time is: (%ld, %ld): %s\n", tv.tv_sec, tv.tv_usec,
	       ctime(&time_reg));
	uuid_parse(str, tst);
	if (!uuid_compare(buf, tst))
		printf("UUID parse and compare succeeded.\n");
	else {
		printf("UUID parse and compare failed!\n");
		failed++;
	}
	uuid_clear(tst);
	if (uuid_is_null(tst))
		printf("UUID clear and is null succeeded.\n");
	else {
		printf("UUID clear and is null failed!\n");
		failed++;
	}
	uuid_copy(buf, tst);
	if (!uuid_compare(buf, tst))
		printf("UUID copy and compare succeeded.\n");
	else {
		printf("UUID copy and compare failed!\n");
		failed++;
	}
	if (failed) {
		printf("%d failures.\n", failed);
		exit(1);
	}
	return 0;
}

	

