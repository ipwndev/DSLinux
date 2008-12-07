CC = ucfront-gcc arm-linux-elf-gcc
CFLAGS = -O2 -g -fomit-frame-pointer -fno-common -fno-builtin -Wall  -mswp-byte-writes -DCONFIG_NDS_ROM8BIT -mcpu=arm946e-s -mfpu=fpe3 -DEMBED -D__PIC__ -fpic -msingle-pic-base -Dlinux -D__linux__ -Dunix -D__uClinux__ -fpack-struct=2
LIBFLAGS = -lpng -lm -lz
LD = arm-linux-elf-ld
LDFLAGS = -Wl,-elf2flt -D__PIC__ -fpic -msingle-pic-base -mswp-byte-writes
OBJS = bitmap.o framebuffer.o keyboard.o ds_png.o

all: fbview

fbview: ${OBJS}
	${CC} -o fbview ${CFLAGS} ${LDFLAGS} ${OBJS} ${LIBFLAGS} fbview.c

install:
	cp ${TARGET} /usr/bin/

clean:
	rm -f fbview ${OBJS}

bitmap.o:
	${CC} -c -o bitmap.o ${CFLAGS} ${LDFLAGS} bitmap.c

ds_png.o:
	${CC} -c -o ds_png.o ${CFLAGS} ${LDFLAGS} ds_png.c

framebuffer.o:
	${CC} -c -o framebuffer.o ${CFLAGS} ${LDFLAGS} framebuffer.c

keyboard.o:
	${CC} -c -o keyboard.o ${CFLAGS} ${LDFLAGS} keyboard.c

.PHONY: all clean
