##
# arch/armnommu/kernel/Makefile
# modified for uClinux by Hyok S. Choi, 2004
#

AFLAGS_head.o := -DTEXTADDR=$(TEXTADDR)

# Object file lists.

obj-y		:= compat.o dma.o entry.o entry-common.o irq.o   \
		   process.o ptrace.o semaphore.o setup.o signal.o sys_arm.o \
		   time.o traps.o

obj-$(CONFIG_APM)		+= apm.o
obj-$(CONFIG_MODULES)		+= armksyms.o module.o
obj-$(CONFIG_ISA_DMA)		+= dma-isa.o
obj-$(CONFIG_PCI)		+= bios32.o

# test if we don't need to compile the head.S, here.
ifeq ($(HEADMACH),)
head-y			:= head.o
else
head-y			:= $(HEADMACH)
endif

extra-y := $(head-y) init_task.o vmlinux.lds

# Spell out some dependencies that aren't automatically figured out
$(obj)/entry.o: 	arch/arm/kernel/entry-header.S include/asm-armnommu/constants.h \
			$(obj)/../mach-$(MACHINE)/entry-macro.S $(obj)/entry-macro.S
$(obj)/entry-common.o: 	arch/arm/kernel/entry-header.S include/asm-armnommu/constants.h \
			$(obj)/calls.S
$(obj)/entry-macro.S:	$(obj)/../mach-$(MACHINE)/entry-macro.S
	@echo '  Making $(obj)/entry-macro.S -> ../mach-$(MACHINE)/entry-macro.S symlink'
	@rm -f entry-macro.S
	@ln -sf ../mach-$(MACHINE)/entry-macro.S $(obj)/entry-macro.S
	@touch $@
$(obj)/apm.o:		arch/arm/kernel/apm.c FORCE
	$(call if_changed_dep,cc_o_c)
$(obj)/bios32.o:	arch/arm/kernel/bios32.c FORCE
	$(call if_changed_dep,cc_o_c)
$(obj)/compat.o:	arch/arm/kernel/compat.c FORCE
	$(call if_changed_dep,cc_o_c)
$(obj)/dma-isa.o:	arch/arm/kernel/dma-isa.c FORCE
	$(call if_changed_dep,cc_o_c)
$(obj)/dma.o:		arch/arm/kernel/dma.c FORCE
	$(call if_changed_dep,cc_o_c)
$(obj)/irq.o:		arch/arm/kernel/irq.c FORCE
	$(call if_changed_dep,cc_o_c)
$(obj)/ptrace.o:	arch/arm/kernel/ptrace.c arch/arm/kernel/ptrace.h FORCE
	$(call if_changed_dep,cc_o_c)
$(obj)/semaphore.o:	arch/arm/kernel/semaphore.c FORCE
	$(call if_changed_dep,cc_o_c)
$(obj)/signal.o:	arch/arm/kernel/signal.c FORCE
	$(call if_changed_dep,cc_o_c)
$(obj)/time.o:		arch/arm/kernel/time.c FORCE
	$(call if_changed_dep,cc_o_c)
$(obj)/traps.o:		arch/arm/kernel/traps.c FORCE
	$(call if_changed_dep,cc_o_c)

