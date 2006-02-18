#
# Michel Arboi <arboi@alussinan.org> hacked asp_source_data.nasl which
# was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# GPL
#
# Script audit and contributions from Carmichael Security <http://www.carmichaelsecurity.com>
#      Erik Anderson <eanders@carmichaelsecurity.com>
#      Added BugtraqID and CAN
#
# References:
# Date:  Fri, 29 Jun 2001 13:01:21 -0700 (PDT)
# From: "Extirpater" <extirpater@yahoo.com>
# Subject: 4 New vulns. vWebServer and SmallHTTP
# To: bugtraq@securityfocus.com, vuln-dev@securityfocus.com
#

if(description)
{
 script_id(11071);
 script_version ("$Revision$");
 script_cve_id("CAN-2001-1248");
 script_bugtraq_id(2975);
 name["english"] = "ASP source using %20 trick";
 name["francais"] = "Sources des fichiers ASP en ajoutant %20";
 script_name(english:name["english"], francais:name["francais"]);
 
 desc["english"] = "
It is possible to get the source code of the remote
ASP scripts by appending %20 at the end
of the request (like GET /default.asp%20)


ASP source code usually contains sensitive information such
as logins and passwords.

Solution :  install all the latest security patches
	
Risk factor : Serious";
	
 desc["francais"] = "
Il est possible d'obtenir le code source des fichiers
ASP distants en ajoutant %20 apr�s le nom du fichier
(comme par exemple GET /default.asp%20)


Les codes sources ASP contiennent souvent des informations
sensibles telles que des logins et des mots de passe.

Solution : installez tous les derniers patch de s�curit�
Facteur de risque : S�rieux";


 script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "downloads the source of ASP scripts";
 summary["francais"] = "t�l�charge le code source des fichiers ASP";
 
 script_summary(english:summary["english"], francais:summary["francais"]);
 
 script_category(ACT_GATHER_INFO);
 
 # In fact, Renaud wrote more than halt of this script!
 script_copyright(english:"This script is Copyright (C) 2002 Michel Arboi",
		francais:"Ce script est Copyright (C) 2002 Michel Arboi");
 family["english"] = "CGI abuses";
 family["francais"] = "Abus de CGI";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes", "webmirror.nasl", "http_version.nasl");
 script_require_ports("Services/www", 80);
 script_require_keys("www/iis");
 exit(0);
}

#
# The script code starts here
#

include("http_func.inc");

function check(file)
{
soc = http_open_socket(port);
 if(soc)
 {
  req = http_get(item:string(file, "%20"), port:port);
  send(socket:soc, data:req);
  r = http_recv(socket:soc);
  http_close_socket(soc);
  # I suspect that the test might be wrong...
  if("Content-Type: application/octet-stream" >< r){
  	security_hole(port);
	return(1);
	}
  # So I added this quick & dirty hack
  if (("<%" >< r) && ("%>" >< r)) {
	security_hole(port);
	return(1);
  }
 }
 return(0);
}


port = get_kb_item("Services/www");
if(!port)port = 80;
if(get_port_state(port))
{
 if(check(file:"/default.asp"))exit(0);
 files = get_kb_list(string("www/", port, "/content/extensions/asp"));
 if(isnull(files))exit(0);
 files = make_list(files);
 check(file:files[0]); 
}
