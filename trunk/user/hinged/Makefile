# Makefile for hinged

EXEC = hinged
OBJS = hinged.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS_$@))

romfs:
	$(ROMFSINST) /bin/$(EXEC)
	$(ROMFSINST) -e CONFIG_USER_HINGED_EXAMPLE data /usr/share/hinged/
	$(ROMFSINST) -e CONFIG_USER_HINGED_EXAMPLE README /usr/share/hinged/
	$(ROMFSINST) -e CONFIG_USER_HINGED_EXAMPLE lidopen /usr/bin
	$(ROMFSINST) -e CONFIG_USER_HINGED_EXAMPLE lidclose /usr/bin

clean:
	rm -f $(EXEC) *.elf *.gdb *.o
