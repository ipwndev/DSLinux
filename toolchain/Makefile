#######################################################################
#
# This Makefile builds the DSLinux toolchain.
#
# Thanks to WinterMute for devkitARM, which was used to compile
# DSLinux before this toolchain saw the light of day.
# This Makefile was also based on his work initially.
#
# Thanks to DarkFader for writing ndstool, which we have
# taken from devkitARM CVS.
#
# Thanks to Mike Frysinger for inspiration:
# http://dev.gentoo.org/~vapier/CROSS-COMPILE-GUTS
#
#######################################################################

CWD	 	= $(shell pwd)
SRCDIR 		:= $(CWD)/src
OBJDIR 		:= $(CWD)/obj
DISTDIR		:= $(CWD)/distfiles
PATCHDIR 	:= $(CWD)/patches
TARGET		:= arm-linux-elf
PREFIX		:= $(CWD)/prefix
SYSROOT 	:= $(PREFIX)/$(TARGET)

BINUTILS_VER	= 2.16.1
GCC_VER		= 4.0.2
ELF2FLT_VER	= 20051225
GENROMFS_VER	= 0.5.1
LIBELF_VER	= 0.8.8

BINUTILS_DIST	:= binutils-$(BINUTILS_VER).tar.bz2
GCC_DIST	:= gcc-core-$(GCC_VER).tar.bz2
GPP_DIST	:= gcc-g++-$(GCC_VER).tar.bz2
ELF2FLT_DIST	:= elf2flt-$(ELF2FLT_VER).tar.gz
GENROMFS_DIST	:= genromfs-$(GENROMFS_VER).tar.gz
LIBELF_DIST	:= libelf-$(LIBELF_VER).tar.gz

# We do basic MD5 checking to prevent hassle with broken downloads.
BINUTILS_DIST_MD5	= 6a9d529efb285071dad10e1f3d2b2967
ELF2FLT_DIST_MD5	= 704ee32b30c58307bb5187d5ea9c7e2b
GCC_DIST_MD5		= f7781398ada62ba255486673e6274b26
GPP_DIST_MD5		= 97da403f4756d9e0b896368fd25c84df
GENROMFS_DIST_MD5	= fee69ecbf8f990fdb0ca0c7267c13e7e
LIBELF_DIST_MD5		= be84e553686dd50c8b6d520f86f46521

DISTFILES :=	$(DISTDIR)/$(BINUTILS_DIST) \
		$(DISTDIR)/$(GCC_DIST) \
		$(DISTDIR)/$(GPP_DIST) \
		$(DISTDIR)/$(ELF2FLT_DIST) \
		$(DISTDIR)/$(GENROMFS_DIST)
	
FETCH_CMD = wget -c

GNU_MIRROR		= ftp://ftp.fu-berlin.de/gnu
SOURCEFORGE_MIRROR	= http://switch.dl.sourceforge.net/sourceforge

BINUTILS_URL	:= $(GNU_MIRROR)/binutils/$(BINUTILS_DIST)
GCC_URL 	:= $(GNU_MIRROR)/gcc/gcc-$(GCC_VER)/$(GCC_DIST)
GPP_URL		:= $(GNU_MIRROR)/gcc/gcc-$(GCC_VER)/$(GPP_DIST)
ELF2FLT_URL	:= http://dslinux.in-berlin.de/$(ELF2FLT_DIST)
# Try this elf2flt mirror if the one above fails:
#ELF2FLT_URL	:= http://stsp.spline.de/dslinux/$(ELF2FLT_DIST)
GENROMFS_URL	:= $(SOURCEFORGE_MIRROR)/romfs/$(GENROMFS_DIST)
LIBELF_URL	:= http://www.mr511.de/software/$(LIBELF_DIST)

BINUTILS_SRCDIR := $(SRCDIR)/binutils-$(BINUTILS_VER)
GCC_SRCDIR	:= $(SRCDIR)/gcc-$(GCC_VER)
ELF2FLT_SRCDIR	:= $(SRCDIR)/elf2flt-$(ELF2FLT_VER)
GENROMFS_SRCDIR	:= $(SRCDIR)/genromfs-$(GENROMFS_VER)
LIBELF_SRCDIR	:= $(SRCDIR)/libelf-$(LIBELF_VER)
KERNEL_SRCDIR	:= $(CWD)/../linux-2.6.x
UCLIBC_SRCDIR	:= $(CWD)/../uClibc
NDSTOOL_SRCDIR	:= $(CWD)/ndstool

BINUTILS_OBJDIR := $(OBJDIR)/binutils-$(BINUTILS_VER)
GCC_OBJDIR	:= $(OBJDIR)/gcc-$(GCC_VER)
ELF2FLT_OBJDIR	:= $(OBJDIR)/elf2ftl-$(ELF2FLT_VER)
LIBELF_OBJDIR	:= $(OBJDIR)/libelf-$(ELF2FLT_VER)

export PATH := $(PREFIX)/bin:$(PATH)

#######################################################################
# Main targets.
#######################################################################

.PHONY: all dirs fetch clean reset nuke

# The order of dependencies of the 'all' target is important
# for the bootstrap process.
all: dirs fetch binutils-install elf2flt-install sysroot-create \
	gcc-stage1-install uClibc-install gcc-stage2-install \
	genromfs-install libelf-install ndstool-install

# Creates empty directories not shipped from CVS.
dirs:
	-$(foreach d,$(PREFIX) $(DISTDIR) $(SRCDIR) $(OBJDIR) $(SYSROOT), \
		[ -d ${d} ] || mkdir -p ${d};)

# If you just want to download distfiles, use this target.
fetch: $(DISTFILES)

# Use these to start a build from the beginning.
reset: binutils-reset uClibc-reset sysroot-reset gcc-stage1-reset \
	gcc-stage2-reset elf2flt-reset libelf-reset ndstool-reset

# Use these together with reset targets to force re-compile.
# Use on their own to save disc space.
clean: binutils-clean uClibc-clean sysroot-clean gcc-clean elf2flt-clean \
	libelf-clean ndstool-clean

# Nukes everything (including the installed toolchain itself!)
# Use this to start ALL OVER AGAIN! Use with caution!
nuke: reset
	rm -rf $(SRCDIR) $(OBJDIR) $(SYSROOT) $(PREFIX)

#######################################################################
# binutils
#######################################################################

binutils-unpack:	$(BINUTILS_SRCDIR)/.unpacked
binutils-configure:	$(BINUTILS_SRCDIR)/.configured
binutils-compile:	$(BINUTILS_SRCDIR)/.compiled
binutils-install:	$(BINUTILS_SRCDIR)/.installed
binutils-reset:
	rm -f $(foreach f, .unpacked .configured .compiled .installed, \
		$(BINUTILS_SRCDIR)/$(f))
binutils-clean:
	-$(MAKE) -C $(BINUTILS_OBJDIR) clean

# fetch distfile for binutils
$(DISTDIR)/$(BINUTILS_DIST):
	cd $(DISTDIR) && $(FETCH_CMD) $(BINUTILS_URL)

# unpack binutils
$(BINUTILS_SRCDIR)/.unpacked: $(DISTDIR)/$(BINUTILS_DIST)
	test "`md5sum $(DISTDIR)/$(BINUTILS_DIST) | cut -d' ' -f 1`" \
		= "$(BINUTILS_DIST_MD5)"
	tar -C $(SRCDIR) -jxf $(DISTDIR)/$(BINUTILS_DIST)
	touch $@

# configure binutils
$(BINUTILS_SRCDIR)/.configured: $(BINUTILS_SRCDIR)/.unpacked
	[ -d $(BINUTILS_OBJDIR) ] || mkdir -p $(BINUTILS_OBJDIR)
	cd $(BINUTILS_OBJDIR) \
		&& $(BINUTILS_SRCDIR)/configure \
		--prefix=$(PREFIX) \
		--with-sysroot=$(SYSROOT) \
		--target=$(TARGET) \
		--disable-nls \
		--disable-shared \
		--disable-threads \
		--with-gcc \
		--with-gnu-as \
		--with-gnu-ld
	touch $@

# compile binutils
$(BINUTILS_SRCDIR)/.compiled: $(BINUTILS_SRCDIR)/.configured
	$(MAKE) -C $(BINUTILS_OBJDIR)
	touch $@

# install binutils
$(BINUTILS_SRCDIR)/.installed: $(BINUTILS_SRCDIR)/.compiled
	$(MAKE) -C $(BINUTILS_OBJDIR) install
	touch $@

#######################################################################
# elf2flt
#######################################################################

elf2flt-unpack:		$(ELF2FLT_SRCDIR)/.unpacked
elf2flt-patch:		$(ELF2FLT_SRCDIR)/.patched
elf2flt-configure:	$(ELF2FLT_SRCDIR)/.configured
elf2flt-compile:	$(ELF2FLT_SRCDIR)/.compiled
elf2flt-install:	$(ELF2FLT_SRCDIR)/.installed
elf2flt-reset:
	rm -f $(foreach f, .unpacked .patched .configured .compiled \
		.installed, $(ELF2FLT_SRCDIR)/$(f))
elf2flt-clean:
	-$(MAKE) -C $(ELF2FLT_OBJDIR) clean

# fetch distfile for elf2flt
$(DISTDIR)/$(ELF2FLT_DIST):
	cd $(DISTDIR) && $(FETCH_CMD) $(ELF2FLT_URL)

# unpack elf2flt
$(ELF2FLT_SRCDIR)/.unpacked: $(DISTDIR)/$(ELF2FLT_DIST)
	test "`md5sum $(DISTDIR)/$(ELF2FLT_DIST) | cut -d' ' -f 1`" \
		= "$(ELF2FLT_DIST_MD5)"
	tar -C $(SRCDIR) -zxf $(DISTDIR)/$(ELF2FLT_DIST)
	touch $@

# patch elf2flt
$(ELF2FLT_SRCDIR)/.patched: $(ELF2FLT_SRCDIR)/.unpacked
	patch -d $(ELF2FLT_SRCDIR) -p1 < $(PATCHDIR)/elf2flt-$(ELF2FLT_VER).diff
	touch $@

# configure elf2flt
$(ELF2FLT_SRCDIR)/.configured: $(BINUTILS_SRCDIR)/.compiled \
		$(ELF2FLT_SRCDIR)/.patched
	[ -d $(ELF2FLT_OBJDIR) ] || mkdir -p $(ELF2FLT_OBJDIR)
	cd $(ELF2FLT_OBJDIR) \
		&& $(ELF2FLT_SRCDIR)/configure \
		--prefix=$(PREFIX) \
		--target=$(TARGET) \
		--with-libbfd=$(BINUTILS_OBJDIR)/bfd/libbfd.a \
		--with-libiberty=$(BINUTILS_OBJDIR)/libiberty/libiberty.a \
		--with-bfd-include-dir=$(BINUTILS_OBJDIR)/bfd \
		--with-binutils-include-dir=$(BINUTILS_SRCDIR)/include
	touch $@

# compile elf2flt
$(ELF2FLT_SRCDIR)/.compiled: $(ELF2FLT_SRCDIR)/.configured
	$(MAKE) -C $(ELF2FLT_OBJDIR)
	touch $@

# install elf2flt
$(ELF2FLT_SRCDIR)/.installed: $(ELF2FLT_SRCDIR)/.compiled
	$(MAKE) -C $(ELF2FLT_OBJDIR) install
	touch $@

#######################################################################
# uClibc
#######################################################################

uClibc-configure:	$(SRCDIR)/.uClibc-configured
uClibc-compile-headers:	$(SRCDIR)/.uClibc-headers-compiled
uClibc-compile:		$(SRCDIR)/.uClibc-compiled
uClibc-install:		$(SRCDIR)/.uClibc-installed
uClibc-reset:
	rm -f $(foreach f, .uClibc-configured .uClibc-compiled \
		.uClibc-installed, $(SRCDIR)/$(f))
uClibc-clean:
	-$(MAKE) -C $(UCLIBC_OBJDIR) clean

# configure uClibc
$(SRCDIR)/.uClibc-configured: $(SRCDIR)/.kernel-configured
	$(MAKE) -C $(UCLIBC_SRCDIR) distclean
	cp $(CWD)/../vendors/Nintendo/common/config.uClibc \
		$(UCLIBC_SRCDIR)/.config
	# fix the target prefix:
	sed -i -e 's:usr/$$(TARGET_ARCH)-linux-uclibc:$(TARGET):g' \
		$(UCLIBC_SRCDIR)/.config
	cd $(UCLIBC_SRCDIR) && yes "" | $(MAKE) oldconfig
	touch $@

# compile uClibc headers
$(SRCDIR)/.uClibc-headers-compiled: $(SRCDIR)/.uClibc-configured
	$(MAKE) -C $(UCLIBC_SRCDIR) KERNEL_SOURCE=$(KERNEL_SRCDIR) headers
	touch $@

# compile uCLibc
$(SRCDIR)/.uClibc-compiled: $(SRCDIR)/.uClibc-configured
	$(MAKE) -C $(UCLIBC_SRCDIR) KERNEL_SOURCE=$(KERNEL_SRCDIR) \
		CROSS=$(TARGET)-
	touch $@

# install uClibc
$(SRCDIR)/.uClibc-installed: $(SRCDIR)/.uClibc-compiled
	$(MAKE) -C $(UCLIBC_SRCDIR) PREFIX=$(PREFIX) install
	# XXX: Apparently there is a switch in the compiler specs that
	# causes the -g flag to use libg.a and not libc.a.
	# uClibc does not provide libg.a, so linking fails if -g is
	# in CFLAGS. Copying libc.a to libg.a seems make things work.
	# Workaround taken from notes by Phil Wilshire found at
	# http://www.ucdot.org/article.pl?sid=03/01/07/0214244
	cp $(PREFIX)/$(TARGET)/lib/libc.a $(PREFIX)/$(TARGET)/lib/libg.a
	touch $@


#######################################################################
# sysroot
#######################################################################

sysroot-create: copy-headers
sysroot-reset:
	rm -f $(foreach f, .kernel-headers-configured \
		.uClibc-headers-compiled .headers-copied, $(SRCDIR)/$(f))
sysroot-clean: 
	rm -rf $(SYSROOT)
kernel-configure:	$(SRCDIR)/.kernel-configured
copy-headers:		$(SRCDIR)/.headers-copied

# configure kernel headers
$(SRCDIR)/.kernel-configured:
	make -C $(KERNEL_SRCDIR) mrproper
	-cd $(KERNEL_SRCDIR) && yes "" | $(MAKE) ARCH=arm oldconfig prepare
	touch $@
			
# copy kernel and uClibc headers into sysroot
$(SRCDIR)/.headers-copied: $(SRCDIR)/.uClibc-headers-compiled
	mkdir -p $(SYSROOT)/usr
	tar -C $(UCLIBC_SRCDIR) --exclude=CVS -h -c -f - include \
		| tar -C $(SYSROOT) -x -f -
	touch $@

#######################################################################
# gcc
#######################################################################

gcc-stage1-unpack:	$(GCC_SRCDIR)/.stage1-unpacked
gcc-stage1-configure:	$(GCC_SRCDIR)/.stage1-configured
gcc-stage1-compile:	$(GCC_SRCDIR)/.stage1-compiled
gcc-stage1-install:	$(GCC_SRCDIR)/.stage1-installed
gcc-stage1-reset:
	rm -f $(foreach f, .stage1-unpacked .stage1-configured \
		.stage1-compiled .stage1-installed, $(GCC_SRCDIR)/$(f))
gcc-stage2-unpack:	$(GCC_SRCDIR)/.stage2-unpacked
gcc-stage2-patch:	$(GCC_SRCDIR)/.stage2-patched
gcc-stage2-configure:	$(GCC_SRCDIR)/.stage2-configured
gcc-stage2-compile:	$(GCC_SRCDIR)/.stage2-compiled
gcc-stage2-install:	$(GCC_SRCDIR)/.stage2-installed
gcc-stage2-reset:
	rm -f $(foreach f, .stage2-unpacked .stage2-patched \
		.stage2-configured .stage2-compiled .stage2-installed, \
		$(GCC_SRCDIR)/$(f))

gcc-clean:
	-$(MAKE) -C $(GCC_OBJDIR) clean

# fetch distfile for gcc
$(DISTDIR)/$(GCC_DIST):
	cd $(DISTDIR) && $(FETCH_CMD) $(GCC_URL) \
		&& $(FETCH_CMD) $(GPP_URL)

# unpack gcc (stage 1)
$(GCC_SRCDIR)/.stage1-unpacked: $(DISTDIR)/$(GCC_DIST)
	tar -C $(SRCDIR) -jxf $(DISTDIR)/$(GCC_DIST)
	touch $@

# configure gcc (stage 1)
$(GCC_SRCDIR)/.stage1-configured: $(GCC_SRCDIR)/.stage1-unpacked
	[ -d $(GCC_OBJDIR) ] || mkdir -p $(GCC_OBJDIR)
	cd $(GCC_OBJDIR) \
		&& $(GCC_SRCDIR)/configure \
		--prefix=$(PREFIX) \
		--with-sysroot=$(SYSROOT) \
		--target=$(TARGET) \
		--enable-languages=c \
		--with-cpu=arm7tdmi \
		--with-gcc \
		--with-gnu-ld \
		--with-gnu-as \
		--disable-shared \
		--disable-threads \
		--disable-nls
	touch $@

# compile gcc (stage 1)
$(GCC_SRCDIR)/.stage1-compiled: $(GCC_SRCDIR)/.stage1-configured
	$(MAKE) -C $(GCC_OBJDIR)
	touch $@

# install gcc (stage 1)
$(GCC_SRCDIR)/.stage1-installed: $(GCC_SRCDIR)/.stage1-compiled
	$(MAKE) -C $(GCC_OBJDIR) install
	touch $@

# unpack gcc (stage 2)
$(GCC_SRCDIR)/.stage2-unpacked: $(DISTDIR)/$(GCC_DIST)
	test "`md5sum $(DISTDIR)/$(GCC_DIST) | cut -d' ' -f 1`" \
		= "$(GCC_DIST_MD5)"
	test "`md5sum $(DISTDIR)/$(GPP_DIST) | cut -d' ' -f 1`" \
		= "$(GPP_DIST_MD5)"
	tar -C $(SRCDIR) -jxf $(DISTDIR)/$(GCC_DIST)
	tar -C $(SRCDIR) -jxf $(DISTDIR)/$(GPP_DIST)
	touch $@

# patch gcc (stage 2)
$(GCC_SRCDIR)/.stage2-patched: $(GCC_SRCDIR)/.stage2-unpacked
	patch -d $(GCC_SRCDIR) -p1 < $(PATCHDIR)/gcc-$(GCC_VER).diff
	touch $@

# configure gcc (stage 2)
$(GCC_SRCDIR)/.stage2-configured: $(GCC_SRCDIR)/.stage2-patched
	[ -d $(GCC_OBJDIR) ] || mkdir -p $(GCC_OBJDIR)
	-$(MAKE) -C $(GCC_OBJDIR) distclean
	# XXX: We need -fpermissive in CFLAGS for libstdc++ to build.
	# Produces lots of warning for non-C++ code though :(
	cd $(GCC_OBJDIR) \
		&& export CFLAGS="-fpermissive -O2 -g" \
		&& $(GCC_SRCDIR)/configure \
		--prefix=$(PREFIX) \
		--with-sysroot=$(SYSROOT) \
		--target=$(TARGET) \
		--enable-languages=c,c++ \
		--with-cpu=arm7tdmi \
		--with-gcc \
		--with-gnu-ld \
		--with-gnu-as \
		--disable-shared \
		--disable-threads \
		--disable-nls
	touch $@

# compile gcc (stage 2)
$(GCC_SRCDIR)/.stage2-compiled: $(GCC_SRCDIR)/.stage2-configured
	$(MAKE) -C $(GCC_OBJDIR)
	touch $@

# install gcc (stage 2)
$(GCC_SRCDIR)/.stage2-installed: $(GCC_SRCDIR)/.stage2-compiled
	$(MAKE) -C $(GCC_OBJDIR) install
	touch $@

#######################################################################
# genromfs
#######################################################################

genromfs-unpack:	$(GENROMFS_SRCDIR)/.unpacked
genromfs-compile:	$(GENROMFS_SRCDIR)/.compiled
genromfs-install:	$(GENROMFS_SRCDIR)/.installed
genromfs-reset:
	rm -f $(foreach f, .unpacked .compiled .installed, \
		$(GENROMFS_SRCDIR)/$(f))
genromfs-clean:
	-$(MAKE) -C $(GENROMFS_SRCDIR) clean

# fetch distfile for genromfs
$(DISTDIR)/$(GENROMFS_DIST):
	cd $(DISTDIR) && $(FETCH_CMD) $(GENROMFS_URL)

# unpack genromfs
$(GENROMFS_SRCDIR)/.unpacked: $(DISTDIR)/$(GENROMFS_DIST)
	test "`md5sum $(DISTDIR)/$(GENROMFS_DIST) | cut -d' ' -f 1`" \
		= "$(GENROMFS_DIST_MD5)"
	tar -C $(SRCDIR) -zxf $(DISTDIR)/$(GENROMFS_DIST)
	touch $@

# compile genromfs
$(GENROMFS_SRCDIR)/.compiled: $(GENROMFS_SRCDIR)/.unpacked
	# Genromfs has a very simple Makefile that does not
	# allow building outside the source directory
	$(MAKE) -C $(GENROMFS_SRCDIR) prefix=$(PREFIX)
	touch $@

# install genromfs
$(GENROMFS_SRCDIR)/.installed: $(GENROMFS_SRCDIR)/.compiled
	# The genromfs Makefile fails to install the man page
	# if the man8 directory does not exist.
	mkdir -p $(PREFIX)/man/man8
	$(MAKE) -C $(GENROMFS_SRCDIR) prefix=$(PREFIX) install
	touch $@

#######################################################################
# libelf
#######################################################################

libelf-unpack:		$(LIBELF_SRCDIR)/.unpacked
libelf-configure:	$(LIBELF_SRCDIR)/.configured
libelf-compile:		$(LIBELF_SRCDIR)/.compiled
libelf-install:		$(LIBELF_SRCDIR)/.installed
libelf-reset:
	rm -f $(foreach f, .unpacked .configured .compiled \
		.installed, $(LIBELF_SRCDIR)/$(f))
libelf-clean:
	-$(MAKE) -C $(LIBELF_OBJDIR) clean

# fetch distfile for libelf
$(DISTDIR)/$(LIBELF_DIST):
	cd $(DISTDIR) && $(FETCH_CMD) $(LIBELF_URL)

# unpack libelf
$(LIBELF_SRCDIR)/.unpacked: $(DISTDIR)/$(LIBELF_DIST)
	test "`md5sum $(DISTDIR)/$(LIBELF_DIST) | cut -d' ' -f 1`" \
		= "$(LIBELF_DIST_MD5)"
	tar -C $(SRCDIR) -zxf $(DISTDIR)/$(LIBELF_DIST)
	touch $@

# configure libelf
$(LIBELF_SRCDIR)/.configured: $(LIBELF_SRCDIR)/.unpacked
	[ -d $(LIBELF_OBJDIR) ] || mkdir -p $(LIBELF_OBJDIR)
	cd $(LIBELF_OBJDIR) \
		&& $(LIBELF_SRCDIR)/configure \
		--prefix=/ \
		--disable-shared \
		--enable-compat
	touch $@

# compile libelf
$(LIBELF_SRCDIR)/.compiled: $(LIBELF_SRCDIR)/.configured
	$(MAKE) -C $(LIBELF_OBJDIR)
	touch $@

# install libelf
$(LIBELF_SRCDIR)/.installed: $(LIBELF_SRCDIR)/.compiled
	$(MAKE) -C $(LIBELF_OBJDIR) instroot=$(PREFIX) install
	touch $@

#######################################################################
# ndstool
#######################################################################

ndstool-compile:		$(NDSTOOL_SRCDIR)/.compiled
ndstool-install:		$(NDSTOOL_SRCDIR)/.installed
ndstool-reset:
	rm -f $(foreach f, .compiled .installed, $(NDSTOOL_SRCDIR)/$(f))
ndstool-clean:
	-$(MAKE) -C $(NDSTOOL_SRCDIR) clean

# compile ndstool
$(NDSTOOL_SRCDIR)/.compiled:
	$(MAKE) -C $(NDSTOOL_SRCDIR) CROSS=$(TARGET)- \
		NDSTOOL_SRCDIR=$(NDSTOOL_SRCDIR) \
		PREFIX=$(PREFIX)
	touch $@

# install ndstool
$(NDSTOOL_SRCDIR)/.installed: $(NDSTOOL_SRCDIR)/.compiled
	install -m 755 $(NDSTOOL_SRCDIR)/ndstool $(PREFIX)/bin
	touch $@


