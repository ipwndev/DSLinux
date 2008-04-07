# uClinux Makefile for the sample C++ application "rtest"
# =======================================================

# name of the executable
EXEC = 	rtest

# object files needed for executable
OBJS = 	rtest.o

# includes
INCS =

# libraries to link with (as needed)
# LDLIBS +=

# LD should output a map (to debug the link process)
# LDFLAGS += -Wl,-M

# declare these targets as phony, so make will not try to look
# for files named like them:
.PHONY: all romfs clean

# 'all' is a phony target. It is called when the build process
# enters this directory and wants to build the program.
all: $(EXEC)

# Translate a .cxx file to produce an object file.
# CXX: name of the c++ compiler, set by environment
# CXXFLAGS: needed flags to compile c++ programs, including uClibc++
CXXFLAGS +=-fno-rtti -fno-exceptions
%.o: %.cxx 
	$(CXX) -c $(CXXFLAGS) -o $@ $<

# This is the rule that builds(links) the executable.
# CXX: name of the c++ compiler, set by environment
# CRTBEGIN/CRTEND: enclosure for inclusion of static constructors.
#                  CRTBEGIN must be the first link object
#                  CRTEND must be the last link object
# LDFLAGS: common flags for the linker
# CXXLIBS: needed libraries for C++ programs, including uClibc++
# LDLIBS:  standard c libraries
$(EXEC): $(OBJS)
	$(CXX) -o $@ $(CRTBEGIN) $(LDFLAGS) $(OBJS) $(CXXLIBS) $(LDLIBS) $(CRTEND)
	# set the stack size (default = 4096 Bytes)
	arm-linux-elf-flthdr -s 8192 $@

# The romfs target is also phony. It is used to copy your app
# into the DSLinux filesystem image.
romfs:
	$(ROMFSINST) /bin/$(EXEC)

# clean is another phony target that removes every file generated
# during the build.
clean:
	rm -f $(EXEC) *.elf *.gdb *.o 
