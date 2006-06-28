#!/bin/sh

# Run this script to do a bulk build of DSLinux.
# This will produce a set of builds for distribution.
#
# Default configurations will be used.
# WARNING: Local configuration changes will be nuked!

set -e

mktopconfig() {
	cat > .config <<EOF
CONFIG_DEFAULTS_NINTENDO=y
CONFIG_DEFAULTS_NINTENDO_$1=y
CONFIG_DEFAULTS_KERNEL_2_6=y
# CONFIG_DEFAULTS_LIBC_NONE is not set
CONFIG_DEFAULTS_LIBC_UCLIBC=y
CONFIG_DEFAULTS_OVERRIDE=y
# CONFIG_DEFAULTS_KERNEL is not set
# CONFIG_DEFAULTS_VENDOR is not set
# CONFIG_DEFAULTS_VENDOR_UPDATE is not set
CONFIG_VENDOR=Nintendo
CONFIG_PRODUCT=$1
CONFIG_LINUXDIR=linux-2.6.x
CONFIG_LIBCDIR=uClibc
CONFIG_LANGUAGE=
EOF
}

OUTDIR=${OUTDIR:-"./bulkbuild"}

for build in DSGBA DSMEM GBAMP GBAMP_EXT2 M3CF NDS SUPERCARDCF SUPERCARDSD
do
	if [ "$build" = "GBAMP_EXT2" ]
	then
		if which fakeroot > /dev/null
		then
			fakeroot_cmd="`which fakeroot`"
		else
			echo
			echo
			echo "Not building $build - fakeroot not found"
			echo
			echo
			sleep 3
			continue
		fi
	else
		fakeroot_cmd=""
	fi
		
	case $build in
		DSGBA)
			distfile=dslinux.ds.gba
		;;
		DSMEM)
			distfile=dslinux-dsmem.tgz
		;;
		GBAMP)
			distfile=dslinux-gbamp.tgz
		;;
		GBAMP_EXT2)
			distfile=dslinux-gbamp-ext2.tgz
		;;
		M3CF)
			distfile=dslinux-m3cf.tgz
		;;
		NDS)
			distfile=dslinux.nds
		;;	
		SUPERCARDCF)
			distfile=dslinux-supercardcf.tgz
		;;

		SUPERCARDSD)
			distfile=dslinux-supercardsd.tgz
		;;
		*)
			echo
			echo "ERROR: distfile is undefined for $build"
			echo
			exit 1
		;;
	esac

	[ -d "$OUTDIR" ] || mkdir -p "$OUTDIR"

	mktopconfig $build && \
	yes '' | make config && \
	$fakeroot_cmd make && \
	cp -v -f ./images/$distfile "$OUTDIR" && \
	(cd "$OUTDIR" && md5sum $distfile | tee $distfile.md5)
done
