#
# Makefile for the linux kernel.
#

# Object file lists.

obj-y                += arch.o irq.o time.o button.o fifo.o

obj-$(CONFIG_NDS_TEXT_CONSOLE) += console.o

extra-y += arm7/head.o arm7/main.o arm7.bin

$(obj)/arm7.bin: $(obj)/arm7.elf
	$(OBJCOPY) -O binary $(obj)/arm7.elf $(obj)/arm7.bin
	rm -f $(obj)/head.o

arm7obj = $(obj)/arm7/head.o $(obj)/arm7/main.o 

$(obj)/arm7.elf: $(arm7obj) $(src)/arm7/arm7.lds
	$(CC) -msoft-float -nostartfiles -nostdlib -o $(obj)/arm7.elf $(arm7obj) -lgcc -T $(src)/arm7/arm7.lds

$(obj)/head.o: $(obj)/arm7.bin


