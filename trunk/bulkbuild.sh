#!/bin/sh

# Run this script to do a bulk build of DSLinux.
# This will produce a set of builds for distribution.
#
# Default configurations will be used.
# WARNING: Local configuration changes will be nuked!

set -e

export KCONFIG_NOTIMESTAMP=TRUE

if [ "$1" = "-u" ]
then
	update_config="CONFIG_DEFAULTS_VENDOR_UPDATE=y"
else
	update_config="# CONFIG_DEFAULTS_VENDOR_UPDATE is not set"
fi

if [ "$1" = "-r" ]
then
	release_config="CONFIG_DEFAULTS_RELEASE_BUILD=y"
else
	release_config="# CONFIG_DEFAULTS_RELEASE_BUILD is not set"
fi

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
$update_config
CONFIG_VENDOR=Nintendo
CONFIG_PRODUCT=$1
CONFIG_LINUXDIR=linux-2.6.x
CONFIG_LIBCDIR=uClibc
CONFIG_LANGUAGE=
$release_config
EOF
}

OUTDIR=${OUTDIR:-"./bulkbuild"}

for build in DSGBA DSMEM GBAMP GBAMP_EXT2 NDS DLDI
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
		NDS)
			distfile=dslinux.nds
		;;	
		DLDI)
			distfile=dslinux-dldi.tgz
		;;
		*)
			echo
			echo "ERROR: distfile is undefined for $build"
			echo
			exit 1
		;;
	esac

	[ -d "$OUTDIR" ] || mkdir -p "$OUTDIR"

    if [ "$1" = "-u" ]
    then
        mktopconfig $build && \
        yes '' | make config
    else
        mktopconfig $build && \
        yes '' | make config && \
        $fakeroot_cmd make && \
        cp -v -f ./images/$distfile "$OUTDIR" && \
        (cd "$OUTDIR" && md5sum $distfile | tee $distfile.md5)
    fi
done
