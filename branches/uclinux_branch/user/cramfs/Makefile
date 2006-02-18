
PROGS = mkcramfs cramfsck

all: $(PROGS)

host_build:
	make -C host_build

mkcramfs: mkcramfs.o
	$(CC) $(LDFLAGS) -o $@ mkcramfs.o -lz $(LDLIBS$(LDLIBS_$@))

cramfsck: cramfsck.o
	$(CC) $(LDFLAGS) -o $@ cramfsck.o -lz $(LDLIBS$(LDLIBS_$@))

romfs:
	$(ROMFSINST) -e CONFIG_USER_CRAMFS_MKCRAMFS /bin/mkcramfs
	$(ROMFSINST) -e CONFIG_USER_CRAMFS_CRAMFSCK /bin/cramfsck

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
	make -C host_build clean

.PHONY:	host_build
