# Makefile for libid3tag in uClinux
# =================================

# Special compiler options
CFLAGS += -DHAVE_CONFIG_H

# Header files
INCS:= 	version.h ucs4.h latin1.h utf16.h utf8.h parse.h render.h \
	field.h frametype.h compat.h genre.h frame.h crc.h util.h \
	tag.h file.h id3tag.h global.h genre.dat debug.h config.h

# Object files
OBJS:=	version.o ucs4.o latin1.o utf16.o utf8.o parse.o render.o \
	field.o frametype.o compat.o genre.o frame.o crc.o util.o \
	tag.o file.o

.PHONY: all
all: libid3tag.a

libid3tag.a: $(OBJS) $(INCS)
	$(AR) rv $@ $(OBJS)
	$(RANLIB) $@

.PHONY: romfs
romfs:

.PHONY: clean
clean:
	-rm -f *.[oa] *~

.PHONY: distclean
distclean: clean
