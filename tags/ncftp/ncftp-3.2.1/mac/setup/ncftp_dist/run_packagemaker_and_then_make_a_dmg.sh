#!/bin/sh

if [ $# -ne 2 ] ; then
	echo "Usage: $0 version.num.ber output-file.dmg" 1>&2
	exit 2
fi

PACKAGE_BUILD_ROOT="$HOME/Package_Temp"
v="$1"
DMG_PATHNAME="$2"

case "$v" in
	[0-9].[0-9]*)
		;;
	*)
		echo "Usage: $0 version.num.ber" 1>&2
		exit 2
		;;
esac

if [ ! -d "mac/setup/ncftp_dist" ] || [ "$v" = "" ] ; then
	if [ -d "$HOME/src" ] ; then
		cd $HOME/src
	fi

	cd "ncftp-$v"
	if [ $? -ne 0 ] ; then
		echo "Could not find the NcFTP source directory;" 1>&2
		echo "Change directory to the source directory, so that the relative" 1>&2
		echo "path ./mac/setup/ncftp_dist is valid." 1>&2
		exit 1
	fi
fi

############ create install tree

sudo_has_run=no
if [ -d "${PACKAGE_BUILD_ROOT}" ] ; then
	/bin/rm -rf "${PACKAGE_BUILD_ROOT}"
	if [ -d "${PACKAGE_BUILD_ROOT}" ] ; then
		echo -n "* Sudo needs to be run.  Ready? (^C to exit)"
		read ans
		sudo /bin/rm -rf "${PACKAGE_BUILD_ROOT}"
		if [ -d "${PACKAGE_BUILD_ROOT}" ] ; then
			echo "Could not remove old directory ${PACKAGE_BUILD_ROOT}" 1>&2
			exit 1
		fi
		sudo_has_run=yes
	fi
fi

umask 022
ncftp_dist="${PACKAGE_BUILD_ROOT}/ncftp_dist"
mkdir -p "$ncftp_dist"
if [ ! -d "$ncftp_dist" ] ; then
	echo "Could not mkdir $ncftp_dist" 1>&2
	exit 1
fi
ncftp_proot="$ncftp_dist/Package_Root"

if [ "$sudo_has_run" != yes ] ; then
	echo -n "* Sudo needs to be run.  Ready? (^C to exit)"
	read ans
fi
sudo /bin/rm -rf "$ncftp_proot"
sudo_has_run=yes

echo "* Creating directory tree..."
mkdir -m 755 "$ncftp_proot" "$ncftp_proot/usr" "$ncftp_proot/usr/bin" "$ncftp_proot/usr/share" "$ncftp_proot/usr/share/man" "$ncftp_proot/usr/share/man/man1"
if [ ! -d "$ncftp_proot/usr/bin" ] ; then
	echo "Could not mkdir $ncftp_proot/usr/bin and others" 1>&2
	exit 1
fi

tmpdir="${PACKAGE_BUILD_ROOT}/tmp"
/bin/rm -rf "$tmpdir"
mkdir "$tmpdir" "${tmpdir}2"
if [ ! -d "$tmpdir" ] ; then
	echo "Could not mkdir $tmpdir" 1>&2
	exit 1
fi

resources_dir="`pwd`/mac/setup/ncftp_dist/Resources"
if [ ! -d "$resources_dir" ] ; then
	echo "Could not find $resources_dir" 1>&2
	exit 1
fi

majorv=`echo "$v" | cut -d. -f1`
minorv=`echo "$v" | cut -d. -f2- | sed 's/\.//g;'`
info_plist="`pwd`/mac/setup/ncftp_dist/Info.plist"
if [ -f "$info_plist.in" ] ; then
	sed 's/@VERSION@/'"$v"'/g;s/@MINOR_VERSION@/'"$minorv"'/g;s/@MAJOR_VERSION@/'"$majorv"'/g;' < "$info_plist.in" > "$info_plist"
	/bin/rm -f "$info_plist.in"
fi
if [ ! -f "$info_plist" ] ; then
	echo "Could not find $info_plist" 1>&2
	exit 1
fi

description_plist="`pwd`/mac/setup/ncftp_dist/Description.plist"
if [ -f "$description_plist.in" ] ; then
	sed 's/@VERSION@/'"$v"'/g;s/@MINOR_VERSION@/'"$minorv"'/g;s/@MAJOR_VERSION@/'"$majorv"'/g;' < "$description_plist.in" > "$description_plist"
	/bin/rm -f "$description_plist.in"
fi
if [ ! -f "$description_plist" ] ; then
	echo "Could not find $description_plist" 1>&2
	exit 1
fi

echo "* Copying files to directory tree..."
cp -pr "$resources_dir" "$ncftp_dist/Resources"
resources_dir="$ncftp_dist/Resources"
cp -p bin/* "$ncftp_proot/usr/bin"
for f in "$ncftp_proot/usr/bin"/*
do
	touch -r ncftp/version.c "$f"
done

cp -p doc/man/* "$ncftp_proot/usr/share/man/man1"
echo "* Running to set permissions and ownerships for directory tree..."
sudo chown -R root:wheel "$ncftp_dist"
sudo chmod -R a+rX "$ncftp_dist"
if [ $? -ne 0 ] ; then
	exit 1
fi
echo "* Done creating ${ncftp_dist}:"
( cd "$ncftp_dist" ; find . -ls )
echo

PACKAGE_PATHNAME="${PACKAGE_BUILD_ROOT}/NcFTP ${v}.pkg"
sudo /bin/rm -rf "$PACKAGE_PATHNAME" "$DMG_PATHNAME"

echo "* Running PackageMaker..."
sudo /Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker -build \
	-p "$PACKAGE_PATHNAME" \
	-f "$ncftp_proot" \
	-b "${tmpdir}2" \
	-ds \
	-v \
	-r "$resources_dir" \
	-i "$info_plist" \
	-d "$description_plist"

es="$?"
if [ "$es" -ne 0 ] ; then
	echo "PackageMaker failed with exit status $es?"
	exit 1
fi
if [ ! -e "$PACKAGE_PATHNAME" ] ; then
	echo "Missing finished package, $PACKAGE_PATHNAME"
	exit 1
fi

echo "* Running pkg_to_dmg.sh ..."
mac/setup/ncftp_dist/pkg_to_dmg.sh "$PACKAGE_PATHNAME" "$DMG_PATHNAME"
if [ $? -ne 0 ] || [ ! -f "$DMG_PATHNAME" ] ; then
	echo "Failed to build .dmg (\"$DMG_PATHNAME\") file from .pkg file (\"$PACKAGE_PATHNAME\")"
	exit 1
fi

sudo /bin/rm -rf "${PACKAGE_BUILD_ROOT}"
exit 0
