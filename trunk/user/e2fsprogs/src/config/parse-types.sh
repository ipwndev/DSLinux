#!/bin/sh

cat > sed.script << "EOF"
/^#/d
/^$/d
s/__extension__ //
s/typedef \(.*\) __u\([1-9]*\);/#define __U\2_TYPEDEF \1/
s/typedef \(.*\) __s\([1-9]*\);/#define __S\2_TYPEDEF \1/
EOF

echo '#include <asm/types.h>' | ${CPP-${CC-gcc} -E} - | \
    sed -f sed.script | grep ^# > asm_types.h

rm sed.script

cp asm_types.h asm_types.c

cat >> asm_types.c <<EOF
#include <stdio.h>
#include <stdlib.h>
main(int argc, char **argv)
{
#ifdef __U8_TYPEDEF
	if (sizeof(__U8_TYPEDEF) != 1) {
		printf("Sizeof(__U8__TYPEDEF) is %d should be 1\n", 
		       sizeof(__U8_TYPEDEF));
		exit(1);
	}
#else
#warning __U8_TYPEDEF not defined
#endif
#ifdef __S8_TYPEDEF
	if (sizeof(__S8_TYPEDEF) != 1) {
		printf("Sizeof(_S8__TYPEDEF) is %d should be 1\n", 
		       sizeof(__S8_TYPEDEF));
		exit(1);
	}
#else
#warning __S8_TYPEDEF not defined
#endif
#ifdef __U16_TYPEDEF
	if (sizeof(__U16_TYPEDEF) != 2) {
		printf("Sizeof(__U16__TYPEDEF) is %d should be 2\n", 
		       sizeof(__U16_TYPEDEF));
		exit(1);
	}
#else
#warning __U16_TYPEDEF not defined
#endif
#ifdef __S16_TYPEDEF
	if (sizeof(__S16_TYPEDEF) != 2) {
		printf("Sizeof(__S16__TYPEDEF) is %d should be 2\n", 
		       sizeof(__S16_TYPEDEF));
		exit(1);
	}
#else
#warning __S16_TYPEDEF not defined
#endif

#ifdef __U32_TYPEDEF
	if (sizeof(__U32_TYPEDEF) != 4) {
		printf("Sizeof(__U32__TYPEDEF) is %d should be 4\n", 
		       sizeof(__U32_TYPEDEF));
		exit(1);
	}
#else
#warning __U32_TYPEDEF not defined
#endif
#ifdef __S32_TYPEDEF
	if (sizeof(__S32_TYPEDEF) != 4) {
		printf("Sizeof(__S32__TYPEDEF) is %d should be 4\n", 
		       sizeof(__S32_TYPEDEF));
		exit(1);
	}
#else
#warning __S32_TYPEDEF not defined
#endif

#ifdef __U64_TYPEDEF
	if (sizeof(__U64_TYPEDEF) != 8) {
		printf("Sizeof(__U64__TYPEDEF) is %d should be 8\n", 
		       sizeof(__U64_TYPEDEF));
		exit(1);
	}
#else
#warning __U64_TYPEDEF not defined
#endif
#ifdef __S64_TYPEDEF
	if (sizeof(__S64_TYPEDEF) != 8) {
		printf("Sizeof(__S64__TYPEDEF) is %d should be 8\n", 
		       sizeof(__S64_TYPEDEF));
		exit(1);
	}
#else
#warning __S64_TYPEDEF not defined
#endif
	exit(0);
}
EOF

${BUILD_CC-${CC-gcc}} -o asm_types asm_types.c
if ! ./asm_types 
then
    echo "Problem detected with asm_types.h"
    echo "" > asm_types.h
fi
rm asm_types.c asm_types

