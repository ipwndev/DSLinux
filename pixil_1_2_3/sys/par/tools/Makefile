#par/tools/Makefile

# These targets are all defined in this Makefile rather than in 
# Rules.make

TARGET_EXTRAS  = platform-objs/xmlimport platform-objs/xmlexport 
TARGET_EXTRAS += platform-objs/parget platform-objs/parset

TARGET_EXTRAS += native-objs/xmlimport native-objs/xmlexport 
TARGET_EXTRAS += native-objs/parget native-objs/parset

# This is called before the build to make sure that all of 
# our directories are in place

PREBUILD_EXTRAS = $(CURDIR)/native-objs $(CURDIR)/platform-objs

# This is called after the build to install the native tools
POSTBUILD_EXTRAS = par-native-install

# This specifies the target for installing the platform tools

INSTALL_EXTRAS = par-platform-install

NATIVE_INSTALL_TARGETS = $(STAGE_DIR)/bin/native/xmlexport $(STAGE_DIR)/bin/native/xmlimport
NATIVE_INSTALL_TARGETS += $(STAGE_DIR)/bin/native/parget $(STAGE_DIR)/bin/native/parset

# The directory to install the platform tools in
INSTALL_BINDIR=$(INSTALL_DIR)/sbin

LIBS= -L$(STAGE_DIR)/lib -lxml -lpar
NATIVE_LIBS = -L$(STAGE_DIR)/lib/native/ -lxml -lpar

CLEAN_EXTRAS=par-clean

INCLUDES=-I../lib

include $(BASE_DIR)/Rules.make

par-platform-install: $(INSTALL_BINDIR)
	cp platform-objs/xmlimport platform-objs/xmlexport \
	platform-objs/parget platform-objs/parset $(INSTALL_BINDIR)

par-native-install: $(NATIVE_INSTALL_TARGETS)

par-clean:
	rm -rf platform-objs native-objs

platform-objs/xmlimport: platform-objs/xmlimport.o platform-objs/parxml.o
	$(CC) -o $@ platform-objs/xmlimport.o platform-objs/parxml.o $(LIBS)

platform-objs/xmlexport: platform-objs/xmlexport.o platform-objs/parxml.o
	$(CC) -o $@ platform-objs/xmlexport.o platform-objs/parxml.o $(LIBS)

platform-objs/parget: platform-objs/parget.o
	$(CC) -o $@ $< $(LIBS)

platform-objs/parset: platform-objs/parset.o
	$(CC) -o $@ $< $(LIBS)

## Native targets

native-objs/xmlimport: native-objs/xmlimport.o native-objs/parxml.o
	gcc -o $@ native-objs/xmlimport.o native-objs/parxml.o $(NATIVE_LIBS)

native-objs/xmlexport: native-objs/xmlexport.o native-objs/parxml.o
	gcc -o $@  native-objs/xmlexport.o native-objs/parxml.o $(NATIVE_LIBS)

native-objs/parget: native-objs/parget.o
	gcc -o $@ $< $(NATIVE_LIBS)

native-objs/parset: native-objs/parset.o
	gcc -o $@ $< $(NATIVE_LIBS)

$(STAGE_DIR)/bin/native/xmlexport: $(STAGE_DIR)/bin/native native-objs/xmlexport
	if [ ! -h $(STAGE_DIR)/bin/native/xmlexport ]; then \
		ln -s $(CURDIR)/native-objs/xmlexport $@; \
	fi

$(STAGE_DIR)/bin/native/xmlimport: $(STAGE_DIR)/bin/native native-objs/xmlimport
	if [ ! -h $(STAGE_DIR)/bin/native/xmlimport ]; then \
		ln -s $(CURDIR)/native-objs/xmlimport $@; \
	fi

$(STAGE_DIR)/bin/native/parget: $(STAGE_DIR)/bin/native native-objs/parget
	if [ ! -h $(STAGE_DIR)/bin/native/parget ]; then \
		ln -s $(CURDIR)/native-objs/parget $@; \
	fi

$(STAGE_DIR)/bin/native/parset: $(STAGE_DIR)/bin/native native-objs/parset
	if [ ! -h $(STAGE_DIR)/bin/native/parset ]; then \
		ln -s $(CURDIR)/native-objs/parset $@; \
	fi


$(CURDIR)/platform-objs $(CURDIR)/native-objs $(STAGE_DIR)/bin/native:
	@ mkdir -p $@
