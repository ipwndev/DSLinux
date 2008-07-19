#nanox/misc/Makefile

TARGET_EXTRAS=nxloadmon nxterm
INSTALL_EXTRAS=install-apps
CLEAN_EXTRAS=clean-apps

# Just the basic nano-X library for nxterm

NXLIBS=-L$(MWDIR)/lib -lnano-X
#LDFLAGS=-Wl,-elf2flt -D__PIC__ -fpic -msingle-pic-base -mswp-byte-writes
# A bit more is needed for nxclock and nxloadmon

WMLIBS= -L$(STAGE_DIR)/lib -L$(MWDIR)/lib -lwm -lnano-X

ifeq ($(CONFIG_COLOSSEUM),y)
WMLIBS+=-lipc
endif

include $(BASE_DIR)/Rules.make

nxloadmon: nxloadmon.o
	$(CC) $(LDFLAGS) $(CFLAGS) $(INCLUDES) nxloadmon.o -o $@ $(WMLIBS)

nxterm: nxterm.o
	$(CC) $(LDFLAGS) $(CFLAGS) $(INCLUDES) nxterm.o -o $@ $(NXLIBS)

install-apps: $(INSTALL_BINDIR)
	@ cp nxloadmon nxterm $(INSTALL_BINDIR)

clean-apps:
	@ rm -f nxloadmon nxterm

