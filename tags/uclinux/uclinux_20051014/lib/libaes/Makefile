.EXPORT_ALL_VARIABLES:

ifneq ($(CONFIG_KLIPS),)

OPENSWANSRCDIR = $(ROOTDIR)/openswan

EXTRA_CFLAGS += -I$(OPENSWANSRCDIR)/lib/libcrypto/libaes \
				-I$(OPENSWANSRCDIR)/linux/include \
				-I$(OPENSWANSRCDIR)/linux/include/aes

vpath %.c $(OPENSWANSRCDIR)/lib/libcrypto/libaes
vpath %.h $(OPENSWANSRCDIR)/lib/libcrypto/libaes

#
# Because libaes gets built twice (once with 'make -C openswan programs' 
# from user/openswan and and once here) we need to put the HW_ASSIST defines 
# for openswan's libaes in lib/libcrypto/libaes/Makefile rather than here
#

first_target: libaes.a

include $(OPENSWANSRCDIR)/lib/libcrypto/libaes/Makefile

romfs:

else
ifneq ($(CONFIG_IPSEC),)
EXTRA_CFLAGS += -I$(ROOTDIR)/freeswan/libcrypto/libaes
EXTRA_CFLAGS += -I$(ROOTDIR)/freeswan/libcrypto/include
EXTRA_CFLAGS += -DEMBED -DHIFN=1 -I$(ROOTDIR)/include -I$(ROOTDIR)/modules/crypto
LDLIBS += ../libc/libc.a

-include $(ROOTDIR)/modules/.config
ifneq ($(CONFIG_OCF_CRYPTODEV),)
EXTRA_CFLAGS += -DCRYPTODEV=1
endif

CONFIG_HW_ASSIST = y

VPATH = $(ROOTDIR)/freeswan/libcrypto/libaes

first_target: libaes.a

include $(ROOTDIR)/freeswan/libcrypto/libaes/Makefile

romfs:
endif
endif
