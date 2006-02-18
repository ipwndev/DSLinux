#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10480);
 script_version ("$Revision$");
 script_bugtraq_id(1457);
 script_cve_id("CVE-2000-0628");
 name["english"] = "Apache::ASP source.asp";
 name["francais"] = "Apache::ASP source.asp";
 script_name(english:name["english"], francais:name["francais"]);
 
 desc["english"] = "
The file /site/eg/source.asp is present.

This file comes with the Apache::ASP package
and allows anyone to write to files in the
same directory.

An attacker may use this flaw to upload his
own scripts and execute arbitrary commands
on this host.

Solution : Upgrade to Apache::ASP 1.95
Risk factor : Serious";



 desc["francais"] = "
Le fichier /site/eg/source.asp est pr�sent.

Ce fichier vient avec le package Apache::ASP 
and permet � n'importe qui d'�crire dans 
des fichiers arbitraires dans le meme repertoire
que celui-ci.

Un pirate peut utiliser ce probl�me pour uploader
des scripts faits maison, et ainsi executer des commandes
arbitraires sur ce serveur.

Solution : Mettez Apache::ASP � jour en version 1.95
Facteur de risque : S�rieux";


 script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "Checks for the presence of /site/eg/source.asp";
 summary["francais"] = "V�rifie la pr�sence de /site/eg/source.asp";
 
 script_summary(english:summary["english"], francais:summary["francais"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 2000 Renaud Deraison",
		francais:"Ce script est Copyright (C) 2000 Renaud Deraison");
 family["english"] = "CGI abuses";
 family["francais"] = "Abus de CGI";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes", "no404.nasl", "http_version.nasl");
 script_require_ports("Services/www", 80);
 script_require_keys("www/apache");
 exit(0);
}

#
# The script code starts here
#

port = is_cgi_installed("/site/eg/source.asp");
if(port)
{
 security_hole(port);
}
