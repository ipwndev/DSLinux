#Simple Makefile for LCalc
#Modified by Dualscreenman to compile for DSLinux
#(removed forced definition of CC, added $(CC) and $(LDFLAGS) to the compile string.

EXEC = lcalc


$(EXEC): Lcalc.c
		$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXEC) Lcalc.c

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	rm -f $(EXEC) $(EXEC).gdb
