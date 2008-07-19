# Makefile for dict-words

# declare these targets as phony, so make will not try to look
# for files named like them:
.PHONY: all romfs clean

# 'all' is a phony target. It is called when the build process
# enters this directory and wants to build the program.
all:
	@echo "Dictionary Ready"

# The romfs target is also phony. It is used to copy your app
# into the DSLinux filesystem image.
romfs:
	$(ROMFSINST) -d -e CONFIG_USER_DICTIONARY_AMERICAN \
		/usr/share/dict/american-english
	$(ROMFSINST) -e CONFIG_USER_DICTIONARY_AMERICAN \
	    -l /usr/share/dict/american-english /usr/share/dict/words
	$(ROMFSINST) -d -e CONFIG_USER_DICTIONARY_BRITISH \
		/usr/share/dict/british-english
	$(ROMFSINST) -e CONFIG_USER_DICTIONARY_BRITISH \
	    -l /usr/share/dict/british-english /usr/share/dict/words


# clean is another phony target that removes every file generated
# during the build.
clean:
	rm -f words
