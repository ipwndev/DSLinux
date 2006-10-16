# uClinux Makefile for rtest
# ==========================

# name of the executable
EXEC = 	rtest

# object files needed for executable
OBJS = 	rtest.o

# includes
INCS =

# libraries to link with
# LDLIBS +=

# LDFLAGS += -Wl,-M

# declare these targets as phony, so make will not try to look
# for files named like them:
.PHONY: all romfs clean

# 'all' is a phony target. It is called when the build process
# enters this directory and wants to build the program.
all: $(EXEC)

%.o: %.cxx 
	$(CXX) -c $(CXXFLAGS) -o $@ $<

# This is the rule that build the executable.
$(EXEC): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)
	arm-linux-elf-flthdr -k $(EXEC) 

# The romfs target is also phony. It is used to copy your app
# into the DSLinux filesystem image.
romfs:
	$(ROMFSINST) /bin/$(EXEC)

# clean is another phony target that removes every file generated
# during the build.
clean:
	rm -f $(EXEC) *.elf *.gdb *.o 
