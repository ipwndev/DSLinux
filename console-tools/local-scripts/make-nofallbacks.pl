#!/usr/bin/perl

($PATTERN, $HOPEFULL_PATTERN) = @ARGV;

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

    if (($label =~ m/$HOPEFULL_PATTERN/) and ($label !~ m/$PATTERN/)) {
	print "$code;$label\n";
    }
}
