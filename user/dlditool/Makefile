# Makefile for dlditool

EXEC = dlditool
OBJS = source/dlditool.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS_$@))
	arm-linux-elf-flthdr -s 8192 $@
romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o
