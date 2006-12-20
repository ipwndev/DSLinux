# Makefile to set up the X86 DEMO platform

# The default run_pixil.sh script template
RUN_TEMPLATE=run_pixil.sh.in

# The default PAR config template

PAR_TEMPLATE=defaults.xml.in
PAR_CONFIG=defaults.xml
LOCAL_DB=x86-demo.db

# A set of directories that we want to build

SYSLOG_DIR=$(INSTALL_DIR)/var/log
NETSCRIPT_DIR=$(INSTALL_DIR)/share/scripts
HTML_DIR=$(INSTALL_DIR)/share/html
MP3_DIR=$(INSTALL_DIR)/share/mp3
MPEG_DIR=$(INSTALL_DIR)/share/media
FONT_DIR=$(INSTALL_DIR)/share/fonts

INSTALL_DIRS=$(SYSLOG_DIR) $(NETSCRIPT_DIR) 
ifeq ($(CONFIG_SAMPLES),y)
SAMPLE_TARGET=copy-samples
endif

include $(BASE_DIR)/Rules.make

SCRIPT_DIR:=$(DEST_DIR)
DEST_DIR:=$(SCRIPT_DIR)/pda

# Actually get the files that we want 

HTML_FILES =  `find $(BASE_DIR)/data/html -path $(BASE_DIR)/data/html/CVS -prune -o -type f -print`
MP3_FILES   = `find $(BASE_DIR)/data/media -name *.mp3`
MPEG_FILES  = `find $(BASE_DIR)/data/media -name *.mpeg -o -name *.mpg`
FONT_FILES  = `find $(BASE_DIR)/data/fonts -name *.fnt`

ifeq ($(CONFIG_X11),y)
SCRIPT_TARGET=
else
SCRIPT_TARGET=build-script
endif

# Empty targets

all:
clean:

install: $(INSTALL_DIRS) $(SAMPLE_TARGET) $(SCRIPT_TARGET) $(PAR_DB)

copy-samples: $(HTML_DIR) $(MP3_DIR) $(MPEG_DIR) $(FONT_DIR)
	@ cp $(HTML_FILES) $(HTML_DIR)
	@ cp $(MP3_FILES) $(MP3_DIR)
	@ cp $(MPEG_FILES) $(MPEG_DIR)
	@ cp $(FONT_FILES) $(FONT_DIR)

build-script: $(RUN_TEMPLATE)
	cat $(RUN_TEMPLATE) | \
	sed -e s%@prefix@%$(PREFIX)%g | \
	sed -e s%@installdir@%$(DEST_DIR)/%g | \
	sed -e s%@mwdir@%$(MWDIR)%g | \
	sed -e s%@fltkdir@%$(FLTKDIR)%g > $(SCRIPT_DIR)/run_pda.sh 

	@ chmod u+x $(SCRIPT_DIR)/run_pda.sh

$(HTML_DIR) $(MP3_DIR) $(MPEG_DIR) $(FONT_DIR) $(INSTALL_DIRS):
	@ mkdir -p $@	

.PHONY: samples
