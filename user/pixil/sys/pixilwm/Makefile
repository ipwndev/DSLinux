#nanox/pixilwm/Makefile

CFLAGS ?=
CFLAGS += -DVIRTUAL_WINDOWS 

INCLUDES += -I$(BASE_DIR)/sys/pixilwm

INSTALL_SODIR=$(INSTALL_DIR)/share/pixilwm
TARGET=pixilwm

ifeq ($(EN_SHARED),--enable-shared)
LIBTYPE :=.so
else
LIBTYPE :=.o
CFLAGS += -DSTATIC_LINK
endif

# These are extra targets that are defined locally

TARGET_EXTRAS += applets/date$(LIBTYPE)

ifeq ($(CONFIG_PIXILWM_PM),y)
TARGET_EXTRAS += applets/backlight$(LIBTYPE)
TARGET_EXTRAS += applets/battery$(LIBTYPE)
endif

ifeq ($(EN_SHARED),--enable-shared)
INSTALL_EXTRAS=inst_applets
endif

ifeq ($(CONFIG_PIXILWM_THEMES),y)
INSTALL_EXTRAS += inst_themes 
endif


OBJS-y =  applets.o main.o wlist.o config.o clients.o container.o 
OBJS-y += inputs.o apps.o home.o root.o categories.o exec.o 
OBJS-y += icons.o screensaver.o

OBJS-$(CONFIG_COLOSSEUM) += ipc.o
OBJS-$(CONFIG_PIXILWM_PM) += powerman.o
OBJS-$(CONFIG_PIXILWM_THEMES) += themes.o themesconfig.o
OBJS-$(CONFIG_PIXILWM_MENUS) += sys_menu.o

ifeq ($(LIBTYPE),.o)
OBJS-y += applets/date$(LIBTYPE)
OBJS-$(CONFIG_PIXILWM_PM) += applets/backlight$(LIBTYPE)
OBJS-$(CONFIG_PIXILWM_PM) += applets/battery$(LIBTYPE)
endif

OBJS := $(OBJS-y)

LIBS+= -lwm -lpixil -lnano-X  

ifeq ($(CONFIG_PIXILWM_THEMES),y)
LIBS += -lxml
endif

ifeq ($(CONFIG_COLOSSEUM),y)
LIBS+=-lipc
endif

ifeq ($(CONFIG_PAR), y)
LIBS+=-lpar
endif

ifeq ($(EN_SHARED),--enable-shared)
LIBS+=-ldl
BUILD_CFLAGS=-rdynamic
endif

CLEAN_EXTRAS=clean_themes

include $(BASE_DIR)/Rules.make

clean_themes:
	rm -rf applets/*.o
	rm -rf applets/*.so

inst_themes:
	mkdir -p $(INSTALL_DIR)/share/themes
	-cp themes/pixil/* $(INSTALL_DIR)/share/themes

inst_applets:
	mkdir -p $(INSTALL_DIR)/share/pixilwm
	cp applets/*.so $(INSTALL_DIR)/share/pixilwm

applets/backlight.so: applets/backlight.o
	$(CC) -shared -o $@ $<

applets/battery.so: applets/battery.o
	$(CC) -shared -o $@ $< 

applets/date.so: applets/date.o
	$(CC) -shared -o $@ $<
