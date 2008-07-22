# Makefile for SDL

all: .compiled

SDL_CONFIGURE_FLAGS=	--disable-cdrom --disable-video-opengl \
			--disable-ipod --disable-video-dummy --disable-shared \
			--disable-loadso --enable-video-fbcon --with-gnu-ld \
			--disable-sdl-dlopen --without-x --disable-assembly \
			--disable-diskaudio --disable-mintaudio --disable-video-x11 \
			--disable-dga --disable-video-dga --disable-video-x11-dgamouse \
			--disable-video-x11-vm --disable-video-x11-xv --disable-video-x11-xinerama \
			--disable-video-x11-xme --disable-video-x11-xrandr --disable-video-x11-dpms \
			--disable-video-photon --disable-video-cocoa --enable-pthreads --enable-timers \
			--disable-pulseaudio --disable-esd \
			--disable-video-directfb --disable-arts --disable-arts-shared 

.configured:
	cd src && env CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" \
		./configure --build=i386 --host=arm-linux-elf \
		--prefix=/usr $(SDL_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src clean
	rm -f .configured .compiled

romfs:

