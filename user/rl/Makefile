# Makefile to build rl for DSLinux
#
# Makefile by Cayenne Boyer

EXEC = rl
OBJS = rl.o alloc.o buffer.o random.o 

.PHONY: all romfs clean

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	rm -f $(EXEC) *.elf *.gdb *.o
