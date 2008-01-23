ifdef CONFIG_USER_SAMBA_SHARED
SHARED := enable
else
SHARED := disable
endif

CONFOPTS = --prefix= --with-smbmount --host=arm-linux-elf --with-privatedir=/etc/ \
			--with-configdir=/etc/ --with-logfilebase=/var/log \
			--without-ldap --disable-cups \
			--$(SHARED)-shared --disable-debug --disable-developer \
			--disable-krb5developer --disable-dmalloc --disable-nls

ifeq ($(ARCH),arm)
CFLAGS := $(CFLAGS:-O1=-Os)
endif

all: build/build
	$(MAKE) -C build

build/build: makefile
	rm -rf build
	mkdir build
	(cd build; sh ../configure $(CONFIGURE_OPTS) $(CONFOPTS))
	touch build/build
clean:
	rm -rf build

romfs:
	arm-linux-elf-flthdr -s 30000 build/bin/smbclient
	$(ROMFSINST) -e CONFIG_USER_SAMBA_SHARED build/bin/libsmbclient.so /lib/libsmbclient.so.0
	$(ROMFSINST) -e CONFIG_USER_SAMBA_NMBD build/bin/nmbd /bin/nmbd
	$(ROMFSINST) -e CONFIG_USER_SAMBA_SMBD build/bin/smbd /bin/smbd
	$(ROMFSINST) -e CONFIG_USER_SAMBA_SMBD build/bin/smbpasswd /bin/smbpasswd
	$(ROMFSINST) -e CONFIG_USER_SAMBA_SMBD build/bin/smbclient /bin/smbclient
	$(ROMFSINST) -e CONFIG_USER_SAMBA_SMBMOUNT build/bin/smbmnt /bin/smbmnt
	$(ROMFSINST) -e CONFIG_USER_SAMBA_SMBMOUNT build/bin/smbmount /bin/smbmount
	$(ROMFSINST) -e CONFIG_USER_SAMBA_SMBUMOUNT build/bin/smbumount /bin/smbumount
	$(ROMFSINST) -e CONFIG_USER_SAMBA_SMBD ../examples/smb.conf.dslinux /etc/smb.conf
	$(ROMFSINST) -e CONFIG_USER_SAMBA_SMBD \
	                -a "netbios-ssn 139/tcp" /etc/services
	$(ROMFSINST) -e CONFIG_USER_SAMBA_NMBD \
	                -a "netbios-ns 137/udp" /etc/services
	$(ROMFSINST) -e CONFIG_USER_SAMBA_SMBD \
	                -a "root::0:root" /etc/group
	$(ROMFSINST) -e CONFIG_USER_SAMBA_SMBD \
                        -a "#netbios-ssn stream tcp nowait root /usr/bin/smbd -F" /etc/inetd.conf.example
	$(ROMFSINST) -e CONFIG_USER_SAMBA_NMBD \
                        -a "#netbios-ns dgram udp wait root /usr/bin/nmbd -F" /etc/inetd.conf.example
