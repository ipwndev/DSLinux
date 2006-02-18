#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10011);
 script_version ("$Revision$");
 script_bugtraq_id(770);
 script_cve_id("CAN-1999-0885");
 
 name["english"] = "get32.exe vulnerability";
 name["francais"] = "get32.exe";
 script_name(english:name["english"], francais:name["francais"]);
 
 desc["english"] = "The 'get32.exe' CGI script is installed on this 
 machine. This CGI has a well known security flaw that allows an 
 attacker to execute arbitrary commands on the remote system with 
 the privileges of the HTTP daemon (typically root or nobody).

Solution : Remove the 'get32.exe' script from your web server's 
CGI directory (usually cgi-bin/)..

Risk factor : Serious";


 desc["francais"] = "Le cgi 'get32.exe' est install�. Celui-ci poss�de
un probl�me de s�curit� bien connu qui permet � n'importe qui de faire
executer des commandes arbitraires au daemon http, avec les privil�ges
de celui-ci (root ou nobody). 

Solution : retirez-le de /cgi-bin.

Facteur de risque : S�rieux";


 script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "Checks for the presence of /cgi-bin/get32.exe";
 summary["francais"] = "V�rifie la pr�sence de /cgi-bin/get32.exe";
 
 script_summary(english:summary["english"], francais:summary["francais"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 1999 Renaud Deraison",
		francais:"Ce script est Copyright (C) 1999 Renaud Deraison");
 family["english"] = "CGI abuses";
 family["francais"] = "Abus de CGI";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes", "no404.nasl");
 script_require_ports("Services/www", 80);
 exit(0);
}

#
# The script code starts here
#

port = is_cgi_installed("get32.exe");
if(port)
{
 security_hole(port);
}
