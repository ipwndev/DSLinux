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
	$(ROMFSINST) ./src/src/[ /usr/bin/coreutils/[
	$(ROMFSINST) ./src/src/base64 /usr/bin/coreutils/base64
	$(ROMFSINST) ./src/src/basename /usr/bin/coreutils/basename
	$(ROMFSINST) ./src/src/cat /usr/bin/coreutils/cat
	$(ROMFSINST) ./src/src/chgrp /usr/bin/coreutils/chgrp
	$(ROMFSINST) ./src/src/chmod /usr/bin/coreutils/chmod
	$(ROMFSINST) ./src/src/chown /usr/bin/coreutils/chown
	$(ROMFSINST) ./src/src/chroot /usr/bin/coreutils/chroot
	$(ROMFSINST) ./src/src/cksum /usr/bin/coreutils/cksum
	$(ROMFSINST) ./src/src/comm /usr/bin/coreutils/comm
	$(ROMFSINST) ./src/src/cp /usr/bin/coreutils/cp
	$(ROMFSINST) ./src/src/csplit /usr/bin/coreutils/csplit
	$(ROMFSINST) ./src/src/cut /usr/bin/coreutils/cut
	$(ROMFSINST) ./src/src/date /usr/bin/coreutils/date
	$(ROMFSINST) ./src/src/dd /usr/bin/coreutils/dd
	$(ROMFSINST) ./src/src/dir /usr/bin/coreutils/dir
	$(ROMFSINST) ./src/src/dircolors /usr/bin/coreutils/dircolors
	$(ROMFSINST) ./src/src/dirname /usr/bin/coreutils/dirname
	$(ROMFSINST) ./src/src/du /usr/bin/coreutils/du
	$(ROMFSINST) ./src/src/echo /usr/bin/coreutils/echo
	$(ROMFSINST) ./src/src/env /usr/bin/coreutils/env
	$(ROMFSINST) ./src/src/expand /usr/bin/coreutils/expand
	$(ROMFSINST) ./src/src/expr /usr/bin/coreutils/expr
	$(ROMFSINST) ./src/src/factor /usr/bin/coreutils/factor
	$(ROMFSINST) ./src/src/false /usr/bin/coreutils/false
	$(ROMFSINST) ./src/src/fmt /usr/bin/coreutils/fmt
	$(ROMFSINST) ./src/src/fold /usr/bin/coreutils/fold
	$(ROMFSINST) ./src/src/ginstall /usr/bin/coreutils/ginstall
	$(ROMFSINST) ./src/src/head /usr/bin/coreutils/head
	$(ROMFSINST) ./src/src/hostid /usr/bin/coreutils/hostid
	$(ROMFSINST) ./src/src/hostname /usr/bin/coreutils/hostname
	$(ROMFSINST) ./src/src/id /usr/bin/coreutils/id
	$(ROMFSINST) ./src/src/join /usr/bin/coreutils/join
	$(ROMFSINST) ./src/src/kill /usr/bin/coreutils/kill
	$(ROMFSINST) ./src/src/link /usr/bin/coreutils/link
	$(ROMFSINST) ./src/src/ln /usr/bin/coreutils/ln
	$(ROMFSINST) ./src/src/logname /usr/bin/coreutils/logname
	$(ROMFSINST) ./src/src/ls /usr/bin/coreutils/ls
	$(ROMFSINST) ./src/src/md5sum /usr/bin/coreutils/md5sum
	$(ROMFSINST) ./src/src/mkdir /usr/bin/coreutils/mkdir
	$(ROMFSINST) ./src/src/mkfifo /usr/bin/coreutils/mkfifo
	$(ROMFSINST) ./src/src/mknod /usr/bin/coreutils/mknod
	$(ROMFSINST) ./src/src/mv /usr/bin/coreutils/mv
	$(ROMFSINST) ./src/src/nice /usr/bin/coreutils/nice
	$(ROMFSINST) ./src/src/nl /usr/bin/coreutils/nl
	$(ROMFSINST) ./src/src/nohup /usr/bin/coreutils/nohup
	$(ROMFSINST) ./src/src/od /usr/bin/coreutils/od
	$(ROMFSINST) ./src/src/paste /usr/bin/coreutils/paste
	$(ROMFSINST) ./src/src/pathchk /usr/bin/coreutils/pathchk
	$(ROMFSINST) ./src/src/pinky /usr/bin/coreutils/pinky
	$(ROMFSINST) ./src/src/pr /usr/bin/coreutils/pr
	$(ROMFSINST) ./src/src/printenv /usr/bin/coreutils/printenv
	$(ROMFSINST) ./src/src/printf /usr/bin/coreutils/printf
	$(ROMFSINST) ./src/src/ptx /usr/bin/coreutils/ptx
	$(ROMFSINST) ./src/src/pwd /usr/bin/coreutils/pwd
	$(ROMFSINST) ./src/src/readlink /usr/bin/coreutils/readlink
	$(ROMFSINST) ./src/src/rm /usr/bin/coreutils/rm
	$(ROMFSINST) ./src/src/rmdir /usr/bin/coreutils/rmdir
	$(ROMFSINST) ./src/src/setuidgid /usr/bin/coreutils/setuidgid
	$(ROMFSINST) ./src/src/sha1sum /usr/bin/coreutils/sha1sum
	$(ROMFSINST) ./src/src/sha224sum /usr/bin/coreutils/sha224sum
	$(ROMFSINST) ./src/src/sha256sum /usr/bin/coreutils/sha256sum
	$(ROMFSINST) ./src/src/sha384sum /usr/bin/coreutils/sha384sum
	$(ROMFSINST) ./src/src/sha512sum /usr/bin/coreutils/sha512sum
	$(ROMFSINST) ./src/src/shred /usr/bin/coreutils/shred
	$(ROMFSINST) ./src/src/shuf /usr/bin/coreutils/shuf
	$(ROMFSINST) ./src/src/sleep /usr/bin/coreutils/sleep
	$(ROMFSINST) ./src/src/sort /usr/bin/coreutils/sort
	$(ROMFSINST) ./src/src/split /usr/bin/coreutils/split
	$(ROMFSINST) ./src/src/stat /usr/bin/coreutils/stat
	$(ROMFSINST) ./src/src/stty /usr/bin/coreutils/stty
	$(ROMFSINST) ./src/src/su /usr/bin/coreutils/su
	$(ROMFSINST) ./src/src/sum /usr/bin/coreutils/sum
	$(ROMFSINST) ./src/src/sync /usr/bin/coreutils/sync
	$(ROMFSINST) ./src/src/tac /usr/bin/coreutils/tac
	$(ROMFSINST) ./src/src/tail /usr/bin/coreutils/tail
	$(ROMFSINST) ./src/src/tee /usr/bin/coreutils/tee
	$(ROMFSINST) ./src/src/test /usr/bin/coreutils/test
	$(ROMFSINST) ./src/src/touch /usr/bin/coreutils/touch
	$(ROMFSINST) ./src/src/tr /usr/bin/coreutils/tr
	$(ROMFSINST) ./src/src/true /usr/bin/coreutils/true
	$(ROMFSINST) ./src/src/tsort /usr/bin/coreutils/tsort
	$(ROMFSINST) ./src/src/tty /usr/bin/coreutils/tty
	$(ROMFSINST) ./src/src/uname /usr/bin/coreutils/uname
	$(ROMFSINST) ./src/src/unexpand /usr/bin/coreutils/unexpand
	$(ROMFSINST) ./src/src/uniq /usr/bin/coreutils/uniq
	$(ROMFSINST) ./src/src/unlink /usr/bin/coreutils/unlink
	$(ROMFSINST) ./src/src/uptime /usr/bin/coreutils/uptime
	$(ROMFSINST) ./src/src/users /usr/bin/coreutils/users
	$(ROMFSINST) ./src/src/vdir /usr/bin/coreutils/vdir
	$(ROMFSINST) ./src/src/wc /usr/bin/coreutils/wc
	$(ROMFSINST) ./src/src/who /usr/bin/coreutils/who
	$(ROMFSINST) ./src/src/whoami /usr/bin/coreutils/whoami
	$(ROMFSINST) ./src/src/yes /usr/bin/coreutils/yes
