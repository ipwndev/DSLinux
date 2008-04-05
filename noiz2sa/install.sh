#!/bin/sh

if [ -e noiz2sa ];
then
	printf "Copying files\n"
else
	printf "noiz2sa not found. Did you remember to download it?\n"
	printf "http://sourceforge.net/project/showfiles.php?group_id=112833\n"
	printf "Place it in the same directory as this script and re-run\n"
	exit 1
fi

if mkdir /usr/share/games/noiz2sa; then
	printf "Share files directory created\n"
else
	printf "This script must be run as root\n"
	exit 1
fi

if cp -pr noiz2sa_share/* /usr/share/games/noiz2sa; then
	printf "Share files copied\n"
else
	printf "This script must be run as root\n"
	exit 1
fi

if chown games. /usr/share/games/noiz2sa; then
	printf "/usr/share/games/noiz2sa chowned\n"
else
	printf "This script must be run as root\n"
	exit 1
fi

if chmod u=rwx,g=rx,o=rx /usr/share/games/noiz2sa; then
	printf "/usr/share/games/noiz2sa chmoded\n"
else
	printf "This script must be run as root\n"
	exit 1
fi

if cp -p noiz2sa /usr/bin; then
	printf "/usr/bin/noiz2sa copied\n"
else
	printf "This script must be run as root\n"
	exit 1
fi

if chown root. /usr/bin/noiz2sa; then
	printf "/usr/bin/noiz2sa chowned\n"
else
	printf "This script must be run as root\n"
	exit 1
fi

if chmod u=rwx,g=rx,o=rx /usr/bin/noiz2sa; then
	printf "/usr/bin/noiz2sa chmoded\n"
else
	printf "This script must be run as root\n"
	exit 1
fi

printf "Done. Run with noiz2sa\n"
