#!/bin/sh
if [ -z "$1" ]
then
	echo "Usage: `basename $0` <file.gdb>"
	exit 1
fi
arm-linux-elf-nm $1 | grep -v '\(compiled\)\|\(\.o$$\)\|\( a \)' | sort
