CXXSOURCES = \
	main.C \
	xautolock.C \
	saver.C \
	passwd.C \
	xlock.C \

CSOURCES = \
	xs_colors.c \
	xs_hsv.c \
	xs_visual.c

TARGET = libsvrcmn.a

OBJECTS = $(CSOURCES:.c=.o) $(CXXSOURCES:.C=.o)
CFG = -DHAVE_SETEUID -DHAVE_UNISTD_H -DHAVE_SYSLOG_H -DHAVE_CRYPT_H -DHAVE_PAM -DKDE_PAM_SERVICE -DHAVE_SHADOW -DHAVE_XPM

CXXFLAGS = $(CFG) -I/usr/include/qt
RANLIB = ranlib

# ok, all variables should be exported from top-level makefile

all: $(TARGET)

$(TARGET): $(OBJECTS)
	-rm -f $(TARGET)
	$(AR) rc $(TARGET) $(OBJECTS)
	$(RANLIB) $(TARGET)

clean:
	-rm -f core $(OBJECTS) *.moc
	-rm -f $(TARGET)

%.o: %.c
	$(CC) $(FLAGS) $(CFLAGS) $(INCS) -c $<

%.o: %.C
	$(CXX) $(FLAGS) $(CXXFLAGS) $(INCS) -c $<

MOC = moc

%.moc: %.h
	$(MOC) $*.h -o $*.moc

# dependencies
main.moc: main.h
main.o: main.moc main.h
saver.moc: saver.h
saver.o: saver.moc saver.h

xautolock.o: xautolock.h
xlock.o: xlock.h
passwd.o: xlockmore.h
xlockmore.h: mode.h
xs_colors.o: xs_colors.h
xs_hsv.o: xs_hsv.h
xs_visual.o: xs_visual.h
