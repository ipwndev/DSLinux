# This Makefile and dlg_config.h is basically equivalent to:
# ./configure --disable-included-msgs --disable-rc-file --disable-Xdialog --disable-gauge --disable-tailbox

EXTRAOBJS =  formbox.o # guage.o tailbox.o rc.o # also edit dlg_config.h
OBJECTS = $(EXTRAOBJS) \
	arrows.o \
	buttons.o \
	checklist.o \
	dlg_keys.o \
	inputbox.o \
	inputstr.o \
	menubox.o \
	mouse.o \
	mousewget.o \
	msgbox.o \
	textbox.o \
	ui_getc.o \
	util.o \
	version.o \
	yesno.o

CFLAGS += -I. -I$(ROOTDIR)/lib/libncurses/include
LIBS = -lncurses -lm

LIB = libdialog.a
PROG = dialog

.PHONY: all romfs clean


all: $(LIB) $(PROG)

dialog.o $(OBJECTS): dialog.h dlg_keys.h dlg_config.h VERSION

$(LIB): $(OBJECTS)
	$(AR) -cr $(LIB) $(OBJECTS)
	$(RANLIB) $@

$(PROG): $(LIB) dialog.o
	$(CC) -o $@ dialog.o -L. -ldialog $(LDFLAGS) $(LIBS)

clean:
	rm -f *.o $(LIB) $(PROG) *.gdb *~

romfs:
	$(ROMFSINST) /bin/$(PROG)
	$(ROMFSINST) -m dialog.1 /usr/share/man/dialog
