#
# Makefile for the linux kernel.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).

HEAD_OBJ  = head-arm-$(MACHINE).o
ENTRY_OBJ = entry-$(PROCESSOR).o

O_TARGET := kernel.o
O_OBJS   := $(ENTRY_OBJ) ksyms.o ioport.o irq.o process.o ptrace.o signal.o sys_arm.o time.o traps.o
#D_OBJS   +=  ioport.o irq.o process.o ptrace.o signal.o sys_arm.o time.o traps.o

all: kernel.o $(HEAD_OBJ)

ifeq ($(MACHINE),trio)
  O_OBJS += setup.o
  D_OBJS = setup.o
endif
ifeq ($(MACHINE),atmel)
  O_OBJS += setup.o
  D_OBJS = setup.o
endif
ifeq ($(MACHINE),gba)
  O_OBJS += setup.o
  D_OBJS = setup.o
endif
ifeq ($(MACHINE),netarm)
  O_OBJS += setup.o netarm_console.o
  D_OBJS = setup.o
endif


$(HEAD_OBJ): $(HEAD_OBJ:.o=.S)
	$(CC) -D__ASSEMBLY__ -g -c $(HEAD_OBJ:.o=.S) -o $@

$(ENTRY_OBJ): $(ENTRY_OBJ:.o=.S)
	$(CC) $(CFLAGS) -g -D__ASSEMBLY__ -c $(ENTRY_OBJ:.o=.S) -o $@

$(ENTRY_OBJ): ../lib/constants.h

.PHONY: ../lib/constants.h

../lib/constants.h:
	$(MAKE) -C ../lib constants.h

include $(TOPDIR)/Rules.make

