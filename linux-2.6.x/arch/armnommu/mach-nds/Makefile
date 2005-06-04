#
# Makefile for the linux kernel.
#

# Object file lists.

obj-y                += arch.o irq.o time.o button.o

obj-$(CONFIG_NDS_TEXT_CONSOLE) += console.o

extra-y += arm7/head.o arm7/main.o arm7.bin

$(obj)/arm7.bin: $(obj)/arm7.elf
	$(OBJCOPY) -O binary $(obj)/arm7.elf $(obj)/arm7.bin
	rm -f $(obj)/head.o

$(obj)/arm7.elf: $(obj)/arm7/head.o $(obj)/arm7/main.o $(src)/arm7/arm7.lds
	$(LD) -o $(obj)/arm7.elf $(obj)/arm7/head.o $(obj)/arm7/main.o -T $(src)/arm7/arm7.lds

$(obj)/head.o: $(obj)/arm7.bin


