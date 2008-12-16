#
# Makefile for the linux kernel.
#

# Object file lists.

obj-y                += arch.o irq.o time.o button.o fifo.o rtc.o disable-itcm-writes.o

obj-$(CONFIG_NDS_TEXT_CONSOLE) += console.o
obj-$(CONFIG_NDS_SOUNDTEST) += soundtest.o
obj-$(CONFIG_NDS_BLK_M3CF) += m3cf.o
obj-$(CONFIG_NDS_BLK_SCCF) += sccf.o
obj-$(CONFIG_NDS_ROM8BIT) += gbaram.o

m3cf-y := m3cf_c.o m3cf_s.o
sccf-y := sccf_c.o sccf_s.o

extra-y += arm7/head.o
extra-y += arm7/main.o
extra-y += arm7/time.o
extra-y += arm7/spi.o
extra-y += arm7/sound.o
extra-y += arm7/microphone.o
extra-y += arm7/wifi.o
extra-y += arm7/divide.o
extra-y += arm7.bin

# this does not seem to have any effect (why?):
$(obj)/head.o: $(obj)/arm7.bin

# since head.o depends on arm7.bin, but upper-level Makefiles don't get this
# for some reason, we re-create head.o whenever arm7.bin is being re-created.
$(obj)/arm7.bin: $(obj)/arm7.elf
	$(OBJCOPY) -O binary $(obj)/arm7.elf $(obj)/arm7.bin
	rm -f $(obj)/head.o

arm7obj = $(obj)/arm7/head.o $(obj)/arm7/main.o $(obj)/arm7/time.o $(obj)/arm7/spi.o $(obj)/arm7/sound.o $(obj)/arm7/microphone.o $(obj)/arm7/wifi.o $(obj)/arm7/divide.o

$(obj)/arm7.elf: $(arm7obj) $(src)/arm7/arm7.lds
	$(LD) -nostartfiles -nostdlib -o $(obj)/arm7.elf $(arm7obj) -T $(src)/arm7/arm7.lds

clean:
	rm -f arm7.elf arm7.bin *.o
