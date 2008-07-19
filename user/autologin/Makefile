# Makefile for autologin

EXEC = autologin
OBJS = autologin.o
INCS = config.h

# declare these targets as phony, so make will not try to look
# for files named like them:
.PHONY: all romfs clean

# 'all' is a phony target. It is called when the build process
# enters this directory and wants to build the program.
all: $(EXEC)

# This is the rule that build the executable.
$(EXEC): $(OBJS) $(INCS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS_$@))

# The romfs target is also phony. It is used to copy your app
# into the DSLinux filesystem image.
romfs:
	$(ROMFSINST) /bin/$(EXEC)
	$(ROMFSINST) /etc/autologin.conf

# clean is another phony target that removes every file generated
# during the build.
clean:
	rm -f $(EXEC) *.elf *.gdb *.o
