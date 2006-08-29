# Makefile for flnx in uClinux
# ============================

# Special compiler options
CXXFLAGS += -DPDA -DNANO_X -I./FL -I./src -I$(ROOTDIR)/include/microwin
CXXFLAGS += -D'NANOXFOLDER="/usr/bin"'

# Header files
INCS   := $(wildcard FL/*)

# Object files
CXX_OBJS := $(patsubst %.cxx,%.o,$(wildcard src/*.cxx))
C_OBJS   := $(patsubst %.c,%.o,$(wildcard src/*.c))

.PHONY: all
all: libfltk.a

$(CXX_OBJS): %.o: %.cxx $(INCS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(C_OBJS): %.o: %.c $(INCS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

libfltk.a: $(C_OBJS) $(CXX_OBJS)
	$(AR) rv $@ $(C_OBJS) $(CXX_OBJS)
	$(RANLIB) $@

.PHONY: romfs
romfs:

.PHONY: clean
clean:
	-rm -f *.[oa] *~
