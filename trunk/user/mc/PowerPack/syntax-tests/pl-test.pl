#!/usr/bin/perl

$inip=$ARGV[0];
$metrov=$ARGV[1];
$minut=$ARGV[2];

#$| = 1;

    require DBI;
    require DBD::mysql;

$dbname="";
$username="";
$dbpass="";

$data="prices";
$oper="operators";
$mach="sessions";

#################### main block ######################

$dbh = DBI->connect("dbi:mysql:$dbname", $username, $dbpass) || die "Cannot connect to db server $DBI::errstr,\n";

test_ip($inip);

exit;
################ end of main block ###################

########################################################################
sub latest_ip_data()
{
my($ipp)=@_;


   $sql="select MAX(start) from $pays where ip='$ipp'";
      $sth=$dbh->prepare($sql);
    $sth->execute();
	$ccdatetime=$sth->fetchrow();
    $sth->finish();

   $sql="select input,output,pay,ostatok from $pays where date_format(start, '%Y-%m-%d %H:%i:%S')='$ccdatetime' and ip='$ipp'";
    $sth=$dbh->prepare($sql);
    $sth->execute();
    $ccd=$sth->fetchrow_arrayref();
	$input  = $ccd->[0];
	$output = $ccd->[1];
	$last_pay = $ccd->[2];
	$ostatok  = $ccd->[3];
    $sth->finish();

return $ccdatetime,$input,$output,$last_pay,$ostatok;
}
########################################################################


#############################################################
sub params
{
  $sql="select p_metr,p_hour,min_traff from $admin limit 1";

    $sth=$dbh->prepare($sql);
    $sth->execute();

    $row=$sth->fetchrow_arrayref();
	my $metrp  = $row->[0];
	my $hourp  = $row->[1];
	my $traffm = $row->[2];
     $sth->finish();

return ($metrp,$hourp,$traffm);
}
########################################################################
########################################################################
sub test_ip()
{
my($ip)=@_;

my($Data,$IN,$OUT,$BABKA,$XBOCT) = &latest_ip_data($ip);
my($ad_metr,$ad_hour,$ad_traff)=params();
my($into,$outro)=iptraf($ip);

$metrov = $IN + $OUT;

$metrov /= 950000;
$minut /= 60;
$hours = $minut/60;

$metrov=sprintf("%.3f",$metrov);
$hours=sprintf("%.2f",$hours);

$bablo = $metrov * $ad_metr + $hours * $ad_hour;
$minimum  = $ad_metr * $ad_traff;

$OXBOCTOK = $XBOCT - $bablo;

if($OXBOCTOK le $minimum) { block($ip); }
###-------------------------------------------
    $sql="
	update
	    $sessions
	set 
            ostatok='$OXBOCTOK',
	    in='$into',
	    out='$outro'
	where 
	    ip='$ip' 
	and 
	    date_format(date, '%Y-%m-%d %H:%i:%S')='$Data'";

    $dbh->do($sql);

return 1;
}