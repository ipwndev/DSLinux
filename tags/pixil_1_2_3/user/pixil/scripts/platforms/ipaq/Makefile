# Set up the Ipaq platform

# The default PAR config template
PAR_TEMPLATE=defaults.xml.in
PAR_CONFIG=defaults.xml
LOCAL_DB=ipaq.db

# A set of directories that we want to build

SCRIPTS_DIR=$(INSTALL_DIR)/share/scripts
HTML_DIR=$(INSTALL_DIR)/share/html
MP3_DIR=$(INSTALL_DIR)/share/mp3
MPEG_DIR=$(INSTALL_DIR)/share/media
FONT_DIR=$(INSTALL_DIR)/share/fonts

INSTALL_DIRS=$(SCRIPTS_DIR)

HTML_FILES =  `find $(BASE_DIR)/data/html -path $(BASE_DIR)/data/html/CVS -prune -o -type f -print`
MP3_FILES   = `find $(BASE_DIR)/data/media -name *.mp3`
MPEG_FILES  = `find $(BASE_DIR)/data/media -name *.mpeg -o -name *.mpg`
FONT_FILES  = `find $(BASE_DIR)/data/fonts -name *.fnt`

ifeq ($(CONFIG_SAMPLES),y)
SAMPLE_TARGET=copy-samples
endif
    
include $(BASE_DIR)/Rules.make

# Empty targets

all:
clean:

install: $(INSTALL_DIRS) $(SAMPLE_TARGET) $(PAR_DB)
	@ cp scripts/pixil_suspend.sh $(SCRIPTS_DIR)
	@ cp nxstart.sh $(INSTALL_DIR)
	@ chmod uga+x $(INSTALL_DIR)/nxstart.sh

copy-samples: $(HTML_DIR) $(MP3_DIR) $(MPEG_DIR) $(FONT_DIR)
	@ cp $(HTML_FILES) $(HTML_DIR)
	@ cp $(MP3_FILES) $(MP3_DIR)
	@ cp $(MPEG_FILES) $(MPEG_DIR)
	@ cp $(FONT_FILES) $(FONT_DIR)

$(HTML_DIR) $(MP3_DIR) $(MPEG_DIR) $(FONT_DIR) $(INSTALL_DIRS):
	@ mkdir -p $@	

.PHONY: samples
