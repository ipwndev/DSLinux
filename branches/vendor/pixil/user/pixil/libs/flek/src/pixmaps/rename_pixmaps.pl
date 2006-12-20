#!/usr/bin/perl

# This script renames pixmaps internally after conversion from Image Magick,
# to the Gimp's naming scheme.

$file_name = $ARGV[0];

open (IN, "< $file_name");
while (<IN>) { $file .= $_; }
close (IN);

$n = $file_name;
$n =~ s/\./\_/g;
$file =~ s/magick/$n/g;

open (OUT, "> $file_name");
print OUT $file;
close (OUT);

