# Makefile for libmad in uClinux
# ==============================

# Special compiler options
CFLAGS += -DHAVE_CONFIG_H -DASO_INTERLEAVE1 -DASO_IMDCT -DFPM_ARM
CFLAGS += -Wall -O -fforce-mem -fforce-addr -fthread-jumps
CFLAGS += -fcse-follow-jumps -fcse-skip-blocks -fexpensive-optimizations
CFLAGS += -fregmove -fschedule-insns2 -fstrength-reduce -fomit-frame-pointer

# Header files
INCS:=  config.h version.h fixed.h bit.h timer.h stream.h frame.h \
	synth.h decoder.h global.h layer12.h layer3.h huffman.h \
	D.dat imdct_s.dat qc_table.dat rq_table.dat sf_table.dat

# Object files
OBJS:=  version.o fixed.o bit.o timer.o stream.o frame.o synth.o \
	decoder.o layer12.o layer3.o huffman.o imdct_l_arm.o

.PHONY: all
all: libmad.a

libmad.a: $(OBJS) $(INCS)
	$(AR) rv $@ $(OBJS)
	$(RANLIB) $@

.PHONY: romfs
romfs:

.PHONY: clean
clean:
	-rm -f *.[oa] *~

