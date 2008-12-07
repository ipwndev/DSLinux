LIBFLAGS = -lpng -lm -lz
OBJS = bitmap.o framebuffer.o keyboard.o ds_png.o

all: fbview

fbview: ${OBJS}
	${CC} -o fbview ${CFLAGS} ${LDFLAGS} ${OBJS} ${LIBFLAGS} fbview.c

install:
	cp ${TARGET} /usr/bin/

clean:
	rm -f fbview fbview.gdb ${OBJS}

bitmap.o:
	${CC} -c -o bitmap.o ${CFLAGS} ${LDFLAGS} bitmap.c

ds_png.o:
	${CC} -c -o ds_png.o ${CFLAGS} ${LDFLAGS} ds_png.c

framebuffer.o:
	${CC} -c -o framebuffer.o ${CFLAGS} ${LDFLAGS} framebuffer.c

keyboard.o:
	${CC} -c -o keyboard.o ${CFLAGS} ${LDFLAGS} keyboard.c

romfs:
	$(ROMFSINST) fbview /usr/bin
.PHONY: all clean
