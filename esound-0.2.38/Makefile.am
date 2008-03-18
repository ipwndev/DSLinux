
SUBDIRS = docs

ACLOCAL_AMFLAGS = -I m4

SPEC_FILE = esound.spec

INCLUDES = $(AUDIOFILE_CFLAGS) -DSYSCONFDIR=\"$(sysconfdir)\" \
    -DSERVERDIR=\"$(ESD_DIR)\"

if BUILD_ESDDSP
ESDDSP = esddsp
libesddsp_la = libesddsp.la
endif
if BUILD_ESDPLAY
ESDPLAY = esdplay
endif

getopt_extra = getopt.c getopt1.c

if NEED_GETOPT
getopt_src = $(getopt_extra)
endif

bin_SCRIPTS = \
	$(ESDDSP)	\
	esd-config

lib_LTLIBRARIES = libesd.la $(libesddsp_la)

libesd_la_CFLAGS = $(AM_CFLAGS)
libesd_la_LDFLAGS = -version-info $(ESD_VERSION_INFO) -no-undefined
libesd_la_LIBADD = $(AUDIOFILE_LIBS) $(SOUND_LIBS)

libesd_la_SOURCES = \
	esdlib.c	\
	esdmgr.c	\
	esdfile.c	\
	esd-config.h	\
	esd_config.c	\
	audio.c		\
	genrand.c	\
	genrand.h	\
	util.c

# one of these gets included by audio.c
AUDIO_BACKENDS = \
	audio_aix.c		\
	audio_alsa.c		\
	audio_alsa09.c  	\
	audio_arts.c		\
	audio_coreaudio.c	\
	audio_dart.c		\
	audio_hpux.c		\
	audio_mklinux.c		\
	audio_irix.c		\
	audio_none.c		\
	audio_osf.c		\
	audio_oss.c		\
	audio_solaris.c		\
	audio_win32.c

libesddsp_la_LDFLAGS = -version-info $(ESD_VERSION_INFO)
libesddsp_la_LIBADD = $(DL_LIB) -lm libesd.la

libesddsp_la_SOURCES = \
	esddsp.c


include_HEADERS = \
	esd.h

esddir = @ESD_DIR@

esd_PROGRAMS = \
	esd		

bin_PROGRAMS = \
	esdcat		\
	esdctl		\
	esdfilt		\
	esdmon		\
	esdrec		\
	esdsample	\
	esdloop		\
	$(ESDPLAY)

esd_SOURCES = 	\
	esd.c 		\
	clients.c	\
	filter.c	\
	mix.c		\
	players.c	\
	proto.c		\
	samples.c	\
	$(getopt_src)   \
	esd-config.h	\
	esd-server.h

esdcat_SOURCES = \
	esdcat.c

esdctl_SOURCES = \
	$(getopt_src) \
	esdctl.c

esdfilt_SOURCES = \
	esdfilt.c

esdmon_SOURCES = \
	esdmon.c

esdrec_SOURCES = \
	esdrec.c

esdsample_SOURCES = \
	esdsample.c

esdloop_SOURCES = \
	esdloop.c

esdplay_SOURCES = \
	$(getopt_src) \
	esdplay.c

LDADD = libesd.la $(AUDIOFILE_LIBS)

esd_LDADD = $(WRAP_LIB) $(LDADD)

m4datadir = $(datadir)/aclocal
m4data_DATA = esd.m4

esdconfdir = $(sysconfdir)
esdconf_DATA = esd.conf

$(PACKAGE)-$(VERSION).tar.gz: dist

rpm: $(PACKAGE)-$(VERSION).tar.gz
	cp $(PACKAGE)-$(VERSION).tar.gz /usr/src/redhat/SOURCES
	rpm -ba $(SPEC_FILE)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = esound.pc

EXTRA_DIST = \
	$(getopt_extra)	\
	esddsp		\
	esound.pc.in	\
	esound.spec	\
	esound.spec.in	\
	esd-config.in	\
        esd.conf        \
	esd.m4 		\
	TIPS 		\
	test-script 	\
	test.wav 	\
	getopt.h 	\
	TODO		\
	MAINTAINERS	\
	m4/as-arts.m4   \
	m4/as-compiler-flag.m4 \
	$(AUDIO_BACKENDS)

