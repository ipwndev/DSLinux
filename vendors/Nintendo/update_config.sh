#!/bin/sh

# Usage:
# Update common/config.vendor with a GBAMP config.vendor then run this script

for i in [D-Z]* 
do
    cp common/config.vendor $i 
    if [ -e $i/vendorpatch ] 
    then
        cd $i 
        patch -p1 < vendorpatch 
        cd - 
    fi 
done

