#!/bin/sh

PROJECT_HOME="$HOME/src/ncftp/current"
if [ ! -d "$PROJECT_HOME" ] ; then
	/bin/ls -ld "$PROJECT_HOME"
	exit 1
fi
cd "$PROJECT_HOME"

a=`pwd`
first=yes
didone=no
for f in \
	./configure.in \
	sio/configure.in \
	Strn/configure.in \
	libncftp/configure.in \
	/home/gleason/src/libncftp/current/libncftp/configure.in \
; do
	abs=no
	case "$f" in
		/*) if [ -f "$f" ] ; then abs=yes ; fi ;;
	esac
	cd "$a"
	if [ -f "$a/$f" ] || [ "$abs" = yes ] ; then
		dir=`dirname "$f"`
		first=no
		cd "$dir" || exit 1
		dir=`pwd`
		echo "$dir" | awk -v c="*" -v cols="${COLUMNS-80}" '{if (cols < 20) {cols=80;} line = " " $0 " "; len = length(line); if (len/2 < cols/2) {k=cols/2 - len/2; i=k; while (--i >= 0) {printf("%s", c);} printf("%s", line); for (i=k+length(line); i<cols; i++) {printf("%s", c);}printf("\n");} else {printf("%s\n", $0);}}'
		is_diff=no

		/bin/mv config.h.in config.h.in.orig 2>/dev/null
		autoheader 2>&1 | fgrep -v AC_TRY_RUN
		if [ ! -f config.h.in ] ; then
			echo "* Warning: config.h.in not generated"
			if [ -f config.h.in.orig ] ; then
				echo "* Restoring previous config.h.in"
				/bin/mv config.h.in.orig config.h.in
			fi
		elif [ -f config.h.in.orig ] && ! cmp config.h.in.orig config.h.in >/dev/null ; then
			# echo "Running" diff -u config.h.in.orig config.h.in
			# diff -u config.h.in.orig config.h.in
			/bin/rm config.h.in.orig
			/bin/ls -l config.h.in
			is_diff=yes
		else
			# echo "(config.h.in is unchanged)"
			/bin/mv config.h.in.orig config.h.in
		fi

		/bin/mv configure configure.orig 2>/dev/null
		autoconf 2>&1 | fgrep -v AC_TRY_RUN
		if [ ! -f configure ] ; then
			echo "* Warning: configure not generated"
			if [ -f configure.orig ] ; then
				echo "* Restoring previous configure"
				/bin/mv configure.orig configure
			fi
		elif [ -f configure.orig ] && ! cmp configure.orig configure >/dev/null ; then
			# echo "Running" diff -u configure.orig configure
			# diff -u configure.orig configure
			/bin/rm configure.orig
			/bin/ls -l configure
			is_diff=yes
		else
			# echo "(configure is unchanged)"
			/bin/mv configure.orig configure
		fi

		if [ "$is_diff" = yes ] && [ -f config.cache ] ; then
			echo "Removing `pwd`/config.cache."
			/bin/rm config.cache
		fi
		if [ "$first" = no ] && [ "$is_diff" = yes ] ; then
			echo; echo; echo
		fi
		didone=yes
	fi
done

if [ "$didone" = no ] ; then
	echo "* No autoconf files found -- edit this script and change the PROJECT_HOME." 1>&2
	exit 1
fi
exit 0
