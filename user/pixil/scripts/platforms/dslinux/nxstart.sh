#!/bin/sh

# Set some required environment variables

export LD_LIBRARY_PATH=/usr/pixil/lib:/usr/lib
export PARDB=/usr/pixil/share/par/pixil.db
export CONSOLE=/dev/tty0

# Start the IPC server
/usr/pixil/sbin/clserver &

# Start the Nano-X server
/usr/bin/nano-X -p &

# Make sure that the server is calibrated
/usr/bin/nxcal -d /etc/pixil.calibrate

# Start the window manager
/usr/pixil/bin/pixilwm 

# if we make it this far, then the window manager was stopped suddenly
# kill the outstanding server to save our sanity

killall -q /usr/bin/nano-X
killall -TERM -q /usr/pxil/sbin/clserver


