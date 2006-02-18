#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require Config; import Config;
    unless ($Config{'d_fork'}) {
	print "1..0 # Skip: no fork\n";
	exit 0;
    }
}

$| = 1;
print "1..15\n";

# External program 'tr' assumed.
open(PIPE, "|-") || (exec 'tr', 'YX', 'ko');
print PIPE "Xk 1\n";
print PIPE "oY 2\n";
close PIPE;

if ($^O eq 'vmesa') {
    # Doesn't work, yet.
    for (3..6) {
	print "ok $_ # skipped\n";
    }
} else {
    if (open(PIPE, "-|")) {
	while(<PIPE>) {
	    s/^not //;
	    print;
	}
	close PIPE;        # avoid zombies which disrupt test 12
    }
    else {
	# External program 'echo' assumed.
	print STDOUT "not ok 3\n";
	exec 'echo', 'not ok 4';
    }

    pipe(READER,WRITER) || die "Can't open pipe";

    if ($pid = fork) {
	close WRITER;
	while(<READER>) {
	    s/^not //;
	    y/A-Z/a-z/;
	    print;
	}
	close READER;     # avoid zombies which disrupt test 12
    }
    else {
	die "Couldn't fork" unless defined $pid;
	close READER;
	print WRITER "not ok 5\n";
	open(STDOUT,">&WRITER") || die "Can't dup WRITER to STDOUT";
	close WRITER;
	# External program 'echo' assumed.
	exec 'echo', 'not ok 6';
    }
}
wait;				# Collect from $pid

pipe(READER,WRITER) || die "Can't open pipe";
close READER;

$SIG{'PIPE'} = 'broken_pipe';

sub broken_pipe {
    $SIG{'PIPE'} = 'IGNORE';       # loop preventer
    print "ok 7\n";
}

print WRITER "not ok 7\n";
close WRITER;
sleep 1;
print "ok 8\n";

# VMS doesn't like spawning subprocesses that are still connected to
# STDOUT.  Someone should modify tests #9 to #12 to work with VMS.

if ($^O eq 'VMS') {
    print "ok 9 # skipped\n";
    print "ok 10 # skipped\n";
    print "ok 11 # skipped\n";
    print "ok 12 # skipped\n";
    exit;
}

if ($Config{d_sfio} || $^O eq 'machten' || $^O eq 'beos' || $^O eq 'posix-bc') {
    # Sfio doesn't report failure when closing a broken pipe
    # that has pending output.  Go figure.  MachTen doesn't either,
    # but won't write to broken pipes, so nothing's pending at close.
    # BeOS will not write to broken pipes, either.
    # Nor does POSIX-BC.
    print "ok 9 # skipped\n";
}
else {
    local $SIG{PIPE} = 'IGNORE';
    open NIL, '|true'	or die "open failed: $!";
    sleep 5;
    print NIL 'foo'	or die "print failed: $!";
    if (close NIL) {
	print "not ok 9\n";
    }
    else {
	print "ok 9\n";
    }
}

if ($^O eq 'vmesa') {
    # These don't work, yet.
    print "ok 10 # skipped\n";
    print "ok 11 # skipped\n";
    print "ok 12 # skipped\n";
    exit;
}

# check that errno gets forced to 0 if the piped program exited non-zero
open NIL, '|exit 23;' or die "fork failed: $!";
$! = 1;
if (close NIL) {
    print "not ok 10\n# successful close\n";
}
elsif ($! != 0) {
    print "not ok 10\n# errno $!\n";
}
elsif ($? == 0) {
    print "not ok 10\n# status 0\n";
}
else {
    print "ok 10\n";
}

if ($^O eq 'mpeix') {
    print "ok 11 # skipped\n";
    print "ok 12 # skipped\n";
} else {
    # check that status for the correct process is collected
    my $zombie = fork or exit 37;
    my $pipe = open *FH, "sleep 2;exit 13|" or die "Open: $!\n";
    $SIG{ALRM} = sub { return };
    alarm(1);
    my $close = close FH;
    if ($? == 13*256 && ! length $close && ! $!) {
        print "ok 11\n";
    } else {
        print "not ok 11\n# close $close\$?=$?   \$!=", $!+0, ":$!\n";
    };
    my $wait = wait;
    if ($? == 37*256 && $wait == $zombie && ! $!) {
        print "ok 12\n";
    } else {
        print "not ok 12\n# pid=$wait first=$pid pipe=$pipe zombie=$zombie me=$$ \$?=$?   \$!=", $!+0, ":$!\n";
    }
}

# Test new semantics for missing command in piped open
# 19990114 M-J. Dominus mjd@plover.com
{ local *P;
  print (((open P, "|    " ) ? "not " : ""), "ok 13\n");
  print (((open P, "     |" ) ? "not " : ""), "ok 14\n");
}

# check that status is unaffected by implicit close
{
    local(*NIL);
    open NIL, '|exit 23;' or die "fork failed: $!";
    $? = 42;
    # NIL implicitly closed here
}
if ($? != 42) {
    print "# status $?, expected 42\nnot ";
}
print "ok 15\n";
$? = 0;
