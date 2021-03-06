#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	ndstool
BUILD			:=	build
SOURCES		:=	source
INCLUDES	:=	include
DATA			:=	data

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
DEBUGFLAGS	:=


UNAME := $(shell uname -s)

CFLAGS	:=	$(DEBUGFLAGS) -Wall -O3
CFLAGS	+=	$(INCLUDE)


LDFLAGS	=	$(DEBUGFLAGS)

ifneq (,$(findstring Darwin,$(UNAME)))
        LDFLAGS         +=
endif



ifneq (,$(findstring MINGW,$(UNAME)))
	PLATFORM	:= win32
	EXEEXT		:= .exe
	CFLAGS		+= -mno-cygwin
	LDFLAGS		+= -mno-cygwin -Wl,--strip-debug
endif

ifneq (,$(findstring CYGWIN,$(UNAME)))
	CFLAGS		+= -mno-cygwin
	LDFLAGS		+= -mno-cygwin -Wl,--strip-debug
	EXEEXT		:= .exe
endif

ifneq (,$(findstring Linux,$(UNAME)))
	LDFLAGS 	+= -static -Wl,--strip-debug
endif

CXXFLAGS	=	$(CFLAGS) -fno-rtti -fno-exceptions

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:= -lelf

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS		:= $(PREFIX)/lib

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUTDIR:=	$(CURDIR)
export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
			$(CURDIR)/DefaultArm7 $(CURDIR)/Loader

export CC	:=	gcc
export CXX	:=	g++
export AR	:=	ar
export OBJCOPY	:=	objcopy

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	default_arm7.bin loadme.bin $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.bin)))
BMPFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.bmp)))

export OFILES	:= $(filter-out default_arm7.o, $(BINFILES:.bin=.o) $(BMPFILES:.bmp=.o)  $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o))
#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD) -I$(PREFIX)/include

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) -L$(PREFIX)/lib

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@echo PATH=$(PATH)
	[ -d $@ ] || mkdir -p $@
	make PassMeIncludes
	cc -o raw2c raw2c.c
	make -C Loader
	make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
.PHONY: PassMeIncludes
PassMeIncludes: $(BUILD)/passme_vhd1.h $(BUILD)/passme_vhd2.h

$(BUILD)/passme_vhd1.h: $(SOURCES)/passme.vhd
	cat $< | gawk '/^--!/ { COND=1; $$0=""; } // { gsub("\"", "\\\""); gsub("\011", "\\t"); gsub("\015", ""); if (!COND) print "\"" $$0 "\\n\"" }' > $@

$(BUILD)/passme_vhd2.h: $(SOURCES)/passme.vhd
	cat $< | gawk '/^--!/ { COND=1; $$0=""; } // { gsub("\"", "\\\""); gsub("\011", "\\t"); gsub("\015", ""); if (COND) print "\"" $$0 "\\n\"" }' > $@

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	make -C DefaultArm7 clean
	make -C Loader clean
	rm -fr $(BUILD) $(OUTPUT) raw2c
	rm -f ndstool raw2c

#---------------------------------------------------------------------------------
all: clean $(BUILD)

#---------------------------------------------------------------------------------
run: $(OUTPUT)
	@echo $(OUTPUT)

install:
	cp  $(OUTPUT)$(EXEEXT) $(PREFIX)

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT): $(OFILES)
	@echo linking
	$(LD) $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $(OUTPUT)$(EXEEXT)

#---------------------------------------------------------------------------------
# Compile Targets for C/C++
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
%.o : %.cpp
	@echo $(notdir $<)
	$(CXX) -MMD $(CXXFLAGS) -o $@ -c $<

#---------------------------------------------------------------------------------
%.o : %.c
	@echo $(notdir $<)
	$(CC) -MMD $(CFLAGS) -o $@ -c $<

#---------------------------------------------------------------------------------
%.o : %.s
	@echo $(notdir $<)
	$(CC) -MMD $(ASFLAGS) -o $@ -c $<


#---------------------------------------------------------------------------------
%.c	:	%.bmp
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(NDSTOOL_SRCDIR)/raw2c $<
 
#---------------------------------------------------------------------------------
%.c	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(NDSTOOL_SRCDIR)/raw2c $<

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
