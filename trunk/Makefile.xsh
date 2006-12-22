# This Makefile runs a shell providing a cross-compilation environment.
# See http://www.dslinux.org/wiki/Porting_Howto

ROOTDIR=$(PWD)
PATH     := $(PATH):$(ROOTDIR)/tools
UCLINUX_BUILD_USER=1

ifneq ($(filter lib, $(MAKECMDGOALS)),)
UCLINUX_BUILD_USER=0
UCLINUX_BUILD_LIB=1
endif

include .config
include config.arch

.PHONY: xsh
xsh:
	@echo "Spawning cross-compilation shell. Happy hacking! :-)"
	@echo "------ Environment: ----------------------------------------------------"
	@echo "CC       =   $(CC)"
	@echo "CFLAGS   =   $(CFLAGS)"
	@echo "LD       =   $(LD)"
	@echo "LDFLAGS  =   $(LDFLAGS)"
	@echo "------------------------------------------------------------------------"
	@(echo "$(PATH)" | grep toolchain) 2>&1 >/dev/null || \
		echo "Remember to put the toolchain binaries in your PATH"
	@env PS1=" -xsh- \w > " /bin/sh
