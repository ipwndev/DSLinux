#!/bin/bash
. $HOME/.pixilrc
killall PixilDT
killall -9 expect
killall -9 minicom
killall -9 rz
killall -9 sz

cd $PIXIL_DATA
expect -f $PIXIL_BIN/push.expect
cd $PIXIL_DATA
