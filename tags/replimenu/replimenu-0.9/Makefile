#
# Makefile for 'replimenu'.
# Copyright (C) 2003 Michel Blomgren, michel AT zebra DOT ath DOT cx.
#
# Please look at the "configuration"-file before compiling!
#

include configuration

CWD = $(shell pwd)


all:	clean build

install:
	@rm -f $(BINDIR)/$(EXECNAME)
	@rm -f $(MANDIR)/man1/replimenu.1.gz

	@if [ -e src/$(EXECNAME) ]; then \
		echo -e "\033[1mInstalling $(BINDIR)/$(EXECNAME)...\033[0m" ; \
		$(INSTALL) -m 755 src/$(EXECNAME) $(BINDIR)/$(EXECNAME); \
	fi

	@if [ -e man/replimenu.1 ]; then \
		echo -e "\033[1mInstalling $(MANDIR)/man1/replimenu.1.gz...\033[0m" ; \
		$(INSTALL) -m 644 man/replimenu.1 $(MANDIR)/man1/replimenu.1; \
		gzip $(MANDIR)/man1/replimenu.1; \
	fi

	@echo; \
	echo -e "\033[1mDone.\033[0m" ; \
	echo

build:
	@for i in $(DIRS); do \
		echo ; \
		echo -e "\033[1mCompiling sources in \"$$i\"...\033[0m" ; \
		echo ; \
		cd $(CWD)/$$i ; \
		$(MAKE) build ; \
	done

	@echo ; \
	echo -e "\033[1mDone.\033[0m" ; \
	echo ; \
	echo "You may have seen warning messages similar to..." ; \
	echo "\"warning: comparison is always false due to limited range of data type\"" ; \
	echo "Just ignore them, they are not true." ; \
	echo

clean:
	@for i in $(DIRS); do \
		echo ; \
		echo -e "\033[1mCleaning \"$$i\"...\033[0m" ; \
		echo ; \
		cd $(CWD)/$$i ; \
		$(MAKE) clean ; \
	done

uninstall:
	@echo -e "\033[1mUninstalling $(BINDIR)/$(EXECNAME)...\033[0m"
	@rm -f $(BINDIR)/$(EXECNAME)
	@echo -e "\033[1mUninstalling $(MANDIR)/man1/replimenu.1.gz...\033[0m"
	@rm -f $(MANDIR)/man1/replimenu.1.gz
	@echo; \
	echo -e "\033[1mDone.\033[0m" ; \
	echo

war:
	@echo; \
	echo "The war is over. Simply type \"make\" to compile and \"make install\" to install." ; \
	echo

morewar:
	@echo; \
	echo "The war is over. Simply type \"make\" to compile and \"make install\" to install." ; \
	echo

peace:
	@echo; \
	echo "The war is over. Type \"make uninstall\" to uninstall replimenu." ; \
	echo

distclean: clean

dist:
	@echo ; \
	echo -e "\033[1mMaking README file...\033[0m" ; \
	echo
	./mkreadme

	@echo ; \
	echo -e "\033[1mMaking tarball...\033[0m" ; \
	echo
	mkdir -p replimenu-$(VERSION)/{man,src}
	tar -cf temporaryplace-$(VERSION).tar $(DISTFILES)
	tar -C replimenu-$(VERSION) -xf temporaryplace-$(VERSION).tar
	tar -czf replimenu-$(VERSION).tar.gz replimenu-$(VERSION)
	rm -rf replimenu-$(VERSION)
	rm -rf temporaryplace-$(VERSION).tar

	@echo ; \
	echo -e "\033[1mDone.\033[0m" ; \
	echo
