<?
/*
//	freshmeat.php3
//
//      Author:         Kalle Kiviaho   kivi@ios.chalmers.se
//      Version:        1.1
//      Last update:    1999-03-22
//      URL:            http://swamp.ios.chalmers.se/
//
//  This free software is licensed under the terms of the
//  GNU public license.
//  Copyright 1999 Kalle Kiviaho
//
//      Usage:  
//              <?
//              include("freshmeat.php3");
//              ?>
//
//	It needs write access to $cache_file!
//
*/

$hostname	=	"core.freshmeat.net";
$port		=	80;
$uri		=	"/backend/recentnews.txt";
$header_passed	=	0;
$link_prefix	=	"&nbsp;&nbsp;o ";
$cache_file	=	"/tmp/freshmeat.cache";
$cache_time	=	3600;
$current_time	=	split(" ", microtime(), 2);

if (($current_time[1] - filemtime($cache_file) > $cache_time) || (!file_exists($cache_file))) {
	$fpread = fsockopen("$hostname", $port, &$errno, &$errstr);
	if(!$fpread) {
		echo "$errstr ($errno)<br>\n";
	} else {
		fputs($fpread,"GET $uri HTTP/1.0\n\n");

		$fpwrite = fopen($cache_file, "w");
		while (!feof($fpread)) {
			if ($header_passed == 1) {
				$info = Chop(fgets($fpread, 80));
				$date = Chop(fgets($fpread, 80));
				$link = Chop(fgets($fpread, 80));
				fputs($fpwrite, "$link_prefix<A HREF=\"$link\">$info</A><BR>\n");
			}
			if ($header_passed == 0) {
				if (Chop(fgets($fpread, 80)) == "") {
					$header_passed = 1;
				}
			}
		}
		fclose($fpwrite);
	}
	fclose($fpread);
}
include($cache_file);
?>
