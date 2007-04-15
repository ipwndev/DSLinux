srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

(
cd $srcdir
cat mc-aclocal.m4 gettext.m4 lcmessage.m4 > aclocal.m4
aclocal
autoheader
autoconf
)

#$srcdir/configure $*
$srcdir/mc.configure
