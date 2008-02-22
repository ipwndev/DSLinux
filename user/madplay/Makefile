# uClinux Makefile for madplay
# ============================

# extra compiler options from original makefile
CFLAGS += -DHAVE_CONFIG_H 
CFLAGS += -Wall -O -fforce-mem -fforce-addr -fthread-jumps
CFLAGS += -fcse-follow-jumps -fcse-skip-blocks -fexpensive-optimizations
CFLAGS += -fregmove -fschedule-insns2 -fstrength-reduce -fomit-frame-pointer
CFLAGS += -funroll-loops -finline-functions

# name of the executable
EXEC = 	madplay

# object files needed for executable
OBJS = 	madplay.o audio.o audio_aiff.o audio_cdda.o audio_hex.o audio_null.o \
	audio_raw.o audio_snd.o audio_wave.o audio_oss.o \
	getopt.o getopt1.o version.o resample.o filter.o tag.o crc.o rgain.o  \
	player.o

# includes
INCS = 	audio.h getopt.h global.h gettext.h version.h resample.h filter.h tag.h \
 	crc.h rgain.h player.h config.h

# libraries to link with
LDLIBS += -lid3tag -lmad -lz -lm

# declare these targets as phony, so make will not try to look
# for files named like them:
.PHONY: all romfs clean

# 'all' is a phony target. It is called when the build process
# enters this directory and wants to build the program.
all: $(EXEC)

# This is the rule that build the mad player.
$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

# The romfs target is also phony. It is used to copy your app
# into the DSLinux filesystem image.
romfs:
	$(ROMFSINST) /bin/$(EXEC)
	$(ROMFSINST) -m madplay.1 /usr/share/man/madplay

# clean is another phony target that removes every file generated
# during the build.
clean:
	rm -f $(EXEC) *.elf *.gdb *.o 
