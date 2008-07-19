#!/bin/sh

if [ $# -eq 0 ] ; then
	echo "Usage: $0 path-to-pkg-file [path-to-dmg-output-file]" 1>&2
	exit 1
fi

shtimer=''
for f in /usr/bin/shtimer /usr/local/bin/shtimer
do
	if [ -x "$f" ] ; then shtimer="$f" ; break ; fi
done
if [ "$shtimer" != "" ] ; then
	t0=`"$shtimer" on`
fi

tmpfile="${TMPDIR-/tmp}/pkg_to_dmg.$$"
rm -f "$tmpfile"
cat /dev/null > "$tmpfile"
if [ ! -f "$tmpfile" ] ; then
	echo "Could not create $tmpfile." 1>&2
	exit 1
fi

pkg="$1"
pkgname=`echo "$pkg" | sed 's-^.*/--;'`
volname=`echo "$pkg" | sed 's/\.pkg$//;s-^.*/--;'`
if [ "$#" -gt 1 ] ; then
	dmg="$2"
else
	dmg=`echo "$pkg" | sed 's/\.pkg$/.dmg/;'`
fi

rm -f "$dmg"
if [ -f "$dmg" ] ; then
	echo "Could not remove existing $dmg." 1>&2
	exit 1
fi

pkgsize=`du -sk "$pkg" | cut -f1`
echo "Package size: $pkgsize"
dmgsize=`expr 1024 - '(' $pkgsize '%' 1024 ')' + $pkgsize`
if [ "$dmgsize" -lt 5120 ] ; then
	# Minimum HFS size is 4 MB, but we compress it later anyway.
	dmgsize=5120
fi
echo "Dmg size: $dmgsize"

echo ''
echo "Creating uncompressed disk image..."
echo hdiutil create -size "${dmgsize}k" -fs HFS+ -volname "$volname" "$dmg"
hdiutil create -size "${dmgsize}k" -fs HFS+ -volname "$volname" "$dmg"
if [ ! -f "$dmg" ] ; then
	echo "Could not create $dmg." 1>&2
	exit 1
fi

echo ''
echo "Mounting uncompressed disk image..."
echo hdid "$dmg"
hdid "$dmg" > "$tmpfile" 2>&1

vol=`sed -n '/\/Volumes/{s-\ *$--;s-^.*/Volumes/-/Volumes/-;p;q;};' "$tmpfile"`
disk=`sed -n '/Apple_HFS/{s-\ .*$--;s-/dev/--;p;q;};' "$tmpfile"`
rm -f "$tmpfile"

if [ ! -d "$vol" ] ; then
	echo "Failed to mount volume \"$vol\"."
	exit 1
fi

echo ''
echo "Copying files..."
echo ditto -V -rsrcFork "$pkg" "$vol/$pkgname"
ditto -V -rsrcFork "$pkg" "$vol/$pkgname"
if [ $? -ne 0 ] ; then
	echo "Ditto failed?" 1>&2
	exit 1
fi

echo ''
echo "Unmounting uncompressed disk image..."
echo hdiutil detach "$disk"
hdiutil detach "$disk"

mv "$dmg" "$dmg.0"
if [ $? -ne 0 ] ; then
	echo "Rename $dmg to $dmg.0 failed." 1>&2
	exit 1
fi

echo ''
echo "Compressing disk image..."
hdiutil convert -format UDZO -o "$dmg" "$dmg.0"
if [ -f "$dmg" ] ; then
	/bin/rm "$dmg.0"
fi

echo ''
/bin/ls -l "$dmg"
if [ "$shtimer" != "" ] ; then
	"$shtimer" off "$t0" 'Disk image created.  Elapsed Time = '
else
	echo "Disk image created."
fi
exit 0
