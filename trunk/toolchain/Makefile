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
EIGHTBIT_DIR	:= $(CWD)/8bit

BINUTILS_VER	= 2.17
GCC_VER		= 4.0.4
ELF2FLT_VER	= 20051225
GENROMFS_VER	= 0.5.1
LIBELF_VER	= 0.8.8
GDB_VER		= 6.6

BINUTILS_DIST	:= binutils-$(BINUTILS_VER).tar.bz2
GCC_DIST	:= gcc-core-$(GCC_VER).tar.bz2
GPP_DIST	:= gcc-g++-$(GCC_VER).tar.bz2
ELF2FLT_DIST	:= elf2flt-$(ELF2FLT_VER).tar.gz
GENROMFS_DIST	:= genromfs-$(GENROMFS_VER).tar.gz
LIBELF_DIST	:= libelf-$(LIBELF_VER).tar.gz
GDB_DIST	:= gdb-$(GDB_VER)a.tar.bz2

# We do basic MD5 checking to prevent hassle with broken downloads.
BINUTILS_DIST_MD5	= 1d81edd0569de4d84091526fd578dd7b
ELF2FLT_DIST_MD5	= 704ee32b30c58307bb5187d5ea9c7e2b
GCC_DIST_MD5		= 193e0a7a471cca70e374974bc5a60137
GPP_DIST_MD5		= 5c79e6af5b49dd9cfd22001f3856f447
GENROMFS_DIST_MD5	= fee69ecbf8f990fdb0ca0c7267c13e7e
LIBELF_DIST_MD5		= be84e553686dd50c8b6d520f86f46521
GDB_DIST_MD5		= f77ce6e22dd5b8621faf4f846e7d5e33

DISTFILES :=	$(DISTDIR)/$(BINUTILS_DIST) \
		$(DISTDIR)/$(GCC_DIST) \
		$(DISTDIR)/$(GPP_DIST) \
		$(DISTDIR)/$(ELF2FLT_DIST) \
		$(DISTDIR)/$(GENROMFS_DIST) \
		$(DISTDIR)/$(LIBELF_DIST) \
		$(DISTDIR)/$(GDB_DIST)
	
FETCH_CMD = wget -c --passive-ftp

GNU_MIRROR		= ftp://ftp.fu-berlin.de/gnu
SOURCEFORGE_MIRROR	= http://switch.dl.sourceforge.net/sourceforge

BINUTILS_URL	:= $(GNU_MIRROR)/binutils/$(BINUTILS_DIST)
GCC_URL 	:= $(GNU_MIRROR)/gcc/gcc-$(GCC_VER)/$(GCC_DIST)
GPP_URL		:= $(GNU_MIRROR)/gcc/gcc-$(GCC_VER)/$(GPP_DIST)
ELF2FLT_URL	:= http://dslinux.org/misc/$(ELF2FLT_DIST)
# Try this elf2flt mirror if the one above fails:
#ELF2FLT_URL	:= http://stsp.spline.de/dslinux/$(ELF2FLT_DIST)
GENROMFS_URL	:= $(SOURCEFORGE_MIRROR)/romfs/$(GENROMFS_DIST)
LIBELF_URL	:= http://www.mr511.de/software/$(LIBELF_DIST)
GDB_URL		:= $(GNU_MIRROR)/gdb/$(GDB_DIST)

BINUTILS_SRCDIR := $(SRCDIR)/binutils-$(BINUTILS_VER)
GCC_SRCDIR	:= $(SRCDIR)/gcc-$(GCC_VER)
ELF2FLT_SRCDIR	:= $(SRCDIR)/elf2flt-$(ELF2FLT_VER)
GENROMFS_SRCDIR	:= $(SRCDIR)/genromfs-$(GENROMFS_VER)
LIBELF_SRCDIR	:= $(SRCDIR)/libelf-$(LIBELF_VER)
GDB_SRCDIR	:= $(SRCDIR)/gdb-$(GDB_VER)
KERNEL_SRCDIR	:= $(CWD)/../linux-2.6.x
UCLIBC_SRCDIR	:= $(CWD)/../uClibc
NDSTOOL_SRCDIR	:= $(CWD)/ndstool
SYMBOLMAP_SH	:= $(CWD)/symbolmap.sh

BINUTILS_OBJDIR := $(OBJDIR)/binutils-$(BINUTILS_VER)
GCC_OBJDIR	:= $(OBJDIR)/gcc-$(GCC_VER)
ELF2FLT_OBJDIR	:= $(OBJDIR)/elf2ftl-$(ELF2FLT_VER)
LIBELF_OBJDIR	:= $(OBJDIR)/libelf-$(ELF2FLT_VER)
GDB_OBJDIR	:= $(OBJDIR)/gdb-$(GDB_VER)

# For releases, this must be changed to the version number:
VERSION		:= $(shell date -I)
DISTNAME	:= dslinux-toolchain-$(VERSION)-$(shell uname -m)

export PATH	:= $(PREFIX)/bin:$(PATH)

#######################################################################
# Main targets.
#######################################################################

.PHONY: all fetch reset clean nuke

# The order of dependencies of the 'all' target is important
# for the bootstrap process.
all: dirs-create binutils-install elf2flt-install sysroot-create \
	gcc-stage1-install uClibc-install gcc-stage2-install \
	genromfs-install libelf-install gdb-install ndstool-install \
	symbolmap.sh-install strip dist

# If you just want to download distfiles, use this target.
fetch: $(CWD)/.dirs-created $(DISTFILES)

# Use these to start a build from the beginning.
reset: dirs-reset binutils-reset uClibc-reset sysroot-reset gcc-stage1-reset \
	gcc-stage2-reset elf2flt-reset libelf-reset gdb-reset ndstool-reset strip-reset \
	dist-reset

# Use to save disc space.
clean: binutils-clean uClibc-clean sysroot-clean gcc-clean elf2flt-clean \
	libelf-clean gdb-clean ndstool-clean

# Nukes everything (including the installed toolchain itself!)
# Use this to start ALL OVER AGAIN! Use with caution!
nuke:
	@echo
	@echo "I will now remove the following directories PERMANENTLY:"
	@echo
	@echo "  $(SRCDIR)"
	@echo "  $(OBJDIR)"
	@echo "  $(SYSROOT)"
	@echo "  $(PREFIX)"
	@echo
	@read -p 'Do you want me to continue? ([no]/yes): ' ANSWER ;\
		case $$ANSWER in \
			"yes") \
				echo "You said $$ANSWER. I will continue."; \
				echo rm -rf $(SRCDIR) $(OBJDIR) \
					$(SYSROOT) $(PREFIX); \
				rm -rf $(SRCDIR) $(OBJDIR) \
					$(SYSROOT) $(PREFIX); \
				echo "Resetting build"; \
				$(MAKE) reset; \
				;; \
			"") \
				echo "You said no."; \
				;; \
			*) \
				echo "You said $$ANSWER."; \
				;; \
		esac

#######################################################################
# directories
#######################################################################

dirs-create: $(CWD)/.dirs-created
dirs-reset:
	rm -f $(CWD)/.dirs-created

$(CWD)/.dirs-created: 
	-$(foreach d,$(PREFIX) $(DISTDIR) $(SRCDIR) $(OBJDIR) $(SYSROOT), \
		[ -d ${d} ] || mkdir -p ${d};)
	touch $@

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
		--enable-multilib \
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
		--with-binutils-include-dir=$(BINUTILS_SRCDIR)/include \
		--enable-emit_relocs
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
	# fix target prefix:
	sed -i -e 's:usr/$$(TARGET_ARCH)-linux-uclibc:$(TARGET):g' \
		$(UCLIBC_SRCDIR)/.config
	cd $(UCLIBC_SRCDIR) && yes "" | $(MAKE) oldconfig
	touch $@

# compile uClibc headers
$(SRCDIR)/.uClibc-headers-compiled: $(SRCDIR)/.uClibc-configured
	$(MAKE) -C $(UCLIBC_SRCDIR) KERNEL_SOURCE=$(KERNEL_SRCDIR) headers
	touch $@

# compile uClibc
$(SRCDIR)/.uClibc-compiled: $(SRCDIR)/.uClibc-configured
	$(MAKE) -C $(UCLIBC_SRCDIR) KERNEL_SOURCE=$(KERNEL_SRCDIR) \
		CROSS=$(TARGET)-
	touch $@

# XXX: Apparently there is a switch in the compiler specs that
# causes the -g flag to link with libg.a in addition to libc.a.
# uClibc does not provide libg.a, so linking fails if -g is
# in CFLAGS. Create a dummy libg.a to work around this.
$(PREFIX)/$(TARGET)/lib/libg.a: $(SRCDIR)/.uClibc-compiled
	echo 'void __libg__dummy(){};' | $(TARGET)-gcc -xc -c -o libg.o -
	$(TARGET)-ar r libg.a libg.o
	rm libg.o
	mv libg.a $(PREFIX)/$(TARGET)/lib/

# install uClibc
$(SRCDIR)/.uClibc-installed: $(PREFIX)/$(TARGET)/lib/libg.a
	$(MAKE) -C $(UCLIBC_SRCDIR) PREFIX=$(PREFIX) install
	touch $@


#######################################################################
# sysroot
#######################################################################

sysroot-create: copy-headers
sysroot-reset:
	rm -f $(foreach f, .kernel-configured \
		.uClibc-headers-compiled .headers-copied, $(SRCDIR)/$(f))
sysroot-clean: 
	rm -rf $(SYSROOT)
kernel-configure:	$(SRCDIR)/.kernel-configured
copy-headers:		$(SRCDIR)/.headers-copied

# configure kernel headers
$(SRCDIR)/.kernel-configured:
	# backup kernel config if exists
	-[ -e $(KERNEL_SRCDIR)/.config ] \
		&& cp $(KERNEL_SRCDIR)/.config $(CWD)/.kernelconfig.saved
	make -C $(KERNEL_SRCDIR) mrproper
	-cd $(KERNEL_SRCDIR) && yes "" | $(MAKE) ARCH=arm oldconfig prepare
	touch $@
			
# copy kernel and uClibc headers into sysroot
$(SRCDIR)/.headers-copied: $(SRCDIR)/.uClibc-headers-compiled
	mkdir -p $(SYSROOT)/usr
	tar -C $(UCLIBC_SRCDIR) --exclude=CVS -h -c -f - include \
		| tar -C $(SYSROOT)/usr -x -f -
	# restore kernel config if backup exists
	-[ -e $(CWD)/.kernelconfig.saved ] \
		&& mv $(CWD)/.kernelconfig.saved $(KERNEL_SRCDIR)/.config
	touch $@

#######################################################################
# gcc
#######################################################################

gcc-stage1-unpack:	$(GCC_SRCDIR)/.stage1-unpacked
gcc-stage1-patch:	$(GCC_SRCDIR)/.stage1-patched
gcc-stage1-configure:	$(GCC_SRCDIR)/.stage1-configured
gcc-stage1-compile:	$(GCC_SRCDIR)/.stage1-compiled
gcc-stage1-install:	$(GCC_SRCDIR)/.stage1-installed
gcc-stage1-reset:
	rm -f $(foreach f, .stage1-unpacked .stage1-patched \
		.stage1-configured .stage1-compiled .stage1-installed, \
		$(GCC_SRCDIR)/$(f))
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
	cd $(DISTDIR) && $(FETCH_CMD) $(GCC_URL)

# fetch distfile for g++
$(DISTDIR)/$(GPP_DIST):
	cd $(DISTDIR) && $(FETCH_CMD) $(GPP_URL)

# unpack gcc (stage 1)
$(GCC_SRCDIR)/.stage1-unpacked: $(DISTDIR)/$(GCC_DIST)
	test "`md5sum $(DISTDIR)/$(GCC_DIST) | cut -d' ' -f 1`" \
		= "$(GCC_DIST_MD5)"
	tar -C $(SRCDIR) -jxf $(DISTDIR)/$(GCC_DIST)
	touch $@

# patch gcc (stage 1)
$(GCC_SRCDIR)/.stage1-patched: $(GCC_SRCDIR)/.stage1-unpacked
	# Use Amadeus' ARM code generator that fixes the 8bit write problem
	$(foreach f,arm.h arm.md predicates.md t-arm-elf, \
		cp $(EIGHTBIT_DIR)/$(f) $(GCC_SRCDIR)/gcc/config/arm/;)
	touch $@

# configure gcc (stage 1)
$(GCC_SRCDIR)/.stage1-configured: $(GCC_SRCDIR)/.stage1-patched
	[ -d $(GCC_OBJDIR) ] || mkdir -p $(GCC_OBJDIR)
	cd $(GCC_OBJDIR) \
		&& $(GCC_SRCDIR)/configure \
		--prefix=$(PREFIX) \
		--with-sysroot=$(SYSROOT) \
		--target=$(TARGET) \
		--enable-languages=c \
		--enable-multilib \
		--with-cpu=arm7tdmi \
		--with-gcc \
		--with-gnu-ld \
		--with-gnu-as \
		--disable-shared \
		--disable-threads \
		--disable-nls \
		--disable-libssp
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
$(GCC_SRCDIR)/.stage2-unpacked: $(DISTDIR)/$(GCC_DIST) $(DISTDIR)/$(GPP_DIST)
	test "`md5sum $(DISTDIR)/$(GCC_DIST) | cut -d' ' -f 1`" \
		= "$(GCC_DIST_MD5)"
	test "`md5sum $(DISTDIR)/$(GPP_DIST) | cut -d' ' -f 1`" \
		= "$(GPP_DIST_MD5)"
	tar -C $(SRCDIR) -jxf $(DISTDIR)/$(GCC_DIST)
	tar -C $(SRCDIR) -jxf $(DISTDIR)/$(GPP_DIST)
	touch $@

# patch gcc (stage 2)
$(GCC_SRCDIR)/.stage2-patched: $(GCC_SRCDIR)/.stage2-unpacked
	patch -d $(GCC_SRCDIR) -p1 < $(PATCHDIR)/g++-$(GCC_VER).diff
	# Use Amadeus' ARM code generator that fixes the 8bit write problem
	$(foreach f,arm.h arm.md predicates.md t-arm-elf, \
		cp $(EIGHTBIT_DIR)/$(f) $(GCC_SRCDIR)/gcc/config/arm/;)
	touch $@

# configure gcc (stage 2)
$(GCC_SRCDIR)/.stage2-configured: $(GCC_SRCDIR)/.stage2-patched
	[ -d $(GCC_OBJDIR) ] || mkdir -p $(GCC_OBJDIR)
	-$(MAKE) -C $(GCC_OBJDIR) distclean
	cd $(GCC_OBJDIR) \
		&& $(GCC_SRCDIR)/configure \
		--prefix=$(PREFIX) \
		--with-sysroot=$(SYSROOT) \
		--target=$(TARGET) \
		--enable-languages=c,c++ \
		--enable-multilib \
		--with-cpu=arm7tdmi \
		--with-gcc \
		--with-gnu-ld \
		--with-gnu-as \
		--disable-shared \
		--disable-threads \
		--disable-nls \
		--disable-libssp
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

#######################################################################
# gdb
#######################################################################

gdb-unpack:		$(GDB_SRCDIR)/.unpacked
gdb-configure:		$(GDB_SRCDIR)/.configured
gdb-compile:		$(GDB_SRCDIR)/.compiled
gdb-install:		$(GDB_SRCDIR)/.installed
gdb-reset:
	rm -f $(foreach f, .unpacked .configured .compiled \
		.installed, $(GDB_SRCDIR)/$(f))
gdb-clean:
	-$(MAKE) -C $(GDB_OBJDIR) distclean
	rm -f $(GDB_OBJDIR)/bfd/config.*
	rm -f $(GDB_OBJDIR)/etc/config.*
	rm -f $(GDB_OBJDIR)/gdb/config.*
	rm -f $(GDB_OBJDIR)/gdb/doc/config.*
	rm -f $(GDB_OBJDIR)/gdb/testsuite/config.*
	rm -f $(GDB_OBJDIR)/gdb/testsuite/gdb.stabs/config.*
	rm -f $(GDB_OBJDIR)/intl/config.*
	rm -f $(GDB_OBJDIR)/libiberty/config.*
	rm -f $(GDB_OBJDIR)/opcodes/config.*
	rm -f $(GDB_OBJDIR)/readline/config.*
	rm -f $(GDB_OBJDIR)/sim/config.*
	rm -f $(GDB_OBJDIR)/sim/arm/config.*
	rm -f $(GDB_OBJDIR)/sim/common/config.*
	rm -f $(GDB_OBJDIR)/sim/testsuite/config.*

# fetch distfile for gdb
$(DISTDIR)/$(GDB_DIST):
	cd $(DISTDIR) && $(FETCH_CMD) $(GDB_URL)

# unpack gdb
$(GDB_SRCDIR)/.unpacked: $(DISTDIR)/$(GDB_DIST)
	test "`md5sum $(DISTDIR)/$(GDB_DIST) | cut -d' ' -f 1`" \
		= "$(GDB_DIST_MD5)"
	tar -C $(SRCDIR) -jxf $(DISTDIR)/$(GDB_DIST)
	touch $@

# configure gdb
$(GDB_SRCDIR)/.configured: $(GDB_SRCDIR)/.unpacked
	[ -d $(GDB_OBJDIR) ] || mkdir -p $(GDB_OBJDIR)
	cd $(GDB_OBJDIR) \
		&& $(GDB_SRCDIR)/configure \
		--prefix=$(PREFIX) \
		--target=arm-linux-elf 
	touch $@

# compile gdb
$(GDB_SRCDIR)/.compiled: $(GDB_SRCDIR)/.configured
	$(MAKE) -C $(GDB_OBJDIR)
	touch $@

# install gdb
$(GDB_SRCDIR)/.installed: $(GDB_SRCDIR)/.compiled
	$(MAKE) -C $(GDB_OBJDIR) install
	touch $@

#######################################################################
# symbolmap.sh
#######################################################################

symbolmap.sh-install: $(SRCDIR)/.symbolmap.sh-installed
symbolmap.sh-reset:
	rm -f $(SRCDIR)/.symbolmap.sh-installed

$(SRCDIR)/.symbolmap.sh-installed:
	install -m 755 $(SYMBOLMAP_SH) $(PREFIX)/bin
	touch $@

#######################################################################
# strip
#######################################################################

strip: $(CWD)/.stripped
strip-reset:
	rm -f $(CWD)/.stripped

# Strip the size of the toolchain down as much as possible.
$(CWD)/.stripped:
	# remove large pre-compiled C++ headers
	rm -rf $(PREFIX)/include/c++/$(GCC_VER)/$(TARGET)/bits/stdc++.h.gch
	# remove redundant include directories
	rm -rf $(PREFIX)/$(TARGET)/include
	rm -rf $(PREFIX)/$(TARGET)/usr/include/asm
	ln -sf asm-arm $(PREFIX)/$(TARGET)/usr/include/asm
	# strip binaries completely
	-strip $(PREFIX)/bin/*
	-strip $(PREFIX)/$(TARGET)/bin/*
	-strip $(PREFIX)/libexec/gcc/$(TARGET)/$(GCC_VER)/*
	-strip $(PREFIX)/libexec/gcc/$(TARGET)/$(GCC_VER)/install-tools/*
	# strip debug symbols from libraries
	find $(PREFIX)/$(TARGET)/lib -name *.[oa] -exec $(TARGET)-strip -g {} \;
	find $(PREFIX)/lib/gcc/$(TARGET)/$(GCC_VER)/ -name *.[ao] \
		-exec $(TARGET)-strip -g {} \;
	touch $@

#######################################################################
# dist
#######################################################################

dist: $(CWD)/.stripped $(CWD)/.dist-made
dist-reset:
	rm -f $(CWD)/.dist-made

$(CWD)/.dist-made:
	-[ -d $(DISTNAME) ] && rm -rf $(DISTNAME)
	mkdir $(DISTNAME)
	cp -r $(PREFIX)/* $(DISTNAME)
	tar -jcf $(DISTNAME).tbz $(DISTNAME)
	md5sum $(DISTNAME).tbz > $(DISTNAME).tbz.md5
	touch $@

