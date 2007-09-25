# This Makefile runs a shell providing a cross-compilation environment.
# See http://www.dslinux.org/wiki/Porting_Howto

ROOTDIR=$(PWD)
PATH     := $(PATH):$(ROOTDIR)/tools
UCLINUX_BUILD_USER=1
CONFIG_LIB_UCLIBCPP=1

ifneq ($(filter lib, $(MAKECMDGOALS)),)
UCLINUX_BUILD_USER=0
UCLINUX_BUILD_LIB=1
endif

include .config
include config.arch

.PHONY: xsh
xsh:
	@echo "Spawning cross-compilation shell. Happy hacking! :-)"
	@echo "Type exit to get out of here"
	@echo "YOU CANNOT COMPILE ALL OF DSLinux FROM THIS SHELL!"
	@echo "It is suitable only for porting applications and libraries."
	@echo "------ Environment: ----------------------------------------------------"
	@echo "CC       =   $(CC)"
	@echo "CFLAGS   =   $(CFLAGS)"
	@echo "LD       =   $(LD)"
	@echo "LDFLAGS  =   $(LDFLAGS)"
	@echo "------------------------------------------------------------------------"
	@(echo "$(PATH)" | grep toolchain) 2>&1 >/dev/null || \
		echo "Remember to put the toolchain binaries in your PATH!"
	@env PS1=" -xsh- \w > " /bin/sh
