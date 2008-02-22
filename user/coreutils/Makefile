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

	$(ROMFSINST) -m ./src/man/base64.1 /usr/share/man/base64
	$(ROMFSINST) -m ./src/man/basename.1 /usr/share/man/basename
	$(ROMFSINST) -m ./src/man/cat.1 /usr/share/man/cat
	$(ROMFSINST) -m ./src/man/chgrp.1 /usr/share/man/chgrp
	$(ROMFSINST) -m ./src/man/chmod.1 /usr/share/man/chmod
	$(ROMFSINST) -m ./src/man/chown.1 /usr/share/man/chown
	$(ROMFSINST) -m ./src/man/chroot.1 /usr/share/man/chroot
	$(ROMFSINST) -m ./src/man/cksum.1 /usr/share/man/cksum
	$(ROMFSINST) -m ./src/man/comm.1 /usr/share/man/comm
	$(ROMFSINST) -m ./src/man/cp.1 /usr/share/man/cp
	$(ROMFSINST) -m ./src/man/csplit.1 /usr/share/man/csplit
	$(ROMFSINST) -m ./src/man/cut.1 /usr/share/man/cut
	$(ROMFSINST) -m ./src/man/date.1 /usr/share/man/date
	$(ROMFSINST) -m ./src/man/dd.1 /usr/share/man/dd
	$(ROMFSINST) -m ./src/man/df.1 /usr/share/man/df
	$(ROMFSINST) -m ./src/man/dir.1 /usr/share/man/dir
	$(ROMFSINST) -m ./src/man/dircolors.1 /usr/share/man/dircolors
	$(ROMFSINST) -m ./src/man/dirname.1 /usr/share/man/dirname
	$(ROMFSINST) -m ./src/man/du.1 /usr/share/man/du
	$(ROMFSINST) -m ./src/man/echo.1 /usr/share/man/echo
	$(ROMFSINST) -m ./src/man/env.1 /usr/share/man/env
	$(ROMFSINST) -m ./src/man/expand.1 /usr/share/man/expand
	$(ROMFSINST) -m ./src/man/expr.1 /usr/share/man/expr
	$(ROMFSINST) -m ./src/man/factor.1 /usr/share/man/factor
	$(ROMFSINST) -m ./src/man/false.1 /usr/share/man/false
	$(ROMFSINST) -m ./src/man/fmt.1 /usr/share/man/fmt
	$(ROMFSINST) -m ./src/man/fold.1 /usr/share/man/fold
	$(ROMFSINST) -m ./src/man/groups.1 /usr/share/man/groups
	$(ROMFSINST) -m ./src/man/head.1 /usr/share/man/head
	$(ROMFSINST) -m ./src/man/hostid.1 /usr/share/man/hostid
	$(ROMFSINST) -m ./src/man/hostname.1 /usr/share/man/hostname
	$(ROMFSINST) -m ./src/man/id.1 /usr/share/man/id
	$(ROMFSINST) -m ./src/man/install.1 /usr/share/man/install
	$(ROMFSINST) -m ./src/man/join.1 /usr/share/man/join
	$(ROMFSINST) -m ./src/man/kill.1 /usr/share/man/kill
	$(ROMFSINST) -m ./src/man/link.1 /usr/share/man/link
	$(ROMFSINST) -m ./src/man/ln.1 /usr/share/man/ln
	$(ROMFSINST) -m ./src/man/logname.1 /usr/share/man/logname
	$(ROMFSINST) -m ./src/man/ls.1 /usr/share/man/ls
	$(ROMFSINST) -m ./src/man/md5sum.1 /usr/share/man/md5sum
	$(ROMFSINST) -m ./src/man/mkdir.1 /usr/share/man/mkdir
	$(ROMFSINST) -m ./src/man/mkfifo.1 /usr/share/man/mkfifo
	$(ROMFSINST) -m ./src/man/mknod.1 /usr/share/man/mknod
	$(ROMFSINST) -m ./src/man/mv.1 /usr/share/man/mv
	$(ROMFSINST) -m ./src/man/nice.1 /usr/share/man/nice
	$(ROMFSINST) -m ./src/man/nl.1 /usr/share/man/nl
	$(ROMFSINST) -m ./src/man/nohup.1 /usr/share/man/nohup
	$(ROMFSINST) -m ./src/man/od.1 /usr/share/man/od
	$(ROMFSINST) -m ./src/man/paste.1 /usr/share/man/paste
	$(ROMFSINST) -m ./src/man/pathchk.1 /usr/share/man/pathchk
	$(ROMFSINST) -m ./src/man/pinky.1 /usr/share/man/pinky
	$(ROMFSINST) -m ./src/man/pr.1 /usr/share/man/pr
	$(ROMFSINST) -m ./src/man/printenv.1 /usr/share/man/printenv
	$(ROMFSINST) -m ./src/man/printf.1 /usr/share/man/printf
	$(ROMFSINST) -m ./src/man/ptx.1 /usr/share/man/ptx
	$(ROMFSINST) -m ./src/man/pwd.1 /usr/share/man/pwd
	$(ROMFSINST) -m ./src/man/readlink.1 /usr/share/man/readlink
	$(ROMFSINST) -m ./src/man/rm.1 /usr/share/man/rm
	$(ROMFSINST) -m ./src/man/rmdir.1 /usr/share/man/rmdir
	$(ROMFSINST) -m ./src/man/seq.1 /usr/share/man/seq
	$(ROMFSINST) -m ./src/man/sha1sum.1 /usr/share/man/sha1sum
	$(ROMFSINST) -m ./src/man/sha224sum.1 /usr/share/man/sha224sum
	$(ROMFSINST) -m ./src/man/sha256sum.1 /usr/share/man/sha256sum
	$(ROMFSINST) -m ./src/man/sha384sum.1 /usr/share/man/sha384sum
	$(ROMFSINST) -m ./src/man/sha512sum.1 /usr/share/man/sha512sum
	$(ROMFSINST) -m ./src/man/shred.1 /usr/share/man/shred
	$(ROMFSINST) -m ./src/man/shuf.1 /usr/share/man/shuf
	$(ROMFSINST) -m ./src/man/sleep.1 /usr/share/man/sleep
	$(ROMFSINST) -m ./src/man/sort.1 /usr/share/man/sort
	$(ROMFSINST) -m ./src/man/split.1 /usr/share/man/split
	$(ROMFSINST) -m ./src/man/stat.1 /usr/share/man/stat
	$(ROMFSINST) -m ./src/man/stty.1 /usr/share/man/stty
	$(ROMFSINST) -m ./src/man/su.1 /usr/share/man/su
	$(ROMFSINST) -m ./src/man/sum.1 /usr/share/man/sum
	$(ROMFSINST) -m ./src/man/sync.1 /usr/share/man/sync
	$(ROMFSINST) -m ./src/man/tac.1 /usr/share/man/tac
	$(ROMFSINST) -m ./src/man/tail.1 /usr/share/man/tail
	$(ROMFSINST) -m ./src/man/tee.1 /usr/share/man/tee
	$(ROMFSINST) -m ./src/man/test.1 /usr/share/man/test
	$(ROMFSINST) -m ./src/man/touch.1 /usr/share/man/touch
	$(ROMFSINST) -m ./src/man/tr.1 /usr/share/man/tr
	$(ROMFSINST) -m ./src/man/true.1 /usr/share/man/true
	$(ROMFSINST) -m ./src/man/tsort.1 /usr/share/man/tsort
	$(ROMFSINST) -m ./src/man/tty.1 /usr/share/man/tty
	$(ROMFSINST) -m ./src/man/uname.1 /usr/share/man/uname
	$(ROMFSINST) -m ./src/man/unexpand.1 /usr/share/man/unexpand
	$(ROMFSINST) -m ./src/man/uniq.1 /usr/share/man/uniq
	$(ROMFSINST) -m ./src/man/unlink.1 /usr/share/man/unlink
	$(ROMFSINST) -m ./src/man/uptime.1 /usr/share/man/uptime
	$(ROMFSINST) -m ./src/man/users.1 /usr/share/man/users
	$(ROMFSINST) -m ./src/man/vdir.1 /usr/share/man/vdir
	$(ROMFSINST) -m ./src/man/wc.1 /usr/share/man/wc
	$(ROMFSINST) -m ./src/man/who.1 /usr/share/man/who
	$(ROMFSINST) -m ./src/man/whoami.1 /usr/share/man/whoami
	$(ROMFSINST) -m ./src/man/yes.1 /usr/share/man/yes
