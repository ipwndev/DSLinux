#
#	Makefile -- Build instructions for CyberGuard SG550
#

CONFIG_SH_SCI_REVERSE=y

ROMFSIMG = $(IMAGEDIR)/initrd
IMAGE    = $(IMAGEDIR)/image.bin
FLASH    = $(IMAGEDIR)/flash.bin
IMAGEZ   = $(IMAGEDIR)/imagez.bin
IMAGES   = $(IMAGEDIR)/sh.mot
IMAGEE   = $(IMAGEDIR)/sh.abs

DIRS =

ROMFS_DIRS = bin sbin dev/flash etc etc/config home lib proc usr var \
             home/httpd home/httpd/cgi-bin

DEVICES += $(DEVICE_PTY_64)

FLASH_DEVICES = \
	boot,c,90,0 \
	ethmac,c,90,0 \
	bootarg,c,90,0 \
	config,c,90,2 \
	image,c,90,4 \
	all,c,90,6

all:

clean:
	rm -f mkcramfs

romfs: romfs_dev
	[ -d $(ROMFSDIR) ] || mkdir -p $(ROMFSDIR)
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	$(ROMFSINST) -s /var/tmp /tmp
	$(ROMFSINST) -s /var/mnt /mnt
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) ../romfs /
	$(ROMFSINST) ../network.init.std /etc/network.init
	#
	$(ROMFSINST) /etc/rc
	echo "$(VERSIONSTR) -- " $(BUILD_START_STRING) > $(ROMFSDIR)/etc/version

mkcramfs: $(ROOTDIR)/user/cramfs/mkcramfs.c
	$(HOSTCC) -o $@ -I$(ROOTDIR)/$(LINUXDIR)/include $< -lz

image: mkcramfs
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)
	rm -rf $(ROMFSDIR)/man[1-9]
	$(CROSS)objcopy -O binary $(ROOTDIR)/$(LINUXDIR)/vmlinux \
			$(IMAGEDIR)/linux.bin
	gzip -c -9 < $(IMAGEDIR)/linux.bin > $(IMAGEDIR)/linuz.bin
	# genromfs -v -V "ROMdisk" -f $(ROMFSIMG) -d $(ROMFSDIR)
	./mkcramfs -z -r $(ROMFSDIR) $(ROMFSIMG)
	cat $(ROMFSIMG) $(IMAGEDIR)/linuz.bin > $(IMAGE)
	# cat $(IMAGEDIR)/linux.bin $(ROMFSIMG) | gzip -c -9 > $(IMAGE)
	echo -ne "\000" >> $(IMAGE)
	echo -n $(VERSIONPKG) >> $(IMAGE)
	echo -ne "\000" >> $(IMAGE)
	echo -n $(HW_VENDOR) >> $(IMAGE)
	echo -ne "\000" >> $(IMAGE)
	echo -n $(HW_PRODUCT) >> $(IMAGE)
	$(ROOTDIR)/tools/cksum -b -o 2 $(IMAGE) >> $(IMAGE)
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(IMAGE) /tftpboot
	ADDR=`$(CROSS)nm $(ROOTDIR)/$(LINUXDIR)/vmlinux | \
		grep " __bss_start$$" | \
		cut -d' ' -f1 | xargs printf "0x%s\n"`;\
		echo "ROMFS@$$ADDR" ; \
        $(CROSS)objcopy --add-section=.romfs=$(ROMFSIMG) \
          --adjust-section-vma=.romfs=$${ADDR} --no-adjust-warnings \
          --set-section-flags=.romfs=alloc,load,data   \
		  -O srec \
          $(ROOTDIR)/$(LINUXDIR)/vmlinux $(IMAGES);  \
        : $(CROSS)objcopy --add-section=.romfs=$(ROMFSIMG) \
          --adjust-section-vma=.romfs=$${ADDR} --no-adjust-warnings \
          --set-section-flags=.romfs=alloc,load,data   \
          $(ROOTDIR)/$(LINUXDIR)/vmlinux $(IMAGEE)
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(IMAGES) /tftpboot/.
	# [ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(IMAGEE) /tftpboot/.
	[ ! -f $(ROOTDIR)/boot/boot.bin ] || $(MAKE) vendor_flashbin

#
# build up a flash.bin
#
vendor_flashbin:
	@dd conv=sync if=$(ROOTDIR)/boot/boot.bin of=$(FLASH) bs=48k
	@( \
		MAC0="\000\320\317\000\000\001"; \
		MAC1="\000\320\317\000\000\002"; \
		MAC2="\000\320\317\000\000\003"; \
		MAC3="\000\320\317\000\000\004"; \
		PAD0="\000\000\000\000\000\000"; \
		MAG0="\253\272"; \
		echo -en "$$MAC0$$MAC1$$MAC2$$MAC3$$PAD0$$MAG0"; \
		echo -en "mem=16M console=ttySC1,57600\000"; \
	) | dd conv=sync bs=80k >> $(FLASH)
	@echo "Bad config filesystem" | dd conv=sync bs=512k >> $(FLASH)
	@cat $(IMAGE) >> $(FLASH)
	@echo "Factory `basename $(FLASH)` created."
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(FLASH) /tftpboot/.

include $(ROOTDIR)/vendors/config/config.dev

