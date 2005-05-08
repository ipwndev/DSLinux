#
# Makefile for the kernel character device drivers.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now inherited from the
# parent makes..
#

SUB_DIRS     := 
MOD_SUB_DIRS := $(SUB_DIRS)
ALL_SUB_DIRS := $(SUB_DIRS) ftape

#
# This file contains the font map for the default (hardware) font
#
FONTMAPFILE = cp437.uni

L_TARGET := char.a
M_OBJS   :=
L_OBJS   := tty_io.o n_tty.o tty_ioctl.o pty.o mem.o

ifdef CONSOLE_BAUD_RATE
CFLAGS += -DCONSOLE_BAUD_RATE=$(CONSOLE_BAUD_RATE)
endif
ifdef CONSOLE_PORT
CFLAGS += -DCONSOLE_PORT=$(CONSOLE_PORT)
endif

LX_OBJS += random.o

ifeq ($(CONFIG_SERIAL),y)
  ifndef CONFIG_SUN_SERIAL
  LX_OBJS += serial.o
  endif
else
  ifeq ($(CONFIG_SERIAL),m)
  MX_OBJS += serial.o
  endif
endif

ifeq ($(CONFIG_LEON_SERIAL),y)
L_OBJS += LEONserial.o
endif

ifeq ($(CONFIG_NIOS_SERIAL),y)
L_OBJS += NIOSserial.o
endif

ifeq ($(CONFIG_NIOS_SPI),y)
L_OBJS += spi.o
endif

ifeq ($(CONFIG_COLDFIRE_SERIAL),y)
L_OBJS += mcfserial.o
else
 ifeq ($(CONFIG_COLDFIRE_SERIAL),m)
 M_OBJS += mcfserial.o
 endif
endif

ifeq ($(CONFIG_SH7615_SERIAL),y)
L_OBJS += SH7615serial.o
endif

ifeq ($(CONFIG_MCF_MBUS),y)
L_OBJS += mbus.o
endif

ifeq ($(CONFIG_MCF_QSPI),y)
L_OBJS += mcf_qspi.o
else
 ifeq ($(CONFIG_MCF_QSPI),m)
 M_OBJS += mcf_qspi.o
 endif
endif

ifeq ($(CONFIG_DS1302),y)
L_OBJS += ds1302.o
endif

ifeq ($(CONFIG_EXP),y)
L_OBJS += exp.o
endif

ifeq ($(CONFIG_WATCHDOG),y)
L_OBJS += mcfwatchdog.o
endif

ifeq ($(CONFIG_MAGIC_SYSRQ),y)
L_OBJS += sysrq.o
endif

ifeq ($(CONFIG_68302_SERIAL),y)
L_OBJS += 68302serial.o
else
  ifeq ($(CONFIG_68302_SERIAL),m)
  M_OBJS += 68302serial.o
  endif
endif

ifeq ($(CONFIG_68328_SERIAL),y)
L_OBJS += 68328serial.o
else
  ifeq ($(CONFIG_68328_SERIAL),m)
  M_OBJS += 68328serial.o
  endif
endif

ifeq ($(CONFIG_LC7981),y)
L_OBJS += lc7981.o
endif

ifeq ($(CONFIG_68332_SERIAL),y)
L_OBJS += 68332serial.o
else
  ifeq ($(CONFIG_68332_SERIAL),m)
  M_OBJS += 68332serial.o
  endif
endif

ifeq ($(CONFIG_68332_TPU),y)
L_OBJS += 68332dio.o
endif

ifeq ($(CONFIG_68332_PORTF),y)
L_OBJS += 68332dio.o
endif

ifeq ($(CONFIG_68332_PORTE),y)
L_OBJS += 68332dio.o
endif

ifeq ($(CONFIG_MAX311X_SERIAL),y)
L_OBJS += max311Xserial.o
endif

ifeq ($(CONFIG_SERIAL_ATMEL),y)
L_OBJS += serial-atmel.o
else
  ifeq ($(CONFIG_SERIAL_ATMEL),m)
  M_OBJS += serial-atmel.o
  endif
endif

ifeq ($(CONFIG_LED_ATMEL),y)
L_OBJS += blip-led.o
else
  ifeq ($(CONFIG_LED_ATMEL),m)
  M_OBJS += blip-led.o
  endif
endif

ifeq ($(CONFIG_GBATXT),y)
L_OBJS += gbatxt.o
else
  ifeq ($(CONFIG_GBATXT),m)
  M_OBJS += gbatxt.o
  endif
endif

ifeq ($(CONFIG_SC28L91),y)
L_OBJS += serial-sc28l91.o
else
  ifeq ($(CONFIG_SC28L91),m)
  M_OBJS += serial-sc28l91.o
  endif
endif

ifeq ($(CONFIG_68328_DIGI),y)
M = y
L_OBJS += mc68328digi.o
else
  ifeq ($(CONFIG_68328_DIGI),m)
  M_OBJS += mc68328digi.o
  MM = n
  endif
endif

ifeq ($(CONFIG_68332_KEYBOARD),y)
  L_OBJS += 68332keyboard.o defkeymap.o
endif

ifeq ($(CONFIG_KEYBOARD),y)
ifndef CONFIG_SUN_KEYBOARD
  L_OBJS += keyboard.o defkeymap.o
endif
endif

ifeq ($(CONFIG_DIGI),y)
L_OBJS += pcxx.o
else
  ifeq ($(CONFIG_DIGI),m)
  M_OBJS += pcxx.o
  endif
endif

ifeq ($(CONFIG_CYCLADES),y)
L_OBJS += cyclades.o
else
  ifeq ($(CONFIG_CYCLADES),m)
  M_OBJS += cyclades.o
  endif
endif

ifeq ($(CONFIG_STALLION),y)
L_OBJS += stallion.o
else
  ifeq ($(CONFIG_STALLION),m)
  M_OBJS += stallion.o
  endif
endif

ifeq ($(CONFIG_ISTALLION),y)
L_OBJS += istallion.o
else
  ifeq ($(CONFIG_ISTALLION),m)
  M_OBJS += istallion.o
  endif
endif

ifeq ($(CONFIG_RISCOM8),y)
L_OBJS += riscom8.o
else
  ifeq ($(CONFIG_RISCOM8),m)
  M_OBJS += riscom8.o
  endif
endif

ifeq ($(CONFIG_SPECIALIX),y)
L_OBJS += specialix.o
else
  ifeq ($(CONFIG_SPECIALIX),m)
  M_OBJS += specialix.o
  endif
endif

ifeq ($(CONFIG_ISI),y)
L_OBJS += isicom.o
else
  ifeq ($(CONFIG_ISI),m)
  M_OBJS += isicom.o
  endif
endif


ifeq ($(CONFIG_ATIXL_BUSMOUSE),y)
M = y
L_OBJS += atixlmouse.o
else
  ifeq ($(CONFIG_ATIXL_BUSMOUSE),m)
  M_OBJS += atixlmouse.o
  MM = m
  endif
endif

ifeq ($(CONFIG_BUSMOUSE),y)
M = y
L_OBJS += busmouse.o
else
  ifeq ($(CONFIG_BUSMOUSE),m)
  M_OBJS += busmouse.o
  MM = m
  endif
endif

ifeq ($(CONFIG_PRINTER),y)
L_OBJS += lp.o
else
  ifeq ($(CONFIG_PRINTER),m)
  M_OBJS += lp.o
  endif
endif

ifeq ($(CONFIG_MS_BUSMOUSE),y)
M = y
L_OBJS += msbusmouse.o
else
  ifeq ($(CONFIG_MS_BUSMOUSE),m)
  M_OBJS += msbusmouse.o
  MM = m
  endif
endif

ifeq ($(CONFIG_PSMOUSE),y)
M = y
L_OBJS += psaux.o
else
  ifeq ($(CONFIG_PSMOUSE),m)
  M_OBJS += psaux.o
  MM = m
  endif
endif

ifeq ($(CONFIG_UMISC),y)
# To support third-party modules, misc.c must reside in the kernel
M = y
endif

ifeq ($(CONFIG_M68360_SIM_WDT),y)
M = y
endif

ifeq ($(CONFIG_DS1743),y)
M = y
L_OBJS += ds1743.o
endif

ifeq ($(CONFIG_SOFT_WATCHDOG),y)
M = y
L_OBJS += softdog.o
else
  ifeq ($(CONFIG_SOFT_WATCHDOG),m)
  M_OBJS += softdog.o
  MM = m
  endif   
endif

ifeq ($(CONFIG_PCWATCHDOG),y)
M = y
L_OBJS += pcwd.o
else
  ifeq ($(CONFIG_PCWATCHDOG),m)
  M_OBJS += pcwd.o
  MM = m
  endif
endif

ifdef CONFIG_SUN_MOUSE
M = y
endif

ifeq ($(CONFIG_WDT),y)
M = y
L_OBJS += wdt.o
else
  ifeq ($(CONFIG_WDT),m)
    M_OBJS += wdt.o
    MM = m
  endif
endif

ifeq ($(CONFIG_RTC),y)
M = y
L_OBJS += rtc.o
endif

ifdef CONFIG_QIC02_TAPE
L_OBJS += tpqic02.o 
endif

ifeq ($(CONFIG_FTAPE),y)
SUB_DIRS += ftape
L_OBJS += ftape/ftape.o
else
  ifeq ($(CONFIG_FTAPE),m)
  MOD_SUB_DIRS += ftape
  endif
endif

ifdef CONFIG_APM
M = y
endif

ifdef CONFIG_H8
LX_OBJS += h8.o
M = y
endif

ifdef M
LX_OBJS += misc.o
else
  ifdef MM
  MX_OBJS += misc.o
  endif
endif
  
ifeq ($(CONFIG_SCC),y)
L_OBJS += scc.o
else
  ifeq ($(CONFIG_SCC),m)
  M_OBJS += scc.o
  endif
endif  

ifeq ($(CONFIG_CONSOLE),y)
  ifeq ($(CONFIG_FRAMEBUFFER),y)
  LX_OBJS += fbmem.o
  endif
  L_OBJS += console.o vc_screen.o consolemap.o selection.o vt.o

  ifdef CONFIG_TGA_CONSOLE
    L_OBJS += tga.o
    ifdef CONFIG_VGA_CONSOLE
    L_OBJS += vga.o vesa_blank.o
    endif
  else
    ifndef CONFIG_SUN_CONSOLE
    L_OBJS += vga.o vesa_blank.o
    endif
  endif
endif

ifdef CONFIG_LCDDMA
L_OBJS += lcddma.o
endif

ifdef CONFIG_DAC0800
L_OBJS += dac0800.o
endif

ifeq ($(CONFIG_PWM),y)
L_OBJS += pwm328.o
endif

ifeq ($(CONFIG_AD1845),y)
L_OBJS += ad1845.o
else
  ifeq ($(CONFIG_AD1845),m)
  M_OBJS += ad1845.o
  endif
endif  

ifeq ($(CONFIG_M5249AUDIO),y)
L_OBJS += m5249audio.o
else
  ifeq ($(CONFIG_M5249AUDIO),m)
  M_OBJS += m5249audio.o
  endif
endif  

ifdef CONFIG_DACI2S
L_OBJS += daci2s.o
endif

ifdef CONFIG_LCDTXT
L_OBJS += lcdtxt.o
endif

ifdef CONFIG_KEYPAD
L_OBJS += keypad.o
endif

ifeq ($(CONFIG_KEY),y)
L_OBJS += key.o
else
  ifeq ($(CONFIG_KEY),m)
  M_OBJS += key.o
  endif
endif  

ifdef CONFIG_T6963_DMA 
L_OBJS += t6963fb.o
endif

ifdef CONFIG_T6963_PIO 
L_OBJS += t6963fb.o
endif

ifdef CONFIG_LEDMAN
L_OBJS += ledman.o
endif

ifdef CONFIG_LIRC_SERIAL
L_OBJS += lirc_serial.o
endif

ifdef CONFIG_LIRC_INTR
L_OBJS += lirc_intr.o
endif

ifeq ($(CONFIG_IDETEST),y)
L_OBJS += idetest.o
else
  ifeq ($(CONFIG_IDETEST),m)
  M_OBJS += idetest.o
  endif
endif  

ifdef CONFIG_HITACHI_SCI
L_OBJS += hitachi-sci.o
endif

ifeq ($(CONFIG_72001_SERIAL),y)
L_OBJS += pd72001_serial.o
endif


include $(TOPDIR)/Rules.make

fastdep: uni_hash.tbl

consolemap.o:

conmakehash: conmakehash.c
	$(HOSTCC) -o conmakehash conmakehash.c

uni_hash.tbl: $(FONTMAPFILE) conmakehash
	./conmakehash $(FONTMAPFILE) > uni_hash.tbl
