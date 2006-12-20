# Generated automatically from makefile.cfg by configure.
# Makefile for Independent JPEG Group's software

# makefile.cfg is edited by configure to produce a custom Makefile.

# Read installation instructions before saying "make" !!

# For compiling with source and object files in different directories.
srcdir = .

# Where to install the programs and man pages.
prefix = /usr/local
exec_prefix = ${prefix}
bindir = $(exec_prefix)/bin
libdir = $(exec_prefix)/lib
includedir = $(prefix)/include
binprefix =
manprefix =
manext = 1
mandir = $(prefix)/man/man$(manext)

# You may need to adjust these cc options:
CFLAGS= -O2  -I$(srcdir) $(ALLCFLAGS)
# Generally, we recommend defining any configuration symbols in jconfig.h,
# NOT via -D switches here.
# However, any special defines for ansi2knr.c may be included here:
ANSI2KNRFLAGS= 

# Link-time cc options:
LDFLAGS= 

# To link any special libraries, add the necessary -l commands here.
LDLIBS= 

# If using GNU libtool, LIBTOOL references it; if not, LIBTOOL is empty.
LIBTOOL = 
# $(O) expands to "lo" if using libtool, plain "o" if not.
# Similarly, $(A) expands to "la" or "a".
O = o
A = a

# Library version ID; libtool uses this for the shared library version number.
# Note: we suggest this match the macro of the same name in jpeglib.h.
JPEG_LIB_VERSION = 62

# Put here the object file name for the correct system-dependent memory
# manager file.  For Unix this is usually jmemnobs.o, but you may want
# to use jmemansi.o or jmemname.o if you have limited swap space.
SYSDEPMEM= jmemnobs.$(O)

# miscellaneous OS-dependent stuff
SHELL= /bin/sh
# linker
LN= $(CC)
# file deletion command
RM= rm -f
# directory creation command
MKDIR= mkdir
# library (.a) file creation command
AR= ar rc
# second step in .a creation (use "touch" if not needed)
AR2= ranlib
# installation program
INSTALL= /usr/bin/install -c
INSTALL_PROGRAM= ${INSTALL}
INSTALL_LIB= ${INSTALL} -m 644
INSTALL_DATA= ${INSTALL} -m 644

# End of configurable options.

# library object files common to compression and decompression
COMOBJECTS= jcomapi.$(O) jutils.$(O) jerror.$(O) jmemmgr.$(O) $(SYSDEPMEM)
# decompression library object files
DLIBOBJECTS= jdapimin.$(O) jdapistd.$(O) jdtrans.$(O) jdatasrc.$(O) \
        jdmaster.$(O) jdinput.$(O) jdmarker.$(O) jdhuff.$(O) jdphuff.$(O) \
        jdmainct.$(O) jdcoefct.$(O) jdpostct.$(O) jddctmgr.$(O) \
        jidctfst.$(O) jidctflt.$(O) jidctint.$(O) jidctred.$(O) \
        jdsample.$(O) jdcolor.$(O) jquant1.$(O) jquant2.$(O) jdmerge.$(O)
# These objectfiles are included in libjpeg.a
LIBOBJECTS= $(DLIBOBJECTS) $(COMOBJECTS)

all:  libjpeg.$(A)

# the library:

# without libtool:
libjpeg.a:  $(LIBOBJECTS)
	$(RM) libjpeg.a
	$(AR) libjpeg.a  $(LIBOBJECTS)
	$(AR2) libjpeg.a

clean:
	$(RM) *.o *.lo libjpeg.a libjpeg.la

# Mistake catcher:

# GNU Make likes to know which target names are not really files to be made:
.PHONY: all install install-lib install-headers clean distclean test check


jcomapi.$(O): jcomapi.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdapimin.$(O): jdapimin.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdapistd.$(O): jdapistd.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdatadst.$(O): jdatadst.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h
jdatasrc.$(O): jdatasrc.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jerror.h
jdcoefct.$(O): jdcoefct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdcolor.$(O): jdcolor.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jddctmgr.$(O): jddctmgr.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jdhuff.$(O): jdhuff.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdhuff.h
jdinput.$(O): jdinput.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdmainct.$(O): jdmainct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdmarker.$(O): jdmarker.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdmaster.$(O): jdmaster.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdmerge.$(O): jdmerge.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdphuff.$(O): jdphuff.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdhuff.h
jdpostct.$(O): jdpostct.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdsample.$(O): jdsample.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jdtrans.$(O): jdtrans.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jerror.$(O): jerror.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jversion.h jerror.h
jidctflt.$(O): jidctflt.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jidctfst.$(O): jidctfst.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jidctint.$(O): jidctint.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jidctred.$(O): jidctred.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jdct.h
jquant1.$(O): jquant1.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jquant2.$(O): jquant2.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jutils.$(O): jutils.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h
jmemmgr.$(O): jmemmgr.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
jmemnobs.$(O): jmemnobs.c jinclude.h jconfig.h jpeglib.h jmorecfg.h jpegint.h jerror.h jmemsys.h
