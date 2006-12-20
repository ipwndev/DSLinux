#!/bin/sh

export LD_LIBRARY_PATH=/usr/pixil/lib:/usr/lib
export PARDB=/usr/pixil/share/par/pixil.db
export CONSOLE=/dev/tty1

/usr/pixil/sbin/clserver &
/usr/bin/nano-X -R -p &
/usr/bin/nxcal -d /etc/pixil.calibrate
/usr/pixil/bin/pixilwm 

