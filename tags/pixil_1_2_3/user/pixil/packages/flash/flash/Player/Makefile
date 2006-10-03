FLASH=../Lib
CPPFLAGS=-I$(FLASH) -I$(GUIINC)
CFLAGS=$(OPTIMIZE) $(CPPFLAGS) $(ALLCFLAGS)

LIBS = $(FLASH)/libflash.a $(Z_LIB) $(JPEG_LIB) $(GUILIBS) -lm
LDFLAGS=

swfplayer: $(GUIAPP) $(FLASH)/libflash.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(GUIAPP) $(LIBS)
	cp ./swfplayer ../nxflashplay

clean:
	rm -f *~ *.o swfplayer core gmon.out ../nxflashplay


