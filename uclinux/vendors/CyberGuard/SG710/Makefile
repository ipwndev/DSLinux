#
#	Makefile -- Build instructions for CyberGuard SG710
#

IMAGE    = $(IMAGEDIR)/image.bin
ROMFSIMG = $(IMAGEDIR)/romfs.img
KERNELZ  = $(IMAGEDIR)/zImage
MBRIMG   = $(IMAGEDIR)/mbr.img

ROMFS_DIRS = \
	bin boot \
	dev dev/flash dev/pts \
	etc etc/config \
	home home/httpd home/httpd/cgi-bin \
	lib lib/modules \
	proc \
	sbin \
	usr usr/bin usr/sbin \
	var

DEVICES += $(DEVICE_PTY_64)

FLASH_DEVICES = \
	boot,c,90,0 \
	bootarg,c,90,0 \
	ethmac,c,90,0 \
	image,b,93,0 \
	config,b,93,2 \
	all,b,93,0

dep:

clean:
	-rm -f mkcramfs

romfs: romfs_dev
	[ -d $(ROMFSDIR)/. ] || mkdir -p $(ROMFSDIR)
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	$(ROMFSINST) ../romfs /
	$(ROMFSINST) ../network.init.std /etc/network.init
	$(ROMFSINST) -s /var/tmp /tmp
	$(ROMFSINST) -s /var/mnt /mnt
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) /etc/rc
	echo "desc_ethernet0 Switch A" >> $(ROMFSDIR)/etc/default/config
	echo "desc_ethernet1 Switch B" >> $(ROMFSDIR)/etc/default/config
	#
	echo "$(VERSIONSTR) -- " $(BUILD_START_STRING) > $(ROMFSDIR)/etc/version

mkcramfs: $(ROOTDIR)/user/cramfs/mkcramfs.c
	$(HOSTCC) -o $@ -I$(ROOTDIR)/$(LINUXDIR)/include $< -lz

image: mkcramfs
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)
	-cp $(ROOTDIR)/boot/ixp425/bios.bin $(ROMFSDIR)/boot/biosplus.bin
	-cp $(ROOTDIR)/boot/ixp425/boot.bin $(ROMFSDIR)/boot/bootplus.bin
	rm -rf $(ROMFSDIR)/man[1-9]
	rm -f $(ROMFSDIR)/lib/modules/*/kernel/ixp425/ixp400/ixp400_*.o
	cp $(ROOTDIR)/$(LINUXDIR)/arch/arm/boot/zImage $(KERNELZ)
	./mkcramfs -z -r $(ROMFSDIR) $(ROMFSIMG)
	# genromfs -v -V "ROMdisk" -f $(ROMFSIMG) -d $(ROMFSDIR)
	dd if=/dev/zero of=$(MBRIMG) bs=512 count=1
	echo -e "1,48575,L,*\n,,61,-" | /sbin/sfdisk  -uS -C 966 -H 16 -S 4 $(MBRIMG)
	cat $(MBRIMG) $(ROMFSIMG) $(KERNELZ) > $(IMAGE)
	echo -ne "\000" >> $(IMAGE)
	echo -n $(VERSIONPKG) >> $(IMAGE)
	echo -ne "\000" >> $(IMAGE)
	echo -n $(CONFIG_VENDOR) >> $(IMAGE)
	echo -ne "\000" >> $(IMAGE)
	echo -n $(CONFIG_PRODUCT) >> $(IMAGE)
	$(ROOTDIR)/tools/cksum -b -o 2 $(IMAGE) >> $(IMAGE)
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(IMAGE) /tftpboot

include $(ROOTDIR)/vendors/config/config.dev

