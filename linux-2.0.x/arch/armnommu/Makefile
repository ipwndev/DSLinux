#
# arch/arm/Makefile
#
# This file is included by the global makefile so that you can add your own
# architecture-specific flags and dependencies. Remember to do have actions
# for "archclean" and "archdep" for cleaning up and making dependencies for
# this architecture
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 1995, 1996 by Russell King

#CFLAGS += -DDEBUG
CFLAGS_DEBUG = -g


ifeq ($(CONFIG_CPU_ARM2),y)
PROCESSOR	 = armo
ifeq ($(CONFIG_BINUTILS_NEW),y)
CFLAGS_PROC	+= -mcpu=arm2
else
CFLAGS_PROC	+= -m2
endif
endif

ifeq ($(CONFIG_CPU_ARM3),y)
PROCESSOR	 = armo
ifeq ($(CONFIG_BINUTILS_NEW),y)
CFLAGS_PROC	+= -mcpu=arm3
else
CFLAGS_PROC	+= -m3
endif
endif

ifeq ($(CONFIG_CPU_ARM6),y)
PROCESSOR	 = armv
ifeq ($(CONFIG_BINUTILS_NEW),y)
CFLAGS_PROC	+= -mcpu=arm6
else
CFLAGS_PROC	+= -m6
endif
endif


ifeq ($(CONFIG_CPU_ARM7),y)
PROCESSOR	 = armv
ifeq ($(CONFIG_BINUTILS_NEW),y)
CFLAGS_PROC	+= -mcpu=arm7tdmi -fno-omit-frame-pointer -fno-strict-aliasing
else
CFLAGS_PROC	+= -m6
endif
endif



ifeq ($(CONFIG_CPU_SA110),y)
PROCESSOR	 = armv
ifeq ($(CONFIG_BINUTILS_NEW),y)
CFLAGS_PROC	+= -mcpu=strongarm110
else
CFLAGS_PROC	+= -m6
endif
endif

# Processor Architecture
# CFLAGS_PROC - processor dependent CFLAGS
# PROCESSOR - processor type
# TEXTADDR - Uncompressed kernel link text address
# ZTEXTADDR - Compressed kernel link text address
# ZRELADDR - Compressed kernel relocating address (point at which uncompressed kernel is loaded).
#

ifeq ($(CONFIG_ARCH_TRIO), y)
HEAD		:= arch/armnommu/kernel/head-arm-aplio.o
COMPRESSED_HEAD  = head.o
MACHINE = TRIO
else
ifeq ($(CONFIG_ARCH_ATMEL), y)
HEAD		:= arch/armnommu/kernel/head-arm-atmel.o
COMPRESSED_HEAD  = head.o
MACHINE = ATMEL
else
ifeq ($(CONFIG_ARCH_NETARM), y)
HEAD		:= arch/armnommu/kernel/head-arm-netarm.o
COMPRESSED_HEAD  = head.o
else
HEAD		:= arch/armnommu/kernel/head-$(PROCESSOR).o
COMPRESSED_HEAD  = head.o
endif
endif
endif
ifeq ($(PROCESSOR),armo)
ifeq ($(CONFIG_BINUTILS_NEW),y)
CFLAGS_PROC	+= -mapcs-26 -mshort-load-bytes
endif
TEXTADDR	 = 0x02080000
ZTEXTADDR	 = 0x01800000
ZRELADDR	 = 0x02080000
endif

ifeq ($(CONFIG_ARCH_A5K),y)
MACHINE 	 = a5k
COMPRESSED_EXTRA = $(TOPDIR)/arch/arm/lib/ll_char_wr.o
endif

ifeq ($(CONFIG_ARCH_ARC),y)
MACHINE 	 = arc
COMPRESSED_EXTRA = $(TOPDIR)/arch/arm/lib/ll_char_wr.o
endif

ifeq ($(PROCESSOR),armv)
ifeq ($(CONFIG_BINUTILS_NEW),y)
CFLAGS_PROC	+=
# -mapcs-32 -mapcs -mapcs-frame -mapcs-stack-check
endif
TEXTADDR	 = 0xC0008000
endif

ifeq ($(CONFIG_ARCH_TRIO),y)
MACHINE 	 = trio
#TEXTADDR	 = 0x8000
TEXTADDR	 = 0x02040000
#TEXTADDR	 = 0x2000
endif

ifeq ($(CONFIG_ARCH_NETARM),y)
MACHINE 	 = netarm
TEXTADDR	 = 0x1000
endif

ifeq ($(CONFIG_ARCH_ATMEL),y)
ifeq ($(CONFIG_WILL_BOOT_FROM_FLASH),y)
MACHINE 	 = atmel
TEXTADDR	 = 0x1020000
else
MACHINE 	 = atmel
TEXTADDR	 = 0x02000000
endif
endif

ifeq ($(CONFIG_ARCH_RPC),y)
MACHINE 	 = rpc
COMPRESSED_EXTRA = $(TOPDIR)/arch/arm/lib/ll_char_wr.o
ZTEXTADDR	 = 0x10008000
ZRELADDR	 = 0x10008000
endif

ifeq ($(CONFIG_ARCH_EBSA110),y)
MACHINE 	 = ebsa110
ZTEXTADDR	 = 0x00008000
ZRELADDR	 = 0x00008000
endif


ifeq ($(CONFIG_ARCH_NEXUSPCI),y)
MACHINE 	 = nexuspci
TEXTADDR	 = 0xc0000000
ZTEXTADDR	 = 0x40200000
ZRELADDR	 = 0x40000000
COMPRESSED_EXTRA = $(TOPDIR)/arch/arm/lib/ll_char_wr_scc.o
COMPRESSED_HEAD  = head-nexuspci.o
endif

ifeq ($(CONFIG_ARCH_GBA),y)
HEAD		 := arch/armnommu/kernel/head-arm-gba.o
MACHINE 	 = gba
TEXTADDR	 = 0x08000000
ARCHLDFLAGS      = -Tdata=0x02000000 -T $(TOPDIR)/arch/armnommu/vmlinux-armv.lds.in
endif

OBJDUMP		 = $(CROSS_COMPILE)objdump
PERL		 = perl
CPP		 = $(CC) -E
ARCHCC		:= $(word 1,$(CC))
GCCLIB		:= `$(CC) $(CFLAGS_PROC) --print-libgcc-file-name`
HOSTCFLAGS	:= $(CFLAGS:-fomit-frame-pointer=)
ifeq ($(CONFIG_FRAME_POINTER),y)
CFLAGS		:= $(CFLAGS_PROC) $(CFLAGS:-fomit-frame-pointer=) -pipe -DNO_MM
else
CFLAGS		:= $(CFLAGS_PROC) $(CFLAGS) -fno-omit-frame-pointer -pipe -DNO_MM
endif
LINKFLAGS	 = -Ttext $(TEXTADDR) $(ARCHLDFLAGS)
ZLINKFLAGS	 = -Ttext $(ZTEXTADDR)

SUBDIRS		:= $(SUBDIRS) arch/armnommu/kernel arch/armnommu/mm arch/armnommu/lib
ARCHIVES	:= arch/armnommu/kernel/kernel.o arch/armnommu/mm/mm.o \
		   $(ARCHIVES)
LIBS		:= arch/armnommu/lib/lib.a $(LIBS) $(GCCLIB)

targetsymlinks::
	$(RM) include/asm-armnommu/arch include/asm-armnommu/proc
	(cd include/asm-armnommu; ln -sf arch-$(MACHINE) arch; ln -sf proc-$(PROCESSOR) proc)

arch/armnommu/kernel: dummy
	$(MAKE) linuxsubdirs SUBDIRS=arch/armnommu/kernel

arch/armnommu/mm: dummy
	$(MAKE) linuxsubdirs SUBDIRS=arch/armnommu/mm

MAKEBOOT = $(MAKE) -C arch/$(ARCH)/boot

zImage: vmlinux
	@$(MAKEBOOT) zImage

zinstall: vmlinux
	@$(MAKEBOOT) zinstall

Image: vmlinux
	@$(MAKEBOOT) Image

install: vmlinux
	@$(MAKEBOOT) install

# My testing targets (that short circuit a few dependencies)
#
zImg:;	@$(MAKEBOOT) zImage
Img:;	@$(MAKEBOOT) Image
i:;	@$(MAKEBOOT) install
zi:;	@$(MAKEBOOT) zinstall

archclean:
	@$(MAKEBOOT) clean
	@$(MAKE) -C arch/armnommu/lib clean

archdep: targetsymlinks
	@$(MAKEBOOT) dep

#sed -e /^MACHINE..*=/s,= .*,= rpc,;/^PROCESSOR..*=/s,= .*,= armv, linux/arch/armnommu/Makefile.normal
