#! /bin/sh
# $Id$
#
# Adjust includes for header files that reside in a subdirectory of
# /usr/include, etc.
#
# Parameters (the first case creates the sed script):
#	$1 is the target directory
#	$2 is the source directory
# or (the second case does the install, using the sed script):
#	$1 is the script to use for installing
#	$2 is the target directory
#	$3 is the source directory
#	$4 is the file to install, editing source/target/etc.

PACKAGE=DIALOG
PKGNAME=DLG
CONFIGH=dlg_config.h

TMPSED=headers.sed

if test $# = 2 ; then
	rm -f $TMPSED
	DST=$1
	REF=$2
	LEAF=`basename $DST`
	case $DST in
	/*/include/$LEAF)
		END=`basename $DST`
		for i in $REF/*.h
		do
			NAME=`basename $i`
			echo "s/<$NAME>/<$END\/$NAME>/g" >> $TMPSED
		done
		;;
	*)
		echo "" >> $TMPSED
		;;
	esac
	for name in `
	egrep "#define[ 	][ 	]*[A-Z]" $REF/$CONFIGH \
		| sed	-e 's/^#define[ 	][ 	]*//' \
			-e 's/[ 	].*//' \
		| egrep -v "^GCC_" \
		| egrep -v "^${PACKAGE}_" \
		| sort -u \
		| egrep -v "^${PKGNAME}_"`
	do
		echo "s/\\<$name\\>/${PKGNAME}_$name/g" >>$TMPSED
	done
else
	PRG=""
	while test $# != 3
	do
		PRG="$PRG $1"; shift
	done

	DST=$1
	REF=$2
	SRC=$3

	SHOW=`basename $SRC`
	TMPSRC=${TMPDIR-/tmp}/${SHOW}$$

	echo "	... $SHOW"
	test -f $REF/$SRC && SRC="$REF/$SRC"

	rm -f $TMPSRC
	sed -f $TMPSED $SRC > $TMPSRC
	NAME=`basename $SRC`

	# Just in case someone gzip'd manpages, remove the conflicting copy.
	test -f $DST/$NAME.gz && rm -f $DST/$NAME.gz

	eval $PRG $TMPSRC $DST/$NAME
	rm -f $TMPSRC
fi
