#!/bin/sh

set -x
aclocal-1.9 -I m4
autoheader
automake-1.9 --foreign --add-missing --copy
autoconf
rm -rf autom4te.cache
