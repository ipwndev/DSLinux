# Makefile for Openvpn

all: .compiled

OPENVPN_CONFIGURE_FLAGS=--disable-lzo --includedir=$(ROOTDIR)/include\
			--libdir=$(ROOTDIR)/lib --with-ifconfig-path=/bin/ifconfig

ifeq ($(CONFIG_CONFIG_LIB_LIBSSL),y)
OPENVPN_CONFIGURE_FLAGS+=--with-ssl-lib=$(ROOTDIR)/lib/libssl
endif

.configured:
	cd src && ./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(OPENVPN_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled

romfs:
	$(ROMFSINST) src/openvpn /bin/openvpn
	$(ROMFSINST) openvpn.conf.example /etc/openvpn.conf.example
