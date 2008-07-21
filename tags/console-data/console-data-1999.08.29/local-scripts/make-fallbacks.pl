#!/usr/bin/perl

#
# Parse command-line
#

$PATTERN=$ARGV[0];
shift @ARGV;
@TRANSLATIONS = @ARGV;

#
# Parse each input line
#

my %unicodes = ();		# label => code  for each char matching $PATTARN
my @fallbacks = ();		# array of fallback entries, each of which is stored as
				# a hash whose keys are "labels" and "codes", and values 
				# are refs to arrays.

UNICODE: while (<STDIN>) {
#     ($code, $label, $categ, $comClass,
#      $bidiClass, $decompos, $decDigit, $digit, 
#      $numeric, $mirror, $oldName, $comment,
#      $upper, $lower, $title) = split (/;/);

    ($code, $label, $categ, undef,
     undef, undef, undef, undef,
     undef, undef, undef, undef,
     undef, undef, undef) = split (/;/);
    
    # skip control chars
    next UNICODE if (index ($categ, "C") == 0);

    # if this line is interesting
    if ($label =~ m/$PATTERN/) {
	# store the char in the hash for future use
	$unicodes{$label} = $code;

	# create a new fallback entry
	unshift (@fallbacks, {});
	$fallbacks[0]{labels} = [$label];
	
	# compute the accepted transformed char-labels
	for ($j = 0; $j <= $#TRANSLATIONS; $j++) {
	    $transl = $label;
	    $transl =~ s/$PATTERN/eval"\"$TRANSLATIONS[$j]\""/e;
	    push (@{$fallbacks[0]{labels}}, $transl);
	}
	next UNICODE;	# don't try to match with a smaller pattern
    }
}

# use Data::Dumper;
# print (Dumper(\@fallbacks));
# exit 0;

# process collected data into .fallback format
FB_ENTRY: foreach $fallback (@fallbacks) {
    # cleanup fallback line
    my $tmp = [];		# clean version of $fallback->{labels}
    my $code;
    foreach $char (@{$fallback->{labels}}) {
	if (($code = $unicodes{$char}) and not (grep (/$code/, @{$fallback->{codes}}))) {
	    push (@$tmp, $char);
	    push (@{$fallback->{codes}}, $code);
	}
    }
    $fallback->{labels} = $tmp;

    # if fallback entry only has one char (ie. no fallback), drop it
    if ($#{$fallback->{codes}} == 0) {
	next FB_ENTRY;
    }

    # describing comment line
    foreach $char (@{$fallback->{labels}}) {
	printf ("# %s ", $char);
    }
    print "\n";

    # the entry itself
    foreach $char (@{$fallback->{labels}}) {
	printf ("U+%s ", $unicodes{$char});
    }
    print "\n";
}
