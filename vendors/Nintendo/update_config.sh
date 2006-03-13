#!/bin/sh

for i in [D-Z]* ; do cp common/config.vendor $i ; if [ -e $i/vendorpatch ] ; then cd $i ; patch -p1 < vendorpatch ; cd - ; fi ; done
