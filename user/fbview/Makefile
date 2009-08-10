LIBFLAGS = -lpng -lm -lz -ljpeg -lgpm
TARGET = fbview

all: fbview

fbview:
	${CC} ${TARGET}.c ${CFLAGS} ${LDFLAGS} ${LIBFLAGS} -o ${TARGET}

install:
	cp ${TARGET} /usr/bin/

clean:
	rm -f ${TARGET} ${TARGET}.gdb

romfs:
	$(ROMFSINST) fbview /usr/bin
.PHONY: all clean
