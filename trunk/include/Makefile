#
# build a smart "symlink" header tree so the compile lines are
# much shorter and the problems with include paths are much smaller
#
# David McCullough <davidm@snapgear.com>
#

#
# glibc contains lots of nasty headers in a bad location,  so we need
# to cheat a little here
#

LINKHDRS = \
	$(ROOTDIR)/lib/STLport/stlport/*,STLport \
	$(ROOTDIR)/lib/libgmp/gmp.h,. \
	$(ROOTDIR)/lib/libgmp/mp.h,. \
	$(ROOTDIR)/lib/libgmp/gmp-impl.h,. \
	$(ROOTDIR)/lib/libgmp/longlong.h,. \
	$(ROOTDIR)/lib/libgmp/urandom.h,. \
	$(ROOTDIR)/lib/pcre/builddir/pcre.h,. \
	$(ROOTDIR)/lib/pcre/pcreposix.h,. \
	$(ROOTDIR)/lib/libjpeg/*.h,. \
	$(ROOTDIR)/lib/libpng/*.h,. \
	$(ROOTDIR)/lib/libm/*.h,. \
	$(ROOTDIR)/lib/libncurses/include/*.h,. \
	$(ROOTDIR)/lib/libnet/*.h,. \
	$(ROOTDIR)/lib/libpcap/pcap*.h,. \
	$(ROOTDIR)/lib/libares/ares.h,. \
	$(ROOTDIR)/lib/libidn/lib/*.h,. \
	$(ROOTDIR)/lib/libcurl/include/curl/*.h,curl \
	$(ROOTDIR)/lib/libpcap/net/bpf.h,net \
	$(ROOTDIR)/lib/libssl/src/include/*,. \
	$(ROOTDIR)/lib/zlib/*.h,. \
	$(ROOTDIR)/lib/libbzip2/bzlib.h,. \
	$(ROOTDIR)/prop/libsnapgear/include/*.h,. \
	$(ROOTDIR)/prop/libsnapgear++/include/*,. \
	$(ROOTDIR)/lib/libcrypt_old/*.h,. \
	$(ROOTDIR)/lib/libldap/include/*,. \
	$(ROOTDIR)/lib/libldap/builddir/include/*,. \
	$(ROOTDIR)/lib/libupnp/upnp/inc/upnp,. \
	$(ROOTDIR)/lib/libatm/src/include/atm*.h,. \
	$(ROOTDIR)/lib/flex/build/FlexLexer.h,. \
	$(ROOTDIR)/$(LINUXDIR)/include/linux,. \
	$(ROOTDIR)/$(LINUXDIR)/include/asm-*,. \
	$(ROOTDIR)/$(LINUXDIR)/include/asm,. \
	$(ROOTDIR)/$(LINUXDIR)/include/mtd,. \
	$(ROOTDIR)/config,. \
	$(ROOTDIR)/lib/expat/lib/expat.h,. \
	$(ROOTDIR)/user/tinytcl/*.h,. \
	$(ROOTDIR)/prop/config/config*.h,. \
	$(ROOTDIR)/prop/config/if_*.h,. \
	$(ROOTDIR)/prop/config/libconfig.h,. \
	$(ROOTDIR)/prop/config/meta*.h,. \
	$(ROOTDIR)/prop/config/saverestoreall.h,. \
	$(ROOTDIR)/prop/config/vpnclient_utils.h,. \
	$(ROOTDIR)/prop/setkey/key.h,key \
	$(ROOTDIR)/prop/gccmapping/ValidateUpdate.h,. \
	$(ROOTDIR)/modules/autoconf.h,modules \
	$(ROOTDIR)/modules/ocf/cryptodev.h,crypto \
	$(ROOTDIR)/prop/smgrd/unixware/cmd/sscep/sscep_error_codes.h,. \
	$(ROOTDIR)/lib/libid3tag/id3tag.h,. \
	$(ROOTDIR)/lib/libmad/mad.h,. \
	$(ROOTDIR)/lib/gpm/src/headers/gpm.h,. \
	$(ROOTDIR)/lib/libwww/src/Library/src/*.h,. \
	$(ROOTDIR)/lib/libwww/src/wwwconf.h,. \
	$(ROOTDIR)/lib/libwww/src/modules/expat/xmlparse/xmlparse.h,. \
	$(ROOTDIR)/lib/libwww/src/modules/expat/xmltok/xmltok.h,. \
	$(ROOTDIR)/lib/libwww/src/modules/md5/md5.h,. \
	$(ROOTDIR)/lib/libiconv/src/include/iconv.h,. \
	$(ROOTDIR)/lib/gettext/src/gettext-runtime/intl/libintl.h,. \
	$(ROOTDIR)/lib/glib/src/glib/glib.h,. \
	$(ROOTDIR)/lib/glib/src/glibconfig.h,. \
	$(ROOTDIR)/lib/glib/src/glib/,. \
	$(ROOTDIR)/lib/sdl/src/include/*.h,. \
	$(ROOTDIR)/lib/audiofile/src/libaudiofile/audiofile.h,. \
	$(ROOTDIR)/lib/audiofile/src/libaudiofile/aupvlist.h,. \
	$(ROOTDIR)/lib/sdl_mixer/src/SDL_mixer.h,. \
        $(ROOTDIR)/lib/libtremor/src/*.h,tremor \
	$(ROOTDIR)/lib/libsndfile/src/src/sndfile.h,. \
	$(ROOTDIR)/lib/libsamplerate/src/src/*.h,. 
#
# Both freeswan and openswan can provide des and crypto headers
# and we need to make sure we are using the correct ones.
#
-include $(ROOTDIR)/$(LINUXDIR)/.config
ifeq ($(CONFIG_KLIPS),y) #openswan
LINKHDRS +=	\
	$(ROOTDIR)/openswan/linux/include/des/*.h,des \
	$(ROOTDIR)/openswan/linux/include/crypto/*.h,crypto \
	$(ROOTDIR)/openswan/linux/include/des/*.h,. \
	$(ROOTDIR)/openswan/linux/include/crypto/*.h,. 
else #freeswan
LINKHDRS +=	\
	$(ROOTDIR)/freeswan/libdes/*.h,. \
	$(ROOTDIR)/freeswan/libcrypto/libaes/*.h,. 
endif


.PHONY: all romfs clean


#
# link in the files,  avoid missing or CVS files,  existing files take
# precedance so we do not try to overwrite them
#

all: clean
	@echo "Making symlinks in include/"
	@for p in $(LINKHDRS); do \
		src=`expr "$$p" : '\(.*\),.*'` ; \
		dst=`expr "$$p" : '.*,\(.*\)'` ; \
		for i in $$src; do \
			[ ! -e "$$i" ] && continue; \
			[ "`basename $$i`" = CVS ] && continue; \
			[ -L "$$dst/`basename $$i`" ] && continue; \
			[ -d $$dst ] || mkdir $$dst; \
			ln -s $$i $$dst/.; \
		done ; \
	done
	@rm -f gcc
	@ln -s `$(CC) -print-file-name=include` gcc
#	@echo "Making include/c++ symlink to compiler c++ includes"
#	@rm -f c++
#	@ln -s /usr/local/include/g++-v3 c++
#	@for i in `$(CXX) -v -xc++ /dev/null 2>&1 | sed -e '/^Reading specs/,/^#include <\.\.\.>/d' -e '/End of search list/Q'`; do \
#		if [ -f $$i/new ]; then rm -f c++; ln -s $$i c++; break; fi; \
#	done

romfs:

clean: 	
	-find . -depth -type l -a ! -name Makefile | xargs rm > /dev/null 2>&1 || exit 0
	-find . -depth -type d | grep -v .svn | xargs rmdir > /dev/null 2>&1 || exit 0
