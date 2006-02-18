#
# build using configure

all: builddir/config.status
	$(MAKE) -C builddir MAKE="$(MAKE) -f Makefile"

romfs: builddir/config.status
#	$(MAKE) -C builddir MAKE="$(MAKE) -f Makefile" $@

builddir/config.status: makefile
	rm -rf builddir;					\
	chmod u+x configure;					\
	mkdir builddir; 					\
	cd builddir;						\
	export CFLAGS="$(subst -Wall,,$(CFLAGS))"; 		\
	export LDFLAGS='-lc';					\
	../configure --prefix= 	\
		--host=i386-redhat-linux			\
		--with-headers=$(ROOTDIR)/$(LINUXDIR)/include	\
		--target=$(CROSS_COMPILE:-=)			\
		--disable-fast-install				\
#        --without-threads --without-local --disable-ipv6 \
#		--enable-x-compile --disable-slapd --disable-slurpd \
		; \
	make depend

clean:
	rm -rf builddir

