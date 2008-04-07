# Makefile for SDL_Mixer

all: .compiled

SDL_MIXER_CONFIGURE_FLAGS=	--disable-music-ogg-shared --disable-music-mp3 \
				--disable-smpegtest --disable-music-mp3-shared \
				--enable-music-ogg-tremor --disable-shared --disable-sdltest \
				--with-sdl-exec-prefix=$(ROOTDIR)/lib/sdl/src

.configured:
	cd src && env CFLAGS="$(CFLAGS) -I$(ROOTDIR)/include" LDFLAGS="$(LDFLAGS) -L$(ROOTDIR)/lib -lSDL -lpthread -lm" \
		./configure --build=i386 --host=arm-linux-elf \
		--prefix=/usr $(SDL_MIXER_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src clean
	rm -f .configured .compiled

romfs:

