#!/bin/sh

wd=`pwd`
if [ -f ../sh_util/Makefile.in ] ; then
	cd ..
fi
for f in ncftp libncftp sh_util vis sio Strn ; do
	if [ ! -f "$f" ] && [ ! -d "$f" ] ; then
		echo "Missing directory $f ?" 1>&2
		exit 1
	fi
done

TMPDIR=/tmp
TAR=""
TARFLAGS=""

if [ "$#" -lt 2 ] ; then
	TARDIR="ncftp"
	STGZFILE="$TARDIR.tar.gz"
else
	TARDIR="$1"
	STGZFILE="$2"
	if [ "$#" -eq 4 ] ; then
		# I.e., called from Makefile
		TAR="$3"
		TARFLAGS="$4"
	fi
fi

if [ -x /usr/bin/bzip2 ] ; then
	BZIP="/usr/bin/bzip2"
elif [ -x /usr/local/bin/bzip2 ] ; then
	BZIP="/usr/local/bin/bzip2"
else
	BZIP=":"
fi
SBGZFILE=`echo "$STGZFILE" | sed 's/\.tar\.gz/.tar.bz2/g'`
ZIPFILE=`echo "$STGZFILE" | sed 's/\.tar\.gz/.zip/g'`
rm -rf $TMPDIR/TAR
mkdir -p -m755 $TMPDIR/TAR/$TARDIR 2>/dev/null

chmod 755 configure sh/*

find . -depth -follow -type f | sed '
/\/\./d
/\/samples/d
/libncftp\/configure$/d
/sio\/configure$/d
/Strn\/configure$/d
/\.o$/d
/\.so$/d
/\.a$/d
/\.lib$/d
/\.ncb$/d
/\.pdb$/d
/\.idb$/d
/\.pch$/d
/\.gch$/d
/\.cpch$/d
/SunWS_cache/d
/\.ilk$/d
/\.res$/d
/\.aps$/d
/\.opt$/d
/\.plg$/d
/\.obj$/d
/\.exe$/d
/\.zip$/d
/\.gz$/d
/\.bz2$/d
/\.tgz$/d
/\.tar$/d
/\.swp$/d
/\.orig$/d
/\.rej$/d
/\/\._/d
/\/Makefile\.bin$/d
/\.bin$/d
/\/bin/d
/\/core$/d
/\/ccdv$/d
/\/[Rr]elease$/d
/\/[Dd]ebug$/d
/\/sio\/.*\//d
/shit/d
/\/upload/d
/\/Strn\.version/d
/\/sio\.version/d
/\/config\.h\.in$/p
/\/config\.guess$/p
/\/config\.sub$/p
/\/config\./d
/\/configure\.in/p
/\/configure\./d
/\/Makefile$/d
/\/OLD/d
/\/old/d' | cut -c3- > "$wd/doc/manifest"

if [ -f "$wd/sh/unix2dos.sh" ] ; then
	cp "$wd/doc/manifest" "$wd/doc/manifest.txt" 
	$wd/sh/unix2dos.sh "$wd/doc/manifest.txt"
fi

cpio -Lpdm $TMPDIR/TAR/$TARDIR < "$wd/doc/manifest"
chmod -R a+rX "$TMPDIR/TAR/$TARDIR"

find $TMPDIR/TAR/$TARDIR -type f '(' -name '*.[ch]' -or -name '*.[ch]pp' -or -name '*.in' ')' -exec $wd/sh/dos2unix.sh {} \;

if [ "$TAR" = "" ] || [ "$TARFLAGS" = "" ] ; then
	x=`tar --help 2>&1 | sed -n 's/.*owner=NAME.*/owner=NAME/g;/owner=NAME/p'`
	case "$x" in
		*owner=NAME*)
			TARFLAGS="-c --owner=bin --group=bin -f"
			TAR=tar
			;;
		*)
			TARFLAGS="cf"
			TAR=tar
			x2=`gtar --help 2>&1 | sed -n 's/.*owner=NAME.*/owner=NAME/g;/owner=NAME/p'`
			case "$x2" in
				*owner=NAME*)
					TARFLAGS="-c --owner=bin --group=bin -f"
					TAR=gtar
					;;
			esac
			;;
	esac
fi

( cd $TMPDIR/TAR ; $TAR $TARFLAGS - $TARDIR | gzip -c > $STGZFILE )
cp $TMPDIR/TAR/$STGZFILE .

if [ "$BZIP" != ":" ] ; then
	( cd $TMPDIR/TAR ; $TAR $TARFLAGS - $TARDIR | $BZIP -c > $SBGZFILE )
	cp $TMPDIR/TAR/$SBGZFILE .
fi

( cd $TMPDIR/TAR ; zip -q -r -9 $ZIPFILE $TARDIR )
cp $TMPDIR/TAR/$ZIPFILE .

chmod 644 $STGZFILE $SBGZFILE $ZIPFILE 2>/dev/null
rm -rf $TMPDIR/TAR
touch -r ncftp/version.c $STGZFILE $SBGZFILE $ZIPFILE
ls -l $STGZFILE $SBGZFILE $ZIPFILE 2>/dev/null
exit 0
