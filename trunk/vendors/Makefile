#
#	Makefile -- Vendor specific build instructions
#

.EXPORT_ALL_VARIABLES:
.PHONY: all romfs image clean

#
# Include architecture specific build rules.
#

ifndef ROOTDIR
ROOTDIR=..
endif

UCLINUX_BUILD_USER=1
-include $(LINUX_CONFIG)
-include $(CONFIG_CONFIG)
-include $(ARCH_CONFIG)
-include $(MODULES_CONFIG)

VEND=$(ROOTDIR)/vendors

dir_v = $(VEND)/$(CONFIG_VENDOR)/$(CONFIG_PRODUCT)/.

# Note: the all and clean targets are not normally used,
# since user/Makefile jumps directory to the product directory
all:
	+for i in $(dir_v) ; do \
		if [ -d $$i ]; then \
			touch $$i/.sgbuilt_vendor; \
			$(MAKE) -C $$i || exit $$? ; \
		fi; \
	done

romfs:
	+for i in $(dir_v) ; do \
		[ ! -d $$i ] || $(MAKE) -C $$i romfs || exit $$? ; \
	done

image:
	+for i in $(dir_v) ; do \
		[ ! -d $$i ] || $(MAKE) -C $$i image || exit $$? ; \
	done

vendor_%:
	+for i in $(dir_v) ; do \
		[ ! -d $$i ] || $(MAKE) -C $$i $@ || exit $$? ; \
	done

clean:
	-for i in $(dir_v) ; do \
		if [ -f $$i/.sgbuilt_vendor ]; then \
			$(MAKE) -C $$i clean ; \
			rm -f $$i/.sgbuilt_vendor; \
		fi; \
	done

