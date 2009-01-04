##############################################################################
#
#	Makefile -- Master makefile for all libraries.
#

.EXPORT_ALL_VARIABLES:

##############################################################################
#
# Include architecture specific build rules.
#
ifndef ROOTDIR
ROOTDIR=..
endif

UCLINUX_BUILD_LIB=1
include $(LINUX_CONFIG)
include $(CONFIG_CONFIG)
include $(ARCH_CONFIG)

##############################################################################
#
# always build these dir_
#
dir_y  = ../include $(LIBCDIR) ../include
dir_n  =
dir_   =

ifeq ($(LIBCDIR),libc)
dir_y += libm libcrypt_old
endif

#
# and build these if someone needs them
#
dir_$(CONFIG_LIB_FLEX)         += flex
dir_$(CONFIG_LIB_FLEX_FORCE)   += flex
dir_$(CONFIG_LIB_GPM)          += gpm
dir_$(CONFIG_LIB_GPM_FORCE)    += gpm
dir_$(CONFIG_LIB_LIBAES)       += libaes
dir_$(CONFIG_LIB_LIBAES_FORCE) += libaes
dir_$(CONFIG_LIB_LIBDES)       += libdes
dir_$(CONFIG_LIB_LIBDES_FORCE) += libdes
dir_$(CONFIG_LIB_LIBSSL)       += libssl
dir_$(CONFIG_LIB_LIBSSL_FORCE) += libssl
dir_$(CONFIG_LIB_LIBGMP)       += libgmp
dir_$(CONFIG_LIB_LIBGMP_FORCE) += libgmp
dir_$(CONFIG_LIB_LIBG)         += libg
dir_$(CONFIG_LIB_LIBG_FORCE)   += libg
dir_$(CONFIG_LIB_LIBPAM)       += libpam
dir_$(CONFIG_LIB_LIBPAM_FORCE) += libpam
dir_$(CONFIG_LIB_LIBPCAP)      += libpcap
dir_$(CONFIG_LIB_LIBPCAP_FORCE)+= libpcap
dir_$(CONFIG_LIB_ZLIB)         += zlib
dir_$(CONFIG_LIB_ZLIB_FORCE)   += zlib
dir_$(CONFIG_LIB_LIBBZ2)       += libbzip2
dir_$(CONFIG_LIB_LIBBZ2_FORCE) += libbzip2
dir_$(CONFIG_LIB_LIBATM)       += libatm
dir_$(CONFIG_LIB_LIBATM_FORCE) += libatm
dir_$(CONFIG_LIB_LIBNET)       += Libnet
dir_$(CONFIG_LIB_LIBNET_FORCE) += Libnet
dir_$(CONFIG_LIB_LIBNETOLD)    += libnet
dir_$(CONFIG_LIB_LIBPNG)       += libpng
dir_$(CONFIG_LIB_LIBPNG_FORCE) += libpng
dir_$(CONFIG_LIB_LIBJPEG)      += libjpeg
dir_$(CONFIG_LIB_LIBJPEG_FORCE)+= libjpeg
dir_$(CONFIG_LIB_LIBUPNP)      += libupnp
dir_$(CONFIG_LIB_LIBUPNP_FORCE)+= libupnp
dir_$(CONFIG_LIB_STLPORT)      += STLport
dir_$(CONFIG_LIB_STLPORT_FORCE)+= STLport
dir_$(CONFIG_LIB_EXPAT)        += expat
dir_$(CONFIG_LIB_EXPAT_FORCE)  += expat
dir_$(CONFIG_LIB_LIBLDAP)      += libldap
dir_$(CONFIG_LIB_LIBLDAP_FORCE)+= libldap
dir_$(CONFIG_LIB_TINYTCL)      += ../user/tinytcl
dir_$(CONFIG_LIB_LIBCCMALLOC)  += libccmalloc
dir_$(CONFIG_LIB_ADNS_FORCE)   += adns
dir_$(CONFIG_LIB_ARES)         += libares
dir_$(CONFIG_LIB_ARES_FORCE)   += libares
dir_$(CONFIG_LIB_LIBIDN)       += libidn
dir_$(CONFIG_LIB_LIBIDN_FORCE) += libidn
dir_$(CONFIG_LIB_LIBCURL)      += libcurl
dir_$(CONFIG_LIB_LIBCURL_FORCE)+= libcurl
dir_$(CONFIG_LIB_LIBCRYPT_OLD) += libcrypt_old
dir_$(CONFIG_LIB_LIBCRYPT_OLD_FORCE) += libcrypt_old
dir_$(CONFIG_LIB_NCURSES)       += libncurses
dir_$(CONFIG_LIB_NCURSES_FORCE) += libncurses
dir_$(CONFIG_LIB_LIBPCRE)	+= pcre
dir_$(CONFIG_LIB_LIBPCRE_FORCE) += pcre
dir_$(CONFIG_LIB_ID3TAG)	+= libid3tag
dir_$(CONFIG_LIB_ID3TAG_FORCE)	+= libid3tag
dir_$(CONFIG_LIB_MAD)		+= libmad
dir_$(CONFIG_LIB_MAD_FORCE)	+= libmad
dir_$(CONFIG_LIB_UCLIBCPP)	+= uClibc++
dir_$(CONFIG_LIB_UCLIBCPP_FORCE)+= uClibc++
dir_$(CONFIG_LIB_LIBWWW)	+= libwww
dir_$(CONFIG_LIB_LIBWWW_FORCE)  += libwww
dir_$(CONFIG_LIB_LIBICONV)	+= libiconv
dir_$(CONFIG_LIB_LIBICONV_FORCE)  += libiconv
dir_$(CONFIG_LIB_LIBGETTEXT)	+= gettext
dir_$(CONFIG_LIB_LIBGETTEXT_FORCE)  += gettext
dir_$(CONFIG_LIB_LIBGLIB)	+= glib
dir_$(CONFIG_LIB_LIBGLIB_FORCE)  += glib
dir_$(CONFIG_LIB_LIBSNDFILE) += libsndfile
dir_$(CONFIG_LIB_LIBSNDFILE_FORCE) += libsndfile
dir_$(CONFIG_LIB_LIBSAMPLERATE)  += libsamplerate
dir_$(CONFIG_LIB_LIBSAMPLERATE_FORCE)  += libsamplerate
dir_$(CONFIG_LIB_LIBTREMOR)       += libtremor
dir_$(CONFIG_LIB_LIBTREMOR_FORCE) += libtremor
dir_$(CONFIG_LIB_SDL)       += sdl
dir_$(CONFIG_LIB_SDL_FORCE) += sdl
dir_$(CONFIG_LIB_SDL_MIXER)       += sdl_mixer
dir_$(CONFIG_LIB_SDL_MIXER_FORCE) += sdl_mixer
dir_$(CONFIG_LIB_AUDIOFILE) += audiofile
dir_$(CONFIG_LIB_AUDIOFILE_FORCE) += audiofile

# And build libraries in the prop directory last
dir_y += $(ROOTDIR)/prop

##############################################################################

all:
	for i in $(dir_y) ; do \
		if [ -d $$i ]; then \
			if [ "$$i" = "libncurses" ]; then \
				touch $$i/.sgbuilt_lib; \
				$(MAKE) -C $$i AWK=awk || exit $$?; \
			else \
				touch $$i/.sgbuilt_lib; \
				$(MAKE) -C $$i || exit $$? ; \
			fi; \
		fi; \
		$(MAKE) make_links || exit $? ; \
	done

##############################################################################

# Not the best place to install these symlinks, but it will do for now
romfs:
	$(ROMFSINST) -e CONFIG_USER_FLATFSD_FLATFSD -s config/passwd /etc/passwd
	$(ROMFSINST) -e CONFIG_USER_FLATFSD_FLATFSD -s config/group /etc/group
	$(ROMFSINST) -e CONFIG_USER_FLATFSD_FLATFSD -s config/mtab /etc/mtab
	$(ROMFSINST) -e CONFIG_USER_FLATFSD_FLATFSD -s config/fstab /etc/fstab
	$(ROMFSINST) -e CONFIG_USER_FLATFSD_FLATFSD -s config/resolv.conf /etc/resolv.conf
	$(ROMFSINST) -e CONFIG_USER_FLATFSD_FLATFSD -e CONFIG_USER_TINYLOGIN_SHADOWPASSWDS -s config/shadow /etc/shadow
	for i in $(dir_y) ; do \
		[ ! -d $$i ] || $(MAKE) -C $$i romfs || exit $$? ; \
	done

##############################################################################

clean: clean_links
	-for i in $(dir_y) $(dir_n) $(dir_); do \
		if [ -f $$i/.sgbuilt_lib ]; then \
			$(MAKE) -C $$i clean ; \
			rm -f $$i/.sgbuilt_lib; \
		fi; \
	done

##############################################################################
#
# The libs we link into this directory
#
#
LINKLIBS=\
	$(ROOTDIR)/lib/libjpeg/*.a \
	$(ROOTDIR)/lib/libpng/*.a \
	$(ROOTDIR)/lib/libm/*.a \
	$(ROOTDIR)/lib/libatm/src/lib/.libs/*.a \
	$(ROOTDIR)/lib/libdes/*.a \
	$(ROOTDIR)/lib/libaes/*.a \
	$(ROOTDIR)/lib/libpcap/build/*.a \
	$(ROOTDIR)/lib/libncurses/lib/*.a \
	$(ROOTDIR)/lib/libnet/*.a \
	$(ROOTDIR)/lib/libares/build/*.a \
	$(ROOTDIR)/lib/libidn/build/lib/.libs/libidn.a \
	$(ROOTDIR)/lib/libcurl/build/lib/.libs/libcurl.a \
	$(ROOTDIR)/lib/libcurl/build/lib/.libs/libcurl.so \
	$(ROOTDIR)/lib/libssl/src/*.so* $(ROOTDIR)/lib/libssl/src/*.a \
	$(ROOTDIR)/lib/zlib/libz.a \
	$(ROOTDIR)/lib/libbzip2/libbz2.so* \
	$(ROOTDIR)/lib/libatm/src/lib/*.so* $(ROOTDIR)/lib/libatm/src/lib/*.a \
	$(ROOTDIR)/lib/libcrypt_old/libcrypt_old.a \
	$(ROOTDIR)/lib/libldap/builddir/libraries/libldap/.libs/*.a \
	$(ROOTDIR)/lib/libldap/builddir/libraries/liblber/.libs/*.a \
	$(ROOTDIR)/lib/pcre/builddir/.libs/*.a \
	$(ROOTDIR)/prop/libsnapgear++/*.a $(ROOTDIR)/prop/libsnapgear++/*.so* \
	$(ROOTDIR)/prop/libsnapgear/*.a $(ROOTDIR)/prop/libsnapgear/*.so* \
	$(ROOTDIR)/user/tinytcl/*.a $(ROOTDIR)/user/tinytcl/*.so* \
	$(ROOTDIR)/prop/config/*.a $(ROOTDIR)/prop/config/*.so* \
	$(ROOTDIR)/prop/uberconfig/*.a $(ROOTDIR)/prop/uberconfig/*.so* \
	$(ROOTDIR)/prop/gccmapping/*.a $(ROOTDIR)/prop/gccmapping/*.so* \
	$(ROOTDIR)/lib/expat/*.a $(ROOTDIR)/lib/expat/*.so* \
	$(ROOTDIR)/lib/libupnp/upnp/bin/*.so* $(ROOTDIR)/lib/adns/src/libadns.a \
	$(ROOTDIR)/lib/flex/build/*.a \
	$(ROOTDIR)/lib/libgmp/*.a  \
	$(ROOTDIR)/lib/libid3tag/*.a \
	$(ROOTDIR)/lib/libmad/*.a \
	$(ROOTDIR)/lib/gpm/src/lib/*.a \
	$(ROOTDIR)/lib/uClibc++/src/*.a \
	$(ROOTDIR)/lib/libwww/src/Library/src/.libs/*.a \
	$(ROOTDIR)/lib/libwww/src/modules/expat/xmlparse/.libs/*.a \
	$(ROOTDIR)/lib/libwww/src/modules/expat/xmltok/.libs/*.a \
	$(ROOTDIR)/lib/libwww/src/modules/md5/.libs/*.a \
	$(ROOTDIR)/lib/libiconv/src/lib/.libs/*.a \
	$(ROOTDIR)/lib/gettext/src/gettext-runtime/intl/.libs/*.a \
	$(ROOTDIR)/lib/glib/src/glib/.libs/*.a \
	$(ROOTDIR)/lib/glib/src/gmodule/.libs/*.a \
	$(ROOTDIR)/lib/sdl/src/build/.libs/*.a \
	$(ROOTDIR)/lib/audiofile/src/libaudiofile/.libs/*.a \
	$(ROOTDIR)/lib/libtremor/src/.libs/*.a \
	$(ROOTDIR)/lib/sdl_mixer/src/build/.libs/*.a \
	$(ROOTDIR)/lib/libsamplerate/src/src/.libs/*.a \
	$(ROOTDIR)/lib/libsndfile/src/src/.libs/*.a 

##############################################################################

make_links: clean_links
	@echo "Making symlinks in lib/"
	@for p in $(LINKLIBS); do \
		for i in $$p; do \
			[ ! -e "$$i" ] && continue; \
			[ "`basename $$i`" = CVS ] && continue; \
			[ -L "./`basename $$i`" ] && continue; \
			ln -s $$i .; \
		done ; \
	done

clean_links:
	@-for i in *; do \
		[ "$$i" = "Makefile" ] && continue; \
		[ -L "$$i" ] || continue ; \
		case "`file -L $$i`" in \
		*directory*) ;; \
		*) rm -f $$i ;; \
		esac ; \
	done

##############################################################################
