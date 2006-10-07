# libs/pim/Makefile

platform-y=

platform-$(CONFIG_PLATFORM_X86DEMO)=platform/x86
platform-$(CONFIG_PLATFORM_IPAQ)=platform/ipaq
platform-$(CONFIG_PLATFORM_ZAURUS)=platform/zaurus
platform-$(CONFIG_PLATFORM_TUXSCREEN)=platform/tuxscreen
platform-$(CONFIG_PLATFORM_DSLINUX)=platform/dslinux

LIB_STATIC=libpixil.a
ifeq ($(EN_SHARED),--enable-shared)
LIB_SHARED=libpixil.so
else
LIB_SHARED=
endif

INCLUDES+=-I./include

SRC=${shell ls *.c} ${shell ls $(platform-y)/*.c}
OBJS=${SRC:.c=.o}

include $(BASE_DIR)/Rules.make



