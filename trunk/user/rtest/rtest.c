#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#define TESTADR ((volatile unsigned char *) 0x08000000)
#define TESTLEN 0x1000000;

int main(int argc, char **argv)
{
	volatile unsigned char *p;
	int i;

	printf("RTEST RAM TEST Program\n");


	p = TESTADR;
	i = TESTLEN;

	for ( ;i;i--) {
		*p++ = (unsigned char) i;
	}

	printf("Write done\n");

	p = TESTADR;
	i = TESTLEN;

	for ( ;i;i--) {
		if (*p == (unsigned char)i)
			;
		else {
			printf("Error: Adr %X, expect %X, got %X\n", (unsigned int) p, i & 0xFF, (int)*p);
		}
		p++;
	}
	printf("Read done\n");

	return 0;
}

