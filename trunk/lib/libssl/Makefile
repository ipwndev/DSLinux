# Makefile for OpenSSL

all: .compiled

.configured:
	cd src && ./Configure \
		no-shared no-threads no-asm no-krb5 no-hw \
		--prefix=/nonexistent dslinux
	touch $@

.compiled: .configured
	cd src && make depend && make
	touch $@

clean:
	cd src && $(MAKE) clean
	rm -f .configured .compiled

# Empty romfs target since we only statically link with libraries
# and don't need to install them
romfs:

