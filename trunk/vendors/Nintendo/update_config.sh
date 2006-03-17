#!/bin/sh

# Usage:
# Update common/config.vendor with a GBAMP config.vendor then run this script
# cvs diff
# cvs commit
#
# To update the patches used by this script:
#
# Change config.vendor as required using make menuconfig (customise vendor, update)
# cd vendors/Nintendo
# diff -u common/config.vendor FOO/config.vendor > FOO/vendorpatch
# cvs diff
# cvs commit

for i in [D-Z]* 
do
    cp common/config.vendor $i 
    cp common/config.uClibc $i 
    if [ -e $i/vendorpatch ] 
    then
        cd $i 
        patch -p1 < vendorpatch 
        cd - 
    fi 
done

