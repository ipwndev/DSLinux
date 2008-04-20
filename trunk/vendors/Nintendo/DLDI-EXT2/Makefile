#
#	Makefile -- Build instructions for DSLinux/GBAMP-EXT2
#

include $(LINUX_CONFIG)

ROMFSIMG = $(IMAGEDIR)/romfs.img
ELFIMAGE = $(ROOTDIR)/$(LINUXDIR)/linux
IMAGE    = $(IMAGEDIR)/dslinux.ds.gba

# This really is the FAT filesystem, both filesystems are mounted by this build.
FATFS	 = media


ROMFS_DIRS = bin boot dev proc sbin $(FATFS)

FS_DIRS = 	home etc lib usr/bin usr/bin/coreutils usr/games usr/lib \
		usr/share/udhcpc usr/share/man tmp var/run var/www \
		var/lock var/log var/mail 

DEVICES = \
	tty,c,5,0	console,c,5,1      cua0,c,5,64      cua1,c,5,65 \
	\
	mem,c,1,1	kmem,c,1,2         null,c,1,3       ram0,b,1,0 \
	ram1,b,1,1	hda,b,3,0          hda1,b,3,1 \
	hda2,b,3,2	hda3,b,3,3         hda4,b,3,4 \
	\
	ptyp0,c,2,0    ptyp1,c,2,1        ptyp2,c,2,2      ptyp3,c,2,3 \
	ptyp4,c,2,4    ptyp5,c,2,5        ptyp6,c,2,6      ptyp7,c,2,7 \
	ptyp8,c,2,8    ptyp9,c,2,9        ptypa,c,2,10     ptypb,c,2,11 \
	ptypc,c,2,12   ptypd,c,2,13       ptype,c,2,14     ptypf,c,2,15 \
	\
	dldi,b,240,0 \
	dldi1,b,240,1  dldi2,b,240,2      dldi3,b,240,3    dldi4,b,240,4 \
	\
	dsp,c,14,3\
	\
	fb0,c,29,0\
	fb1,c,29,1\
	\
	psaux,c,10,1   mouse0,c,13,32	  touch,c,13,65 \
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
	ttyp0,c,3,0    ttyp1,c,3,1        ttyp2,c,3,2      ttyp3,c,3,3 \
	ttyp4,c,3,4    ttyp5,c,3,5        ttyp6,c,3,6      ttyp7,c,3,7 \
	ttyp8,c,3,8    ttyp9,c,3,9        ttypa,c,3,10     ttypb,c,3,11 \
	ttypc,c,3,12   ttypd,c,3,13       ttype,c,3,14     ttypf,c,3,15 \
	\
	zero,c,1,5     random,c,1,8       urandom,c,1,9


clean:

romfs:
	-rm -r $(ROMFSDIR)
	[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	
	for i in $(DEVICES); do \
		mknod $(ROMFSDIR)/dev/`echo $$i | sed -e 's/,/ /g'`; \
	done
	# these permissions are needed for openpty and family to work
	# on non-ptmx ptys
	chmod 620 $(ROMFSDIR)/dev/[pt]ty[pqrsPQRS][0-9a-f]
	for i in $(FS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	#Make symlinks
	$(ROMFSINST) /boot/rc
	$(ROMFSINST) /boot/inittab
	$(ROMFSINST) /etc/issue
	$(ROMFSINST) $(NDS_COMMON)/rc.common /etc/rc.common
	$(ROMFSINST) $(NDS_COMMON)/rc.defaults /etc/rc.defaults
	$(ROMFSINST) $(NDS_COMMON)/rc.d /etc/rc.d
	#$(ROMFSINST) $(NDS_COMMON)/passwd /etc/passwd
	$(ROMFSINST) -p +x $(NDS_COMMON)/default.script /usr/share/udhcpc/default.script
	echo "$(VERSIONSTR) -- " `date` > $(ROMFSDIR)/etc/version
	for i in `find $(ROMFSDIR)/$(FATFS)/linux -type d -empty` ; do touch $$i/.keep; done
image:
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)
	$(CROSS_COMPILE)objcopy -O binary $(ELFIMAGE) $(IMAGEDIR)/arm9.bin
	ndstool -c $(IMAGEDIR)/dslinux.nds -9 $(IMAGEDIR)/arm9.bin \
		-r9 0x02000000 -e9 0x02000000  \
		-7 $(ROOTDIR)/$(LINUXDIR)/arch/arm/mach-nds/arm7.bin \
			-r7 0x03800000 -e7 0x03800000  \
			-b $(NDS_COMMON)/tux.bmp "DSLinux;Port of Linux;to the DS!" 
	chmod +x $(ROMFSDIR)/etc/rc.d/*
	tar -C $(ROMFSDIR) -cvzf $(IMAGEDIR)/dslinux-ext2-data.tgz .
	tar -C $(IMAGEDIR) -cvzf $(IMAGEDIR)/dslinux-DLDI-ext2.tgz dslinux.nds dslinux-ext2-data.tgz

