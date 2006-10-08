# sys/syncagent/tcpip/Makefile

ifeq ($(EN_SHARED),--enable-shared)
TARGET_SO=tcpip.so
INSTALL_SODIR=$(INSTALL_DIR)/share/sync
endif

SRC=${shell ls *.c} 
OBJS=${SRC:.c=.o}

INCLUDES=-I../include

include $(BASE_DIR)/Rules.make

