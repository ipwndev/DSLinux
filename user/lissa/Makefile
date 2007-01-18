
EXEC = lissa
OBJS = lissa.o

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBM) $(LDLIBS) $(LIBGCC)

romfs:
	$(ROMFSINST) /usr/games/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
