#!/bin/sh

# Set some required environment variables
export PARDB=/usr/pixil/share/par/pixil.db
export CONSOLE=/dev/tty1
export FRAMEBUFFER=/dev/fb1

fbset -depth 16 -n -fb $FRAMEBUFFER
ifconfig lo up
gpm -k

# Start the IPC server
/usr/pixil/sbin/clserver &

# Start the Nano-X server
# (use PATH to search for the executable)
nano-X -p &

# Make sure that the server is calibrated
# /usr/bin/nxcal -d /etc/pixil.calibrate

# Start the window manager
/usr/pixil/bin/pixilwm 

# if we make it this far, then the window manager was stopped suddenly
# kill the outstanding server to save our sanity

killall -q /usr/bin/nano-X
killall -TERM -q /usr/pxil/sbin/clserver

