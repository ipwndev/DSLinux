#!/bin/sh
#
# A tool to simplify Makefiles that need to put something
# into the ROMFS
#
# Copyright (C) David McCullough, 2002,2003
#
#############################################################################

# Provide a default PATH setting to avoid potential problems...
PATH="/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:$PATH"
NROFF="/usr/bin/nroff"

usage()
{
cat << !EOF >&2
$0: [options] [src] dst
    -v          : output actions performed.
    -e env-var  : only take action if env-var is set to "y".
    -o option   : only take action if option is set to "y".
    -p perms    : chmod style permissions for dst.
	-d          : make dst directory if it doesn't exist
	-S          : don't strip after installing
    -a text     : append text to dst.
	-A pattern  : only append text if pattern doesn't exist in file
    -l link     : dst is a link to 'link'.
    -s sym-link : dst is a sym-link to 'sym-link'.
    -m manpage  : Run page through $NROFF before copying
    if "src" is not provided,  basename is run on dst to determine the
    source in the current directory.

	multiple -e and -o options are ANDed together.  To achieve an OR affect
	use a single -e/-o with 1 or more y/n/"" chars in the condition.

	if src is a directory,  everything in it is copied recursively to dst
	with special files removed (currently CVS and Subversion dirs).
!EOF
	exit 1
}

#############################################################################

setperm()
{
	rc=0
	if [ "$perm" ]
	then
		[ "$v" ] && echo "chmod ${perm} ${ROMFSDIR}${dst}"
		chmod ${perm} ${ROMFSDIR}${dst}
		rc=$?
	fi
	return $rc
}

#############################################################################

file_copy()
{
	rc=0
	if [ -d "${src}" ]
	then
		[ "$v" ] && echo "CopyDir ${src} ${ROMFSDIR}${dst}"
		(
			cd ${src} || return 1
			V=
			[ "$v" ] && V=v
			find . -print | grep -E -v '/CVS|/\.svn' | cpio -p${V}dumL ${ROMFSDIR}${dst}
			rc=$?
		)
	else
		if [ -d ${dst} ]; then
			dstfile=${ROMFSDIR}${dst}/`basename ${src}`
		else
			dstfile=${ROMFSDIR}${dst}
		fi
		rm -f ${dstfile}
		[ "$v" ] && echo "cp ${src} ${dstfile}"
		cp ${src} ${dstfile} && setperm ${dstfile}
		rc=$?
		if [ $rc -eq 0 -a -n "$strip" ]; then
			${STRIPTOOL} ${dstfile} 2>/dev/null
			${STRIPTOOL} -R .comment -R .note ${dstfile} 2>/dev/null
		fi
	fi
	return $rc
}

#############################################################################

file_append()
{
	touch ${ROMFSDIR}${dst} || return 1
	if [ -z "${pattern}" ] && grep -F "${src}" ${ROMFSDIR}${dst} > /dev/null
	then
		[ "$v" ] && echo "File entry already installed."
	elif [ "${pattern}" ] && egrep "${pattern}" ${ROMFSDIR}${dst} > /dev/null
	then
		[ "$v" ] && echo "File pattern already installed."
	else
		[ "$v" ] && echo "Installing entry into ${ROMFSDIR}${dst}."
		echo "${src}" >> ${ROMFSDIR}${dst} || return 1
	fi
	setperm ${ROMFSDIR}${dst}
	return $?
}

#############################################################################

hard_link()
{
	rm -f ${ROMFSDIR}${dst}
	[ "$v" ] && echo "ln ${src} ${ROMFSDIR}${dst}"
	ln ${ROMFSDIR}${src} ${ROMFSDIR}${dst}
	return $?
}

#############################################################################

sym_link()
{
	rm -f ${ROMFSDIR}${dst}
	[ "$v" ] && echo "ln -s ${src} ${ROMFSDIR}${dst}"
	ln -sf ${src} ${ROMFSDIR}${dst}
	return $?
}
man_page()
{
	#Man page installation
	if [ ${CONFIG_USER_MANPAGES} = "y" ]
	then
		$NROFF -man ${src} |fmt -w 64 > ${ROMFSDIR}${dst}
	fi
	return $?
}
#############################################################################
#
# main program entry point
#

if [ -z "$ROMFSDIR" ]
then
	echo "ROMFSDIR is not set" >&2
	usage
	exit 1
fi

v=
option=y
pattern=
perm=
func=file_copy
mdir=
src=
dst=
strip=1

while getopts 'dSve:o:A:p:a:l:s:m' opt "$@"
do
	case "$opt" in
	v) v="1";                           ;;
	d) mdir="1";                        ;;
	S) strip=;							;;
	o) option="$OPTARG";                ;;
	e) eval option=\"\$$OPTARG\";       ;;
	p) perm="$OPTARG";                  ;;
	a) src="$OPTARG"; func=file_append; ;;
	A) pattern="$OPTARG";               ;;
	l) src="$OPTARG"; func=hard_link;   ;;
	s) src="$OPTARG"; func=sym_link;    ;;
	m) func=man_page;   ;;

	*)  break ;;
	esac
#
#	process option here to get an ANDing effect
#
	case "$option" in
	*[mMyY]*) # this gives OR effect, ie., nYn
		;;
	*)
		[ "$v" ] && echo "Condition not satisfied."
		exit 0
		;;
	esac
done

shift `expr $OPTIND - 1`

case $# in
1)
	dst="$1"
	if [ -z "$src" ]
	then
		src="`basename $dst`"
	fi
	;;
2)
	if [ ! -z "$src" ]
	then
		echo "Source file already provided" >&2
		exit 1
	fi
	src="$1"
	dst="$2"
	;;
*)
	usage
	;;
esac

if [ "$mdir" -a ! -d "`dirname ${ROMFSDIR}${dst}`/." ]
then
	mkdir -p "`dirname ${ROMFSDIR}${dst}`/." || exit 1
fi

$func || exit 1

exit 0

#############################################################################
