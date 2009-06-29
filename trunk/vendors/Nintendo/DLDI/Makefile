#
#	Makefile -- Build instructions for DSLinux/DLDI
#

include $(LINUX_CONFIG)

ROMFSIMG := $(IMAGEDIR)/romfs.img
ELFIMAGE := $(IMAGEDIR)/image.elf
FATFS	 := media
OPTDIR	 := opt

ROMFS_DIRS := boot dev dev/input proc sbin $(FATFS)/linux $(OPTDIR) tmp sys

FS_DIRS := etc/rc.d home lib usr/bin usr/bin/coreutils usr/games usr/lib usr/share/udhcpc \
	   var/lock var/tmp var/run var/log var/mail var/www usr/share/man usr/share/consolefonts \
	   usr/share/hinged

TTYP_DEVICES = 0 1 2 3 4 5 6 7 8 9 a b c d e f

INPUT_DEVICES := \
		js0,c,13,0 	js1,c,13,1 	mouse0,c,13,32	mouse1,c,13,33 \
		mice,c,13,63	event0,c,13,64	event1,c,13,65

DEVICES := \
	tty,c,5,0      console,c,5,1      cua0,c,5,64      cua1,c,5,65  \
	\
	mem,c,1,1      kmem,c,1,2         null,c,1,3 \
	\
	ram0,b,1,0     ram1,b,1,1 \
	\
	dldi,b,240,0 \
	dldi1,b,240,1  dldi2,b,240,2      dldi3,b,240,3    dldi4,b,240,4 \
	\
	ptyp0,c,2,0    ptyp1,c,2,1        ptyp2,c,2,2      ptyp3,c,2,3 \
	ptyp4,c,2,4    ptyp5,c,2,5        ptyp6,c,2,6      ptyp7,c,2,7 \
	ptyp8,c,2,8    ptyp9,c,2,9        ptypa,c,2,10     ptypb,c,2,11 \
	ptypc,c,2,12   ptypd,c,2,13       ptype,c,2,14     ptypf,c,2,15 \
	\
	dsp,c,14,3     mixer,c,14,0 \
	\
	fb0,c,29,0\
	fb1,c,29,1\
	\
	psaux,c,10,1   mouse0,c,13,32	  touch,c,13,65\
	\
	rom0,b,31,0    rom1,b,31,1        rom2,b,31,2      rom3,b,31,3 \
	rom4,b,31,4    rom5,b,31,5        rom6,b,31,6      rom7,b,31,7 \
	\
	mtd0,c,90,0    firmware,c,90,2 \
	\
	tun,c,10,200 \
	tty0,c,4,0     tty1,c,4,1         tty2,c,4,2       tty3,c,4,3 \
	tty4,c,4,4     tty5,c,4,5         tty6,c,4,6       tty7,c,4,7 \
	\
	\
	zero,c,1,5     random,c,1,8       urandom,c,1,9

.PHONY: clean
clean:

.PHONY: romfs
	# The purpose of this target is to create a basic romfs infrastructure.
romfs:
	# create directories in romfs
	-rm -r $(ROMFSDIR)
	[ -d $(ROMFSDIR) ] || mkdir -p $(ROMFSDIR)
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	
	# create device nodes
	for i in $(DEVICES); do \
		touch $(ROMFSDIR)/dev/@$$i; \
	done
	
	# create input device nodes
	for i in $(INPUT_DEVICES); do \
		touch $(ROMFSDIR)/dev/input/@$$i; \
	done
	# these permissions are needed for openpty and family to work
	# on non-ptmx ptys
	chmod 620 $(ROMFSDIR)/dev/@[pt]ty[pqrsPQRS][0-9a-f],*
	# create directories which will be symlinked to Flashcard
	for i in $(FS_DIRS); do \
		[ -d $(ROMFSDIR)/$(FATFS)/linux/$$i ] || mkdir -p $(ROMFSDIR)/$(FATFS)/linux/$$i; \
	done
	# create symlinks to Flashcard
	$(ROMFSINST) -s $(FATFS)/linux/var /var
	$(ROMFSINST) -s $(FATFS)/linux/lib /lib
	$(ROMFSINST) -s $(FATFS)/linux/usr /usr
	$(ROMFSINST) -s $(FATFS)/linux/usr/bin /bin
	$(ROMFSINST) -s $(FATFS)/linux/home /home
	$(ROMFSINST) -s $(FATFS)/linux/etc /etc
	$(ROMFSINST) -s /$(FATFS)/linux/var/run/log /dev/log
	
	#Create symlinks for ttyp devicess
	for i in $(TTYP_DEVICES); do \
		$(ROMFSINST) -s /var/run/ttyp$$i /dev/ttyp$$i; \
	done
	
	# copy some DLDI specific files to romfs
	$(ROMFSINST) /boot/rc
	$(ROMFSINST) /boot/inittab
	$(ROMFSINST) /$(FATFS)/linux/etc/issue
	# copy some Nintendo specific files to romfs
	$(ROMFSINST) $(NDS_COMMON)/rc.common /$(FATFS)/linux/etc/rc.common
	$(ROMFSINST) $(NDS_COMMON)/rc.defaults /$(FATFS)/linux/etc/rc.defaults
	for f in $(NDS_COMMON)/rc.d/*; do \
		[ -d $$f ] || $(ROMFSINST) $$f /$(FATFS)/linux/etc/rc.d/ ; \
	done
	# copy the default DHCPD script
	$(ROMFSINST) -p +x $(NDS_COMMON)/default.script /usr/share/udhcpc/default.script
	# create version information
	# If release version put svn info into revision
ifeq ($(CONFIG_DEFAULTS_RELEASE_BUILD),y)
	svn info $(ROOTDIR) > $(ROMFSDIR)/etc/revision
endif
	echo "$(VERSIONSTR) -- " `date` > $(ROMFSDIR)/etc/version
	# make sure that empty directories are not removed
	for i in `find $(ROMFSDIR)/$(FATFS)/linux -type d -empty` ; do touch $$i/.keep; done

.PHONY: finishrom
	# This target is called as first step to create an image.
	# The romfs is populated and has to be finished.
finishrom:
	# remove the symlink to $(FATFS)/linux/usr/bin where all these applications are now
	rm $(ROMFSDIR)/bin
	# create a new empty bin directory
	mkdir $(ROMFSDIR)/bin
	# Copy busybox symlinks to internal RAM.
	-for i in `sort $(ROOTDIR)/user/busybox/busybox.links` ; do\
		j=`basename $$i` ; \
		mv $(ROMFSDIR)/$(FATFS)/linux/usr/bin/$$j $(ROMFSDIR)/bin ; \
	done
	# Copy files to internal RAM. These files are needed to boot and mount the flashcard.
	-mv $(ROMFSDIR)/$(FATFS)/linux/usr/bin/busybox $(ROMFSDIR)/bin
	-mv $(ROMFSDIR)/$(FATFS)/linux/usr/bin/login $(ROMFSDIR)/bin
	-mv $(ROMFSDIR)/$(FATFS)/linux/usr/bin/init $(ROMFSDIR)/bin
	-mv $(ROMFSDIR)/$(FATFS)/linux/usr/bin/autologin $(ROMFSDIR)/bin
	# Compress the data area of these programs
	$(CROSS)flthdr -d $(ROMFSDIR)/bin/*
	# If we have external RAM, we can spend some of our precious internal RAM
        # to speed up some applications.
	-cp -p $(ROMFSDIR)/$(FATFS)/linux/usr/bin/nano-X $(ROMFSDIR)/$(OPTDIR)
	-cp -p $(ROMFSDIR)/$(FATFS)/linux/usr/bin/madplay $(ROMFSDIR)/$(OPTDIR)
	# Compress the data area of these programs
	$(CROSS)flthdr -d $(ROMFSDIR)/$(OPTDIR)/*

.PHONY: setupimage
	# Setup the basic image directories
setupimage: finishrom
	# create the image directory
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)
	# create the linux directory for the flashcard
	[ -d $(IMAGEDIR)/linux ] || mkdir -p $(IMAGEDIR)/linux
	-rm -r $(IMAGEDIR)/linux
	# move the files for the flashcard out of the romfs
	mv $(ROMFSDIR)/$(FATFS)/linux $(IMAGEDIR)

.PHONY: dslinux dslinuxm
	# dslinux(m) builds the image from the romfs.
	# In the (m) version, the $(OPTDIR) is included.
dslinux-y := -x /$(OPTDIR)
dslinux dslinuxm: setupimage
	genromfs -v -V "ROMdisk" -f $(IMAGEDIR)/$@.img -d $(ROMFSDIR) $($@-y)
	BSS=`$(CROSS_COMPILE)objdump --headers $(ROOTDIR)/$(LINUXDIR)/linux | \
	grep .bss` && \
	BSSADDR=`set -- $${BSS} ; echo 0x$${4}` && \
	BSSSIZE=`set -- $${BSS} ; echo 0x$${3}` && \
	ADDR=`echo $${BSSADDR} $${BSSSIZE} | \
	perl -ane 'printf "0x%x\n",hex($$F[0]) + hex($$F[1])' ` && \
	echo "BSS=$${BSSADDR},$${BSSSIZE} ADDR=$${ADDR}" && \
	$(CROSS_COMPILE)objcopy --add-section=.romfs=$(IMAGEDIR)/$@.img \
	--adjust-section-vma=.romfs=$${ADDR} --no-adjust-warnings \
	--set-section-flags=.romfs=alloc,load,data   \
	$(ROOTDIR)/$(LINUXDIR)/linux $(IMAGEDIR)/$@.elf 2> /dev/null && \
	$(CROSS_COMPILE)objcopy -O binary $(IMAGEDIR)/$@.elf $(IMAGEDIR)/arm9.bin && \
	ndstool -c $(IMAGEDIR)/$@.nds -9 $(IMAGEDIR)/arm9.bin \
		-r9 0x02000000 -e9 0x02000000  \
		-7 $(ROOTDIR)/$(LINUXDIR)/arch/arm/mach-nds/arm7.bin \
	       	-r7 0x03800000 -e7 0x03800000  \
		-b $(NDS_COMMON)/tux.bmp "DSLinux;Port of Linux;to the DS!"

.PHONY: image
	# master command for image generation. Delegates all of the hard stuff to
	# other targets and collect the results.
image:	dslinux dslinuxm
	( cd $(IMAGEDIR); tar -cvzf dslinux-dldi.tgz linux dslinux.nds dslinuxm.nds) ;
