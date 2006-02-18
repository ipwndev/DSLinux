#!../miniperl
# Habit . . .
#
# Extract info from config.h, and add extra data here, to generate config.sh
# Edit the static information after __END__ to reflect your site and options
# that went into your perl binary.  In addition, values which change from run
# to run may be supplied on the command line as key=val pairs.
#
# Last Modified: 28-Jun-1996  Luther Huffman  lutherh@stratcom.com
#

#==== Locations of installed Perl components
$p9pvers="_P9P_VERSION";
$prefix='';
$p9p_objtype=$ENV{'objtype'};
$builddir="/sys/src/cmd/perl/$p9pvers";
$installbin="/$p9p_objtype/bin";
$installman1dir="/sys/man/1";
$installman3dir="/sys/man/2";
$installprivlib="/sys/lib/perl";
$installarchlib = "/$p9p_objtype/lib/perl/$p9pvers";
$archname="plan9_$p9p_objtype";
$installsitelib="$installprivlib/site_perl";
$installsitearch="$installarchlib/site_perl";
$installscript="/bin";

unshift(@INC,'lib');  # In case someone didn't define Perl_Root
                      # before the build

if ($ARGV[0] eq '-f') {
  open(ARGS,$ARGV[1]) or die "Can't read data from $ARGV[1]: $!\n";
  @ARGV = ();
  while (<ARGS>) {
    push(@ARGV,split(/\|/,$_));
  }
  close ARGS;
}

if (-f "config.h") { $infile = "config.h"; $outdir = "../"; }
elsif (-f "plan9/config.h") { $infile = "plan9/config.h";  $outdir = "./"; }

if ($infile) { print "Generating config.sh from $infile . . .\n"; }
else { die <<EndOfGasp;
Can't find config.h to read!
	Please run this script from the perl source directory or
	the plan9 subdirectory in the distribution.
EndOfGasp
}
$outdir = '';
open(IN,"$infile") || die "Can't open $infile: $!\n";
open(OUT,">${outdir}config.sh") || die "Can't open ${outdir}config.sh: $!\n";

$time = localtime;
$cf_by = $ENV{'user'};
($vers = $]) =~ tr/./_/;

# Plan 9 doesn't actually use version numbering. Following the original Unix
# precedent of assigning a Unix edition number based on the edition number
# of the manuals, I am referring to this as Plan 9, 1st edition.
$osvers = '1';

print OUT <<EndOfIntro;
# This file generated by genconfig.pl on a Plan 9 system.
# Input obtained from:
#     $infile
#     $0
# Time: $time

package='perl5'
CONFIG='true'
cf_time='$time'
cf_by='$cf_by'
ccdlflags=''
cccdlflags=''
libpth='$installprivlib'
ld='pcc'
lddlflags=''
ranlib=''
ar='ar'
nroff='/bin/nroff'
eunicefix=':'
hint='none'
hintfile=''
intsize='4'
longsize='4'
shortsize='2'
shrplib='define'
usemymalloc='n'
usevfork='true'
useposix='true'
spitshell='cat'
dlsrc='dl_none.c'
binexp='$installbin'
man1ext=''
man3ext=''
arch='$archname'
archname='$archname'
osname='plan9'
extensions='IO Socket Opcode Fcntl POSIX DynaLoader FileHandle'
osvers='$osvers'
sig_maxsig='19'
sig_name='ZERO HUP INT QUIT ILL ABRT FPE KILL SEGV PIPE ALRM TERM USR1 USR2 CHLD CONT STOP TSTP TTIN TTOU'
sig_num='0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19'
sig_numsig='20'
prefix='$prefix'
builddir='$builddir'
installbin='$installbin'
installman1dir='$installman1dir'
installman3dir='$installman3dir'
installprivlib='$installprivlib'
installarchlib='$installarchlib'
installsitelib='$installsitelib'
installsitearch='$installsitearch'
installscript='$installscript'
scriptdir='$installscript'
scriptdirexp='$installscript'
EndOfIntro

# Plan 9 compiler stuff
print OUT "cc='pcc'\n";
print OUT "d_attribut='undef'\n";
print OUT "d_socket='define'\n";
print OUT "d_sockpair='define'\n";
print OUT "d_sigsetjmp='define'\n";
print OUT "sigjmp_buf='sigjmp_buf'\n";
print OUT "sigsetjmp='sigsetjmp(buf,save_mask)'\n";
print OUT "siglongjmp='siglongjmp(buf,retval) '\n";
print OUT "exe_ext=''\n";
if ($p9p_objtype eq '386') {
	$objext = '.8';
	$alignbytes = '4';
	$cstflags = 2;
}
elsif ($p9p_objtype eq '68020') {
	$objext = '.2';
	$alignbytes = '2';
	$cstflags = 0;
}
elsif ($p9p_objtype eq 'mips') {
	$objext = '.v';
	$alignbytes = '8';
	$cstflags = 0;
}
elsif ($p9p_objtype eq 'sparc') {
	$objext = '.k';
	$alignbytes = '4';
	$cstflags = 0;
}
print OUT "obj_ext='$objext'\n";
print OUT "alignbytes='$alignbytes'\n";
print OUT "castflags='$cstflags'\n";

$myname = $ENV{'site'} ;
($myhostname,$mydomain) = split(/\./,$myname,2);
print OUT "myhostname='$myhostname'\n" if $myhostname;
if ($mydomain) {
  print OUT "mydomain='.$mydomain'\n";
  print OUT "perladmin='$cf_by\@$myhostname.$mydomain'\n";
  print OUT "cf_email='$cf_by\@$myhostname.$mydomain'\n";
}
else {
  print OUT "perladmin='$cf_by'\n";
  print OUT "cf_email='$cf_by'\n";
}
print OUT "myuname='Plan9 $myname $osvers $p9p_objtype'\n";

# Before we read the C header file, find out what config.sh constants are
# equivalent to the C preprocessor macros
if (open(SH,"${outdir}config_h.SH")) {
  while (<SH>) {
    next unless m%^#(?!if).*\$%;
    s/^#//; s!(.*?)\s*/\*.*!$1!;
    my(@words) = split;
    $words[1] =~ s/\(.*//;  # Clip off args from macro
    # Did we use a shell variable for the preprocessor directive?
    if ($words[0] =~ m!^\$(\w+)!) { $pp_vars{$words[1]} = $1; }
    if (@words > 2) {  # We may also have a shell var in the value
      shift @words;              #  Discard preprocessor directive
      my($token) = shift @words; #  and keep constant name
      my($word);
      foreach $word (@words) {
        next unless $word =~ m!\$(\w+)!;
        $val_vars{$token} = $1;
        last;
      }
    }
  }
  close SH;
}
else { warn "Couldn't read ${outfile}config_h.SH: $!\n"; }
$pp_vars{PLAN9} = 'define'; #Plan 9 specific

# OK, now read the C header file, and retcon statements into config.sh
while (<IN>) {  # roll through the comment header in config.h
  last if /config-start/;
}

while (<IN>) {
  chop;
  while (/\\\s*$/) {  # pick up contination lines
    my $line = $_;
    $line =~ s/\\\s*$//;
    $_ = <IN>;
    s/^\s*//;
    $_ = $line . $_;
  }              
  next unless my ($blocked,$un,$token,$val) =
                 m%^(\/\*)?\s*\#\s*(un)?def\w*\s+([A-Za-z0-9]\w+)\S*\s*(.*)%;
  if (/config-skip/) {
    delete $pp_vars{$token} if exists $pp_vars{$token};
    delete $val_vars{$token} if exists $val_vars{$token};
    next;
  }
  $val =~ s!\s*/\*.*!!; # strip off trailing comment
  my($had_val); # Maybe a macro with args that we just #undefd or commented
  if (!length($val) and $val_vars{$token} and ($un || $blocked)) {
    print OUT "$val_vars{$token}=''\n";
    delete $val_vars{$token};
    $had_val = 1;
  }
  $state = ($blocked || $un) ? 'undef' : 'define';
  if ($pp_vars{$token}) {
    print OUT "$pp_vars{$token}='$state'\n";
    delete $pp_vars{$token};
  }
  elsif (not length $val and not $had_val) {
    # Wups -- should have been shell var for C preprocessor directive
    warn "Constant $token not found in config_h.SH\n";
    $token =~ tr/A-Z/a-z/;
    $token = "d_$token" unless $token =~ /^i_/;
    print OUT "$token='$state'\n";
  }
  next unless length $val;
  $val =~ s/^"//; $val =~ s/"$//;               # remove end quotes
  $val =~ s/","/ /g;                            # make signal list look nice
 
 if ($val_vars{$token}) {
    print OUT "$val_vars{$token}='$val'\n";
    if ($val_vars{$token} =~ s/exp$//) {print OUT "$val_vars{$token}='$val'\n";}
    delete $val_vars{$token};
  }
  elsif (!$pp_vars{$token}) {  # Haven't seen it previously, either
    warn "Constant $token not found in config_h.SH (val=|$val|)\n";
    $token =~ tr/A-Z/a-z/;
    print OUT "$token='$val'\n";
    if ($token =~ s/exp$//) {print OUT "$token='$val'\n";}
  }
}
close IN;

foreach (sort keys %pp_vars) {
  warn "Didn't see $_ in $infile\n";
}
foreach (sort keys %val_vars) {
  warn "Didn't see $_ in $infile(val)\n";
}


# print OUT "libs='",join(' ',@libs),"'\n";
# print OUT "libc='",join(' ',@crtls),"'\n";

if (open(PL,"${outdir}patchlevel.h")) {
  while (<PL>) {
    if    (/^#define PERL_VERSION\s+(\S+)/) {
      print OUT "PERL_VERSION='$1'\n";
      print OUT "PATCHLEVEL='$1'\n";		# XXX compat
    }
    elsif (/^#define PERL_SUBVERSION\s+(\S+)/) {
      print OUT "PERL_SUBVERSION='$1'\n";
      print OUT "SUBVERSION='$1'\n";		# XXX compat
    }
  }
  close PL;
}
else { warn "Can't read ${outdir}patchlevel.h - skipping 'PERL_VERSION'"; }

print OUT "pager='/bin/p'\n";

close OUT;


