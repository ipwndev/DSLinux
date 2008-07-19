#
# arch/arm/Makefile
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 1995-2001 by Russell King
# Copyright (C) 2004 by Hyok S. Choi (for uClinux/ARM 2.6)

LDFLAGS_vmlinux	:=-p --no-undefined -X
CPPFLAGS_vmlinux.lds = -DTEXTADDR=$(TEXTADDR) -DDATAADDR=$(DATAADDR)
OBJCOPYFLAGS	:=-O binary -R .note -R .comment -S
GZFLAGS		:=-9
#CFLAGS		+=-pipe

# Do not use arch/arm/defconfig - it's always outdated.
# Select a platform that is kept up-to-date
KBUILD_DEFCONFIG := versatile_defconfig

# defines filename extension for mmu type, if any.
ifeq ($(CONFIG_MMU),)
MMUEXT		:= -nommu
endif

ifeq ($(CONFIG_FRAME_POINTER),y)
CFLAGS		+=-fno-omit-frame-pointer -mapcs -mno-sched-prolog
endif

ifeq ($(CONFIG_CPU_BIG_ENDIAN),y)
CPPFLAGS	+= -mbig-endian
AS		+= -EB
LD		+= -EB
else
CPPFLAGS	+= -mlittle-endian
AS		+= -EL
LD		+= -EL
endif

comma = ,

# This selects which instruction set is used.
# Note that GCC does not numerically define an architecture version
# macro, but instead defines a whole series of macros which makes
# testing for a specific architecture or later rather impossible.
arch-$(CONFIG_CPU_32v6)		:=-D__LINUX_ARM_ARCH__=6 $(call cc-option,-march=armv6,-march=armv5t -Wa$(comma)-march=armv6)
arch-$(CONFIG_CPU_32v5)		:=-D__LINUX_ARM_ARCH__=5 $(call cc-option,-march=armv5te,-march=armv4)
arch-$(CONFIG_CPU_32v4)		:=-D__LINUX_ARM_ARCH__=4 -march=armv4
arch-$(CONFIG_CPU_32v3)		:=-D__LINUX_ARM_ARCH__=3 -march=armv3

# This selects how we optimise for the processor.
tune-$(CONFIG_CPU_ARM610)	:=-mtune=arm610
tune-$(CONFIG_CPU_ARM710)	:=-mtune=arm710
tune-$(CONFIG_CPU_ARM7TDMI)	:=-mtune=arm7tdmi
tune-$(CONFIG_CPU_S3C4510B)	:=-mtune=arm7tdmi
tune-$(CONFIG_CPU_ARM720T)	:=-mtune=arm7tdmi
tune-$(CONFIG_CPU_ARM740T)	:=-mtune=arm7tdmi
tune-$(CONFIG_CPU_ARM9TDMI)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_ARM920T)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_ARM922T)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_ARM925T)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_ARM926T)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_ARM940T)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_ARM946T)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_SA110)	:=-mtune=strongarm110
tune-$(CONFIG_CPU_SA1100)	:=-mtune=strongarm1100
tune-$(CONFIG_CPU_XSCALE)	:=$(call cc-option,-mtune=xscale,-mtune=strongarm110) -Wa,-mcpu=xscale
tune-$(CONFIG_CPU_V6)		:=$(call cc-option,-mtune=arm1136j-s,-mtune=strongarm)

# Need -Uarm for gcc < 3.x
CFLAGS_ABI	:=$(call cc-option,-mapcs-32) $(call cc-option,-mno-thumb-interwork,)
CFLAGS		+=$(CFLAGS_ABI) $(arch-y) $(tune-y) $(call cc-option,-mshort-load-bytes,$(call cc-option,-malignment-traps,)) -msoft-float -Uarm
AFLAGS		+=$(CFLAGS_ABI) $(arch-y) $(tune-y) -msoft-float

# Add a compiler option to substitute all 8-bit-writes (strb)
# with swpb, to be able to use 16bit memory as general memory.
ifeq ($(CONFIG_NDS_ROM8BIT),y)
CFLAGS += -mswp-byte-writes
# don't erase the assembler files. For debugging purposes only.
# CFLAGS += -save-temps -fverbose-asm
endif

CHECKFLAGS	+= -D__arm__

#Default value
head-y		:= arch/arm/kernel/head$(MMUEXT).o arch/arm/kernel/init_task.o
ifeq ($(CONFIG_MMU),y)
textaddr-y	:= 0xC0008000
else
textaddr-y	:= 0x00008000
endif

 machine-$(CONFIG_ARCH_RPC)	   := rpc
 machine-$(CONFIG_ARCH_EBSA110)	   := ebsa110
 machine-$(CONFIG_ARCH_CLPS7500)   := clps7500
  incdir-$(CONFIG_ARCH_CLPS7500)   := cl7500
 machine-$(CONFIG_FOOTBRIDGE)	   := footbridge
  incdir-$(CONFIG_FOOTBRIDGE)	   := ebsa285
textaddr-$(CONFIG_ARCH_CO285)	   := 0x60008000
 machine-$(CONFIG_ARCH_CO285)	   := footbridge
  incdir-$(CONFIG_ARCH_CO285)	   := ebsa285
 machine-$(CONFIG_ARCH_SHARK)	   := shark
 machine-$(CONFIG_ARCH_SA1100)	   := sa1100
ifeq ($(CONFIG_ARCH_SA1100),y)
# SA1111 DMA bug: we don't want the kernel to live in precious DMA-able memory
textaddr-$(CONFIG_SA1111)	   := 0xc0208000
endif
 machine-$(CONFIG_ARCH_PXA)	   := pxa
 machine-$(CONFIG_ARCH_L7200)	   := l7200
 machine-$(CONFIG_ARCH_INTEGRATOR) := integrator
 machine-$(CONFIG_ARCH_CAMELOT)	   := epxa10db
textaddr-$(CONFIG_ARCH_CLPS711X)   := 0xc0028000
 machine-$(CONFIG_ARCH_CLPS711X)   := clps711x
textaddr-$(CONFIG_ARCH_FORTUNET)   := 0xc0008000
 machine-$(CONFIG_ARCH_IOP3XX)	   := iop3xx
 machine-$(CONFIG_ARCH_IXP4XX)	   := ixp4xx
 machine-$(CONFIG_ARCH_IXP2000)    := ixp2000
 machine-$(CONFIG_ARCH_OMAP1)	   := omap1
  incdir-$(CONFIG_ARCH_OMAP)	   := omap
 machine-$(CONFIG_ARCH_S3C2410)	   := s3c2410
 machine-$(CONFIG_ARCH_LH7A40X)	   := lh7a40x
 machine-$(CONFIG_ARCH_VERSATILE)  := versatile
 machine-$(CONFIG_ARCH_IMX)	   := imx
 machine-$(CONFIG_ARCH_H720X)	   := h720x
 machine-$(CONFIG_ARCH_AAEC2000)   := aaec2000
 machine-$(CONFIG_ARCH_ATMEL)	   := atmel
textaddr-$(CONFIG_ARCH_ATMEL)	   := 0x01000000
 machine-$(CONFIG_ARCH_ESPD_4510B) := espd_4510b
textaddr-$(CONFIG_ARCH_ESPD_4510B) := 0x00008000
 machine-$(CONFIG_ARCH_S3C3410)	   := s3c3410
textaddr-$(CONFIG_ARCH_S3C3410)	   := 0x01020000
 machine-$(CONFIG_ARCH_S3C44B0)	   := s3c44b0x
textaddr-$(CONFIG_ARCH_S3C44B0)	   := 0x0c008000
 machine-$(CONFIG_ARCH_S5C7375)	   := s5c7375
textaddr-$(CONFIG_ARCH_S5C7375)	   := 0x00008000
 machine-$(CONFIG_ARCH_S3C24A0)	   := s3c24a0
ifeq ($(CONFIG_MMU),)
textaddr-$(CONFIG_ARCH_S3C24A0)	   := 0x10008000
endif
 machine-$(CONFIG_ARCH_P2001)	   := p2001
textaddr-$(CONFIG_ARCH_P2001)	   := 0x40100000
 machine-$(CONFIG_ARCH_NDS)	   := nds
textaddr-$(CONFIG_ARCH_NDS)	   := 0x02000000

ifeq ($(CONFIG_ARCH_EBSA110),y)
# This is what happens if you forget the IOCS16 line.
# PCMCIA cards stop working.
CFLAGS_3c589_cs.o :=-DISA_SIXTEEN_BIT_PERIPHERAL
export CFLAGS_3c589_cs.o
endif

TEXTADDR := $(textaddr-y)
ifeq ($(CONFIG_XIP_KERNEL),y)
  DATAADDR := $(TEXTADDR)
  xipaddr-$(CONFIG_ARCH_CO285) := 0x5f000000
  xipaddr-$(CONFIG_ARCH_NDS)   := $(CONFIG_XIP_PHYS_ADDR)
  # FIXME: a default value for an implementation-dependent value is bogus.
  xipaddr-y ?= 0xbf000000
  # Replace phys addr with virt addr while keeping offset from base.
  # FIXME: a default formula for the implementation-dependent PHYS->VIRT mapping is bogus.
  TEXTADDR := $(shell echo $(CONFIG_XIP_PHYS_ADDR) $(xipaddr-y) | \
                      awk --non-decimal-data '/[:xdigit:]/ \
                          { printf("0x%x\n", and($$1, 0x000fffff) + $$2) }' )
  # set TEXTADDR for me, leaving the crap alone.
  ifeq ($(CONFIG_ARCH_NDS),y)
    TEXTADDR := $(xipaddr-y)
  endif
endif

ifeq ($(incdir-y),)
incdir-y := $(machine-y)
endif
INCDIR   := arch-$(incdir-y)
ifneq ($(machine-y),)
MACHINE  := arch/arm/mach-$(machine-y)/
else
MACHINE  :=
endif

# set HEAD as default (kernel/head.o)
HEAD		:= head$(MMUEXT).o
# Check if an overriding head.S exist for non-paged kernel
ifeq ($(CONFIG_MMU),)
ifeq ($(MACHINE)head.S, $(wildcard $(MACHINE)head.S))
head-y		:= $(MACHINE)head.o arch/arm/kernel/init_task.o
HEAD		:= ../mach-$(machine-y)/head.o
endif
endif

export	MACHINE HEAD MMUEXT TEXTADDR GZFLAGS

# Do we have FASTFPE?
FASTFPE		:=arch/arm/fastfpe
ifeq ($(FASTFPE),$(wildcard $(FASTFPE)))
FASTFPE_OBJ	:=$(FASTFPE)/
endif

# If we have a machine-specific directory, then include it in the build.
core-y				+= $(MACHINE)
core-y				+= arch/arm/kernel/ arch/arm/mm/ arch/arm/common/
core-$(CONFIG_FPE_NWFPE)	+= arch/arm/nwfpe/
core-$(CONFIG_FPE_FASTFPE)	+= $(FASTFPE_OBJ)
core-$(CONFIG_VFP)		+= arch/arm/vfp/

# If we have a common platform directory, then include it in the build.
core-$(CONFIG_ARCH_OMAP)	+= arch/arm/plat-omap/

drivers-$(CONFIG_OPROFILE)      += arch/arm/oprofile/
drivers-$(CONFIG_ARCH_CLPS7500)	+= drivers/acorn/char/
drivers-$(CONFIG_ARCH_L7200)	+= drivers/acorn/char/

libs-y				+= arch/arm/lib/

# Default target when executing plain make
ifeq ($(CONFIG_MMU),y)
ifeq ($(CONFIG_XIP_KERNEL),y)
all: xipImage
else
all: zImage
endif
else
all: Image
endif

boot := arch/arm/boot

#	Update machine arch and proc symlinks if something which affects
#	them changed.  We use .arch to indicate when they were updated
#	last, otherwise make uses the target directory mtime.

include/asm-arm/.arch: $(wildcard include/config/arch/*.h) include/config/MARKER
	@echo '  SYMLINK include/asm-arm/arch -> include/asm-arm/$(INCDIR)'
ifneq ($(KBUILD_SRC),)
	$(Q)mkdir -p include/asm-arm
	$(Q)ln -fsn $(srctree)/include/asm-arm/$(INCDIR) include/asm-arm/arch
else
	$(Q)ln -fsn $(INCDIR) include/asm-arm/arch
endif
	@touch $@

archprepare: maketools

.PHONY: maketools FORCE
maketools: include/linux/version.h include/asm-arm/.arch FORCE
	$(Q)$(MAKE) $(build)=arch/arm/tools include/asm-arm/mach-types.h

# Convert bzImage to zImage
bzImage: zImage

zImage Image xipImage bootpImage uImage: vmlinux
	$(Q)$(MAKE) $(build)=$(boot) MACHINE=$(MACHINE) $(boot)/$@

zinstall install: vmlinux
	$(Q)$(MAKE) $(build)=$(boot) MACHINE=$(MACHINE) $@

CLEAN_FILES += include/asm-arm/mach-types.h \
	       include/asm-arm/arch include/asm-arm/.arch

# We use MRPROPER_FILES and CLEAN_FILES now
archclean:
	$(Q)$(MAKE) $(clean)=$(boot)

# My testing targets (bypasses dependencies)
bp:;	$(Q)$(MAKE) $(build)=$(boot) MACHINE=$(MACHINE) $(boot)/bootpImage
i zi:;	$(Q)$(MAKE) $(build)=$(boot) MACHINE=$(MACHINE) $@


define archhelp
  echo  '* zImage        - Compressed kernel image (arch/$(ARCH)/boot/zImage)'
  echo  '  Image         - Uncompressed kernel image (arch/$(ARCH)/boot/Image)'
  echo  '* xipImage      - XIP kernel image, if configured (arch/$(ARCH)/boot/xipImage)'
  echo  '  bootpImage    - Combined zImage and initial RAM disk' 
  echo  '                  (supply initrd image via make variable INITRD=<path>)'
  echo  '  install       - Install uncompressed kernel'
  echo  '  zinstall      - Install compressed kernel'
  echo  '                  Install using (your) ~/bin/installkernel or'
  echo  '                  (distribution) /sbin/installkernel or'
  echo  '                  install to $$(INSTALL_PATH) and run lilo'
endef
