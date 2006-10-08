#nanox/pixilwm/Makefile

SRC=${shell ls *.c} 
OBJS=${SRC:.c=.o}

TARGET=syncagent

ifeq ($(CONFIG_PAR),y)
LIBS+=-lpar
endif

ifeq ($(CONFIG_COLOSSEUM),y)
LIBS+=-lipc
endif

ifeq ($(EN_SHARED),--enable-shared)
LIBS+=-ldl
endif

include $(BASE_DIR)/Rules.make
