MICROWIN_DIR=/user/projects/microwin-0.89.devel/src

CFLAGS=-O0 -g -I$(MICROWIN_DIR)/include -I.. 

all: version.so date.so loadmon.so

%.so: %.o
	gcc -shared -o $@ $<

clean:
	rm -f *.so *.o

.SUFFIXES:  .c .o

c.o: 
	$(CC) -c $(CFLAGS) -o $@ $<


