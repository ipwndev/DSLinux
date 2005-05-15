#
# arch/armnommu/Makefile
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright (C) 2004 by Hyok S. Choi (for uClinux/ARM 2.6)
# Copyright (C) 1995-2001 by Russell King

LDFLAGS_vmlinux	:=-p --no-undefined -X
CPPFLAGS_vmlinux.lds = -DTEXTADDR=$(TEXTADDR) -DDATAADDR=$(DATAADDR)
OBJCOPYFLAGS	:=-O binary -R .note -R .comment -S
GZFLAGS		:=-9
CFLAGS		+=-fno-builtin -D__linux__

CFLAGS		:=$(CFLAGS:-O2=-Os)

ifeq ($(CONFIG_FRAME_POINTER),y)
CFLAGS		+=-fno-omit-frame-pointer -mapcs -mno-sched-prolog
endif

ifeq ($(CONFIG_CPU_BIG_ENDIAN),y)
CFLAGS		+= -mbig-endian
AS		+= -EB
LD		+= -EB
AFLAGS		+= -mbig-endian
else
CFLAGS		+= -mlittle-endian
AS		+= -EL
LD		+= -EL
AFLAGS		+= -mlittle-endian
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
tune-$(CONFIG_CPU_ARM7V3)	:=-mtune=arm610
tune-$(CONFIG_CPU_ARM610)	:=-mtune=arm610
tune-$(CONFIG_CPU_ARM710)	:=-mtune=arm7tdmi
tune-$(CONFIG_CPU_S3C4510B)	:=-mtune=arm7tdmi
tune-$(CONFIG_CPU_ARM720T)	:=-mtune=arm7tdmi
tune-$(CONFIG_CPU_ARM740T)	:=-mtune=arm7tdmi
tune-$(CONFIG_CPU_ARM920T)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_ARM922T)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_ARM925T)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_ARM926T)	:=-mtune=arm9tdmi
tune-$(CONFIG_CPU_ARM940T)	:=-mtune=arm9tdmi

# Need -Uarm for gcc < 3.x
CFLAGS		+=-mapcs-32 $(arch-y) $(tune-y) $(call cc-option,-malignment-traps,-mshort-load-bytes) -msoft-float -Uarm
AFLAGS		+=-mapcs-32 $(arch-y) $(tune-y) -msoft-float

CHECKFLAGS	+= -D__arm__

#Default value
DATAADDR	:= .

# These are the default values for independt
DATAADDR	:= .
PROCESSOR	:= armv
head-y		:= arch/armnommu/kernel/head.o 	# default head
textaddr-y	:= 0x00008000			# default text address

# setup the machine name and the machine dependent settings
#   in alphabetical order.
machine-$(CONFIG_ARCH_ATMEL)		:= atmel
	textaddr-$(CONFIG_ARCH_ATMEL)		:= 0x01000000
machine-$(CONFIG_ARCH_ESPD_4510B)	:= espd_4510b
	textaddr-$(CONFIG_ARCH_ESPD_4510B)	:= 0x00008000
machine-$(CONFIG_ARCH_S3C3410)		:= s3c3410
	textaddr-$(CONFIG_ARCH_S3C3410)		:= 0x01020000
machine-$(CONFIG_ARCH_S3C44B0)		:= s3c44b0x
	textaddr-$(CONFIG_ARCH_S3C44B0)		:= 0x0c008000
machine-$(CONFIG_ARCH_S5C7375) 		:= s5c7375
	textaddr-$(CONFIG_ARCH_S5C7375) 	:= 0x00008000
machine-$(CONFIG_ARCH_S3C24A0)		:= s3c24a0
	textaddr-$(CONFIG_ARCH_S3C24A0) 	:= 0x10008000
machine-$(CONFIG_ARCH_P2001) 		:= p2001
	textaddr-$(CONFIG_ARCH_P2001)	 	:= 0x40100000
machine-$(CONFIG_ARCH_NDS) 		:= nds
	textaddr-$(CONFIG_ARCH_NDS)	 	:= 0x08000000
	textaddr-$(CONFIG_NDS_NDS)	 	:= 0x02005000

# set the environment variables and export
MACHINE  := $(machine-y)
TEXTADDR := $(textaddr-y)
ifeq ($(incdir-y),)
incdir-y := $(MACHINE)
endif
INCDIR   := arch-$(incdir-y)
  
export	MACHINE PROCESSOR TEXTADDR GZFLAGS CFLAGS_BOOT

# Does the machine has its own head.S ?
HEADMACH 	:= arch/armnommu/mach-$(MACHINE)/head.S
ifeq ($(HEADMACH), $(wildcard $(HEADMACH)))
head-y		:= arch/armnommu/mach-$(MACHINE)/head.o
HEADMACH	:= ../mach-$(MACHINE)/head.o
else
HEADMACH 	:=
endif
export HEADMACH

# add the init_task to head
head-y		+= arch/armnommu/kernel/init_task.o

# Do we have FASTFPE?
FASTFPE		:=arch/arm/fastfpe
ifeq ($(FASTFPE),$(wildcard $(FASTFPE)))
FASTFPE_OBJ	:=$(FASTFPE)/
endif

# If we have a machine-specific directory, then include it in the build.
core-y				+= arch/armnommu/kernel/ arch/armnommu/mm/ arch/arm/common/
ifneq ($(MACHINE),)
core-y				+= arch/armnommu/mach-$(MACHINE)/
endif
core-$(CONFIG_FPE_NWFPE)	+= arch/arm/nwfpe/
core-$(CONFIG_FPE_FASTFPE)	+= $(FASTFPE_OBJ)
core-$(CONFIG_VFP)		+= arch/arm/vfp/

drivers-$(CONFIG_OPROFILE)      += arch/arm/oprofile/

libs-y				+= arch/arm/lib/

# Default target when executing plain make
all: uCImage

boot := arch/armnommu/boot

#	Update machine arch and proc symlinks if something which affects
#	them changed.  We use .arch to indicate when they were updated
#	last, otherwise make uses the target directory mtime.

include/asm-armnommu/.arch: $(wildcard include/config/arch/*.h) include/config/MARKER
	@echo '  SYMLINK include/asm-armnommu/arch -> include/asm-armnommu/$(INCDIR)'
ifneq ($(KBUILD_SRC),)
	$(Q)mkdir -p include/asm-armnommu
	$(Q)ln -fsn $(srctree)/include/asm-armnommu/$(INCDIR) include/asm-armnommu/arch
else
	$(Q)ln -fsn $(INCDIR) include/asm-armnommu/arch
endif
	@touch $@

prepare: maketools include/asm-armnommu/.arch

.PHONY: maketools FORCE
maketools: include/asm-armnommu/constants.h include/linux/version.h FORCE
	$(Q)$(MAKE) $(build)=arch/arm/tools include/asm-arm/mach-types.h

# Convert bzImage to zImage
bzImage: zImage

zImage Image bootpImage uImage: vmlinux
	$(Q)$(MAKE) $(build)=$(boot) $(boot)/$@

uCImage: vmlinux linux.bin

linux.bin: vmlinux FORCE
	@$(OBJCOPY) $(OBJCOPYFLAGS) vmlinux $@
	@echo '  Kernel: $@ is ready'
	
zinstall install: vmlinux
	$(Q)$(MAKE) $(build)=$(boot) $@

CLEAN_FILES += include/asm-armnommu/constants.h* include/asm-arm/mach-types.h \
			include/asm-armnommu/arch include/asm-armnommu/.arch \
			linux.bin linux arch/armnommu/kernel/entry-macro.S

# We use MRPROPER_FILES and CLEAN_FILES now
archclean:
	$(Q)$(MAKE) $(clean)=$(boot)

# My testing targets (bypasses dependencies)
bp:;	$(Q)$(MAKE) $(build)=$(boot) $(boot)/bootpImage
i zi:;	$(Q)$(MAKE) $(build)=$(boot) $@

arch/$(ARCH)/kernel/asm-offsets.s: include/asm include/linux/version.h \
				   include/asm-armnommu/.arch

include/asm-$(ARCH)/constants.h: arch/$(ARCH)/kernel/asm-offsets.s
	$(call filechk,gen-asm-offsets)

define archhelp
  echo  '* zImage        - Compressed kernel image (arch/$(ARCH)/boot/zImage)'
  echo  '  Image         - Uncompressed kernel image (arch/$(ARCH)/boot/Image)'
  echo  '  uCImage	 - Uncompressed kernel and the stripped binary (linux.bin)'
  echo  '  bootpImage    - Combined zImage and initial RAM disk' 
endef
