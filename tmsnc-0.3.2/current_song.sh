#!/bin/sh

if [ "$(ps axc|awk '{if ($5=="iTunes") print $1}')" == "" ]; then
	exit 1;
fi

CSONG=`osascript -e 'tell application "iTunes"
	artist of current track & " - " & name of current track
	end tell' 2> /dev/null`;

echo $CSONG
