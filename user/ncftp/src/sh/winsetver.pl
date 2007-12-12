#!/usr/bin/perl -w

use strict;

my @rc_files = (
	"ncftp/rc.rc",
	"sh_util/rc.rc",
	"win/bmed/bmed.rc"
);

my $setup_file = "win/setup/ncftp.wse";


sub SetVersionInRCFile
{
	my ($f, $fver, $fbld, $pver, $pbld) = @_;
	my ($rcdata) = "";
	my ($line);
	my ($verstr);
	my ($curyear);

	return (0) unless ($fver =~ /^\d+\.\d+\.\d+$/);
	$fbld = "increment" unless (defined($fbld));
	$pver = $fver unless (defined($pver));
	$pbld = $fbld unless (defined($pbld));

	if (open(F, "< $f")) {
		while (defined($line = <F>)) {
			$rcdata .= $line;
		}
		close(F);

		#
		# Set the FILE version.
		#
		if ($fbld eq "increment") {
			$fbld = 0;
			if ($rcdata =~ /\s*FILEVERSION\s+\d+,\d+,\d+,(\d+)\s*/) {
				$fbld = $1;
			}
			$fbld++;
		}

		$verstr = "$fver,$fbld";
		$verstr =~ s/\./,/g;

		$rcdata =~ s/FILEVERSION\s+\d+,\d+,\d+,\d+/FILEVERSION $verstr/;
		$rcdata =~ s/FileVersion.*\\0/FileVersion\", \"$fver\\0/m;


		#
		# Set the PRODUCT version.
		#
		if ($pbld eq "increment") {
			$pbld = 0;
			if ($rcdata =~ /\s*PRODUCTVERSION\s+\d+,\d+,\d+,(\d+)\s*/) {
				$pbld = $1;
			}
			$pbld++;
		}

		$verstr = "$pver,$pbld";
		$verstr =~ s/\./,/g;

		$rcdata =~ s/PRODUCTVERSION\s+\d+,\d+,\d+,\d+/PRODUCTVERSION $verstr/;
		$rcdata =~ s/ProductVersion.*\\0/ProductVersion\", \"$pver\\0/m;


		#
		# Make sure the COPYRIGHT is up-to-date.
		#
		$curyear = (localtime())[5] + 1900;
		$rcdata =~ s/Copyright\s(\S|\(C\))\s*\d{4}/Copyright © $curyear/i;

		if (open(F, "> $f")) {
			print F $rcdata;
			close(F);
			return (1);
		}
	}

	return (0);
}	# SetVersionInRCFile



sub SetVersionInSetupFile
{
	my ($f, $fver) = @_;
	my ($fdata) = "";
	my ($line);

	return (0) unless ($fver =~ /^\d+\.\d+\.\d+$/);

	if (open(F, "< $f")) {
		while (defined($line = <F>)) {
			$fdata .= $line;
		}
		close(F);

		$fdata =~ s/^(\s*(Title|EXE\sFilename)=.*?)([\d\.]+)(.*)$/$1$fver$4/mg;

		# Variable=APPTITLE
		$fdata =~ s/^(\s*Value=NcFTP.*?)([\d\.]+)(.*)$/$1$fver$3/mg;

		if (open(F, "> $f")) {
			print F $fdata;
			close(F);
			return (1);
		}
	}

	return (0);
}	# SetVersionInSetupFile



sub Main
{
	my ($f);
	my ($ver);

	if ((! defined($ARGV[0])) || ($ARGV[0] !~ /^\d+\.\d+\.\d+$/)) {
		print STDERR "Usage:    $0 <version>\n";
		print STDERR "Example:  $0 3.1.2\n";
		exit(2);
	}
	$ver = $ARGV[0];

	foreach $f (@rc_files) {
		if (! -f $f) {
			die "Missing \"$f\".  Chdir to the NcFTP source first.\n";
		}
		SetVersionInRCFile($f, $ver, "increment")
			or die "Could not change $f to version $ver: $!\n";
		print "Changed: $f\n";
	}

	$f = $setup_file;
	SetVersionInSetupFile($f, $ver)
		or die "Could not change $f to version $ver: $!\n";
	print "Changed: $f\n";
	exit(0);
}	# Main



Main();
