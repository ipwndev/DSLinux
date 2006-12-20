#############################################################################
#
#	Makefile -- Build instructions for CyberGuard/SG810
#
#############################################################################

include $(LINUX_CONFIG)
include $(CONFIG_CONFIG)
include $(ARCH_CONFIG)

#############################################################################

LINUXFILE = bzImage
LINUXIMG  = $(ROOTDIR)/$(LINUXDIR)/arch/i386/boot/$(LINUXFILE)

VENDDIR    = $(ROOTDIR)/vendors/$(CONFIG_VENDOR)/$(CONFIG_PRODUCT)/.

PREFIX     = $(IMAGEDIR)/SG810-CF

# the number of 512 byte blocks in config partition (4096 = 2Mb)
CONFIG     = 4096

KERNELZ    = $(LINUXFILE)
RAMDISKZ   = cramfs.img.gz
ISOFILE  = $(IMAGEDIR)/SG810.iso
ISODIR     = $(ROOTDIR)/iso

#############################################################################

DIRS = 

ROMFS_DIRS = \
	bin boot \
	dev dev/flash dev/pts \
	etc etc/config \
	home home/httpd home/httpd/cgi-bin \
	lib lib/modules \
	proc \
	tmp \
	sbin \
	usr usr/bin usr/sbin \
	var var/run var/log var/tmp var/lock var/empty

DEVICES += $(DEVICE_PTY_16)

FLASH_DEVICES = \
	image,b,22,0 \
	config,b,22,2

#############################################################################

all:
	dirs=$(DIRS) ; \
	for i in $$dirs ; do  make -C $$i || exit $? ; done

clean:
	-dirs=$(DIRS) ; \
	for i in $$dirs; do [ ! -d $$i ] || make -C $$i clean; done
	rm -f romfs.img ext3.img cramfs.img mkcramfs

romfs: romfs_dev
	[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	$(ROMFSINST) $(LINUXIMG) /boot/vmlinuz
	$(ROMFSINST) ../romfs /
	$(ROMFSINST) ../network.init.std /etc/network.init
	$(ROMFSINST) -s /var/tmp /tmp
	$(ROMFSINST) -s /var/mnt /mnt
	$(ROMFSINST) -s /var/tmp/log /dev/log
	$(ROMFSINST) /etc/rc
	#
	echo "$(VERSIONSTR) -- " $(BUILD_START_STRING) > $(ROMFSDIR)/etc/version

#############################################################################
#
# tool we need for build host based versions of the programs
#

mkcramfs: $(ROOTDIR)/user/cramfs/mkcramfs.c
	$(HOSTCC) -o $@ -I$(ROOTDIR)/$(LINUXDIR)/include $< -lz

LILO_CFLAGS = -I$(ROOTDIR)/user/lilo `cat $(ROOTDIR)/user/lilo/mylilo.h` -DLCF_IGNORECASE -DHAS_BOOT_H -DLCF_M386 -DLCF_READONLY -DLCF_VARSETUP

LILOSOURCE = lilo.c map.c geometry.c boot.c device.c common.c bsect.c cfg.c \
	temp.c partition.c identify.c probe.c shs2.c

.PHONY: lilo
lilo:
	chmod +x lilo
	#gcc $(LILO_CFLAGS) -o $@ $(patsubst %.c,$(ROOTDIR)/user/lilo/%.c,$(LILOSOURCE))

#############################################################################
#
# re copy the kernel so that "make linux image" works
#

image: cf-images
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(PREFIX)-recover.bin /tftpboot/.
	gzip -9 -f -c cramfs.img > $(RAMDISKZ) 
	mkdir -p $(ISODIR)/boot/isolinux
	cp isolinux.bin $(ISODIR)/boot/isolinux/
	cp boot.msg $(ISODIR)/boot/isolinux/
	cp isolinux.cfg $(ISODIR)/boot/isolinux/
	cp $(LINUXIMG) $(ISODIR)/boot/isolinux/
	cp $(RAMDISKZ) $(ISODIR)/boot/isolinux/initrd.gz
	mkisofs -J -o $(ISOFILE) -b boot/isolinux/isolinux.bin \
		-c boot/isolinux/boot.cat -no-emul-boot \
		-boot-load-size 4 -boot-info-table $(ISODIR)

#############################################################################
#
# make images for various sizes of compact flash,  check your geometry,
# you may need to add your own here,  remove any your don't want built from
# the cf-images target
#

.PHONY: cf-images extimage-64MB extimage-128MB

romfs.img: mkcramfs
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)
	rm -rf $(ROMFSDIR)/man[1-9]
	./mkcramfs -z -r $(ROMFSDIR) cramfs.img
	mknbi-linux --append=init=/bin/sh --rootmode=ro --rootdir=/dev/0100 \
			--output=$(PREFIX)-recover.bin $(LINUXIMG) cramfs.img
	genromfs -v -V "ROMdisk" -f romfs.img -d $(ROMFSDIR)

cf-images: extimage-64MB extimage-128MB

extimage-64MB: romfs.img
	#$(MAKE) extimage HEADS=8 SECTORS=32 CYLS=480 IMAGE=$(PREFIX)-64MB.sgu
	$(MAKE) extimage HEADS=4 SECTORS=32 CYLS=978 IMAGE=$(PREFIX)-64MB.sgu

extimage-128MB: romfs.img
	$(MAKE) extimage HEADS=8 SECTORS=32 CYLS=960 IMAGE=$(PREFIX)-128MB.sgu

#############################################################################
#
# Make a lilo bootable compact flash image based on paramters passed in
#

.PHONY: extimage
extimage: lilo
	@echo '********************************************************' > /dev/tty
	@echo '********************************************************' > /dev/tty
	@echo '**                                                    **' > /dev/tty
	@echo '**                                                    **' > /dev/tty
	@echo '** Time to make the final compact flash images.       **' > /dev/tty
	@echo '** For this you need to be "root". You will also need **' > /dev/tty
	@echo '** a kernel with the loopback device enabled and ext2 **' > /dev/tty
	@echo '** filesystem support.  Enter the root password below **' > /dev/tty
	@echo '** if asked,  or an invalid password to abort.        **' > /dev/tty
	@echo '**                                                    **' > /dev/tty
	@echo '**                                                    **' > /dev/tty
	@echo '********************************************************' > /dev/tty
	@echo '********************************************************' > /dev/tty
	@echo > /dev/tty
	rm -f mbr.img ext3.img
	sudo sh make-ext3-image $(CYLS) $(HEADS) $(SECTORS) $(CONFIG)
	cat mbr.img ext3.img | gzip -9 > $(IMAGE)
	cp $(IMAGE) $(basename $(IMAGE)).bin.gz
ifdef CONFIG_USER_NETFLASH_HMACMD5
	$(ROOTDIR)/user/netflash/hmacmd5 $(CONFIG_USER_NETFLASH_HMACMD5_KEY) \
				< $(IMAGE) >> $(IMAGE)
else
	$(ROOTDIR)/tools/cksum -b -o 2 $(IMAGE) >> $(IMAGE)
endif
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(IMAGE) /tftpboot/.

#############################################################################

include $(ROOTDIR)/vendors/config/config.dev
