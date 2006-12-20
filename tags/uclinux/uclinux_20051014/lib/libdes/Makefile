#
# If using freeswan then we will build using its libdes, otherwise we
# prefer the openswan libdes.
#

.EXPORT_ALL_VARIABLES:


ifneq ($(CONFIG_IPSEC),)

EXTRA_CFLAGS += -I$(ROOTDIR)/freeswan/libdes
EXTRA_CFLAGS += -DEMBED
LDLIBS += ../libc/libc.a ../libm/libm.a

-include $(ROOTDIR)/modules/.config
ifneq ($(CONFIG_OCF_CRYPTODEV),)
EXTRA_CFLAGS += -DCRYPTODEV=1
endif



VPATH = $(ROOTDIR)/freeswan/libdes

first_target: libdes.a

include $(ROOTDIR)/freeswan/libdes/Makefile

romfs:

else

OPENSWANSRCDIR = $(ROOTDIR)/openswan

EXTRA_CFLAGS += -I$(OPENSWANSRCDIR)/lib/libcrypto/libdes \
				-I$(OPENSWANSRCDIR)/linux/include \
				-I$(OPENSWANSRCDIR)/linux/include/des

#
# Because libdes gets built twice (once with 'make -C openswan programs' 
# from user/openswan and and once here) we need to put the HW_ASSIST defines 
# for openswan's libdes in lib/libcrypto/libdes/Makefile rather than here
#

vpath %.c $(OPENSWANSRCDIR)/lib/libcrypto/libdes
vpath %.h $(OPENSWANSRCDIR)/lib/libcrypto/libdes

first_target: libdes.a

include $(OPENSWANSRCDIR)/lib/libcrypto/libdes/Makefile

romfs:

endif

the_one_true_dummy_target:

