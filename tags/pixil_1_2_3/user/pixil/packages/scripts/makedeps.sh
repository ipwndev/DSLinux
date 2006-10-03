#!/bin/sh

FILE=$1

cat $FILE | while read pkg dir; do \
	echo ".PHONY: $dir"
	echo "DIRS-\$($pkg) += $dir";
	echo "";
done

