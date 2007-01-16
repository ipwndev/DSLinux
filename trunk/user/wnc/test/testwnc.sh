#!/bin/bash
if [ $(id -u) = 0 ]
then
	echo "Don't run this script as root!"
	exit 1
fi
this="$PWD/$(dirname "$0")"
export PATH="$PATH:$this/bin"

sed -e "s,exec /etc/rc.d/network,exec ${1:-msh} $this/network," \
-e "s,VARDIR=.*,VARDIR=/tmp/wncsh," wnc.sh > wnc.sh.tmp

${1:-msh} wnc.sh.tmp
rm -f wnc.sh.tmp
