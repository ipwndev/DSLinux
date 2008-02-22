all:
	$(MAKE) -C source
clean:
	$(MAKE) -C source clean
romfs:
	$(MAKE) -C source romfs
	$(ROMFSINST) -m docs/manpages/nmbd.8 /usr/share/man/nmbd
	$(ROMFSINST) -m docs/manpages/smbd.8 /usr/share/man/smbd
	$(ROMFSINST) -m docs/manpages/smbpasswd.5 /usr/share/man/smbpasswd
