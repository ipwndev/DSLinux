# Makefile for coreutils

all: .compiled

COREUTILS_CONFIGURE_FLAGS=--disable-nls --disable-largefile \
				--without-included-regex

.configured:
	cd src/ &&\
		./configure --host=arm-linux-elf \
		--prefix=/nonexistent $(COREUTILS_CONFIGURE_FLAGS)
	touch $@

.compiled: .configured
	$(MAKE) -C src
	touch $@

clean:
	-[ -f src/Makefile ] && $(MAKE) -C src distclean
	rm -f .configured .compiled
romfs:
	arm-linux-elf-flthdr -s 30000 ./src/src/ls
	$(ROMFSINST) ./src/src/[ /usr/bin/[
	$(ROMFSINST) ./src/src/base64 /usr/bin/base64
	$(ROMFSINST) ./src/src/basename /usr/bin/basename
	$(ROMFSINST) ./src/src/cat /usr/bin/cat
	$(ROMFSINST) ./src/src/chgrp /usr/bin/chgrp
	$(ROMFSINST) ./src/src/chmod /usr/bin/chmod
	$(ROMFSINST) ./src/src/chown /usr/bin/chown
	$(ROMFSINST) ./src/src/chroot /usr/bin/chroot
	$(ROMFSINST) ./src/src/cksum /usr/bin/cksum
	$(ROMFSINST) ./src/src/comm /usr/bin/comm
	$(ROMFSINST) ./src/src/cp /usr/bin/cp
	$(ROMFSINST) ./src/src/csplit /usr/bin/csplit
	$(ROMFSINST) ./src/src/cut /usr/bin/cut
	$(ROMFSINST) ./src/src/date /usr/bin/date
	$(ROMFSINST) ./src/src/dd /usr/bin/dd
	$(ROMFSINST) ./src/src/dir /usr/bin/dir
	$(ROMFSINST) ./src/src/dircolors /usr/bin/dircolors
	$(ROMFSINST) ./src/src/dirname /usr/bin/dirname
	$(ROMFSINST) ./src/src/du /usr/bin/du
	$(ROMFSINST) ./src/src/echo /usr/bin/echo
	$(ROMFSINST) ./src/src/env /usr/bin/env
	$(ROMFSINST) ./src/src/expand /usr/bin/expand
	$(ROMFSINST) ./src/src/expr /usr/bin/expr
	$(ROMFSINST) ./src/src/factor /usr/bin/factor
	$(ROMFSINST) ./src/src/false /usr/bin/false
	$(ROMFSINST) ./src/src/fmt /usr/bin/fmt
	$(ROMFSINST) ./src/src/fold /usr/bin/fold
	$(ROMFSINST) ./src/src/ginstall /usr/bin/ginstall
	$(ROMFSINST) ./src/src/head /usr/bin/head
	$(ROMFSINST) ./src/src/hostid /usr/bin/hostid
	$(ROMFSINST) ./src/src/hostname /usr/bin/hostname
	$(ROMFSINST) ./src/src/id /usr/bin/id
	$(ROMFSINST) ./src/src/join /usr/bin/join
	$(ROMFSINST) ./src/src/kill /usr/bin/kill
	$(ROMFSINST) ./src/src/link /usr/bin/link
	$(ROMFSINST) ./src/src/ln /usr/bin/ln
	$(ROMFSINST) ./src/src/logname /usr/bin/logname
	$(ROMFSINST) ./src/src/ls /usr/bin/ls
	$(ROMFSINST) ./src/src/md5sum /usr/bin/md5sum
	$(ROMFSINST) ./src/src/mkdir /usr/bin/mkdir
	$(ROMFSINST) ./src/src/mkfifo /usr/bin/mkfifo
	$(ROMFSINST) ./src/src/mknod /usr/bin/mknod
	$(ROMFSINST) ./src/src/mv /usr/bin/mv
	$(ROMFSINST) ./src/src/nice /usr/bin/nice
	$(ROMFSINST) ./src/src/nl /usr/bin/nl
	$(ROMFSINST) ./src/src/nohup /usr/bin/nohup
	$(ROMFSINST) ./src/src/od /usr/bin/od
	$(ROMFSINST) ./src/src/paste /usr/bin/paste
	$(ROMFSINST) ./src/src/pathchk /usr/bin/pathchk
	$(ROMFSINST) ./src/src/pinky /usr/bin/pinky
	$(ROMFSINST) ./src/src/pr /usr/bin/pr
	$(ROMFSINST) ./src/src/printenv /usr/bin/printenv
	$(ROMFSINST) ./src/src/printf /usr/bin/printf
	$(ROMFSINST) ./src/src/ptx /usr/bin/ptx
	$(ROMFSINST) ./src/src/pwd /usr/bin/pwd
	$(ROMFSINST) ./src/src/readlink /usr/bin/readlink
	$(ROMFSINST) ./src/src/rm /usr/bin/rm
	$(ROMFSINST) ./src/src/rmdir /usr/bin/rmdir
	$(ROMFSINST) ./src/src/setuidgid /usr/bin/setuidgid
	$(ROMFSINST) ./src/src/sha1sum /usr/bin/sha1sum
	$(ROMFSINST) ./src/src/sha224sum /usr/bin/sha224sum
	$(ROMFSINST) ./src/src/sha256sum /usr/bin/sha256sum
	$(ROMFSINST) ./src/src/sha384sum /usr/bin/sha384sum
	$(ROMFSINST) ./src/src/sha512sum /usr/bin/sha512sum
	$(ROMFSINST) ./src/src/shred /usr/bin/shred
	$(ROMFSINST) ./src/src/shuf /usr/bin/shuf
	$(ROMFSINST) ./src/src/sleep /usr/bin/sleep
	$(ROMFSINST) ./src/src/sort /usr/bin/sort
	$(ROMFSINST) ./src/src/split /usr/bin/split
	$(ROMFSINST) ./src/src/stat /usr/bin/stat
	$(ROMFSINST) ./src/src/stty /usr/bin/stty
	$(ROMFSINST) ./src/src/su /usr/bin/su
	$(ROMFSINST) ./src/src/sum /usr/bin/sum
	$(ROMFSINST) ./src/src/sync /usr/bin/sync
	$(ROMFSINST) ./src/src/tac /usr/bin/tac
	$(ROMFSINST) ./src/src/tail /usr/bin/tail
	$(ROMFSINST) ./src/src/tee /usr/bin/tee
	$(ROMFSINST) ./src/src/test /usr/bin/test
	$(ROMFSINST) ./src/src/touch /usr/bin/touch
	$(ROMFSINST) ./src/src/tr /usr/bin/tr
	$(ROMFSINST) ./src/src/true /usr/bin/true
	$(ROMFSINST) ./src/src/tsort /usr/bin/tsort
	$(ROMFSINST) ./src/src/tty /usr/bin/tty
	$(ROMFSINST) ./src/src/uname /usr/bin/uname
	$(ROMFSINST) ./src/src/unexpand /usr/bin/unexpand
	$(ROMFSINST) ./src/src/uniq /usr/bin/uniq
	$(ROMFSINST) ./src/src/unlink /usr/bin/unlink
	$(ROMFSINST) ./src/src/uptime /usr/bin/uptime
	$(ROMFSINST) ./src/src/users /usr/bin/users
	$(ROMFSINST) ./src/src/vdir /usr/bin/vdir
	$(ROMFSINST) ./src/src/wc /usr/bin/wc
	$(ROMFSINST) ./src/src/who /usr/bin/who
	$(ROMFSINST) ./src/src/whoami /usr/bin/whoami
	$(ROMFSINST) ./src/src/yes /usr/bin/yes
