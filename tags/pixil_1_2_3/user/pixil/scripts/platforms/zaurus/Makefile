# Set up the Zaurus platform

# The default PAR config template
PAR_TEMPLATE=defaults.xml.in
PAR_CONFIG=defaults.xml
LOCAL_DB=zaurus.db

# A set of directories that we want to build

SCRIPTS_DIR=$(INSTALL_DIR)/share/scripts
HTML_DIR=$(INSTALL_DIR)/share/html
MP3_DIR=$(INSTALL_DIR)/share/mp3
MPEG_DIR=$(INSTALL_DIR)/share/media

INSTALL_DIRS=$(SCRIPTS_DIR) 

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

copy-samples: $(HTML_DIR) $(MP3_DIR) $(MPEG_DIR)
	@ cp $(BASE_DIR)/data/html/* $(HTML_DIR)

$(HTML_DIR) $(MP3_DIR) $(MPEG_DIR) $(INSTALL_DIRS):
	mkdir -p $@	

.PHONY: samples
