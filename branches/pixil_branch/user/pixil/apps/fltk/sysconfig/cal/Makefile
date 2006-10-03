# apps/sysconfig/about/Makefile

TARGET_SO=cal.so
INSTALL_SODIR=$(INSTALL_DIR)/share/sysconfig

SRC=${shell ls *.cxx} 
OBJS=${SRC:.cxx=.o}

OBJS += cal_ui.o

INCLUDES=-I../include

include $(BASE_DIR)/Rules.make

