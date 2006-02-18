#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10477);
 script_version ("$Revision$");
 script_bugtraq_id(1548);
 script_cve_id("CVE-2000-0672");
 name["english"] = "Tomcat's /admin is world readable";
 name["francais"] = "/admin de Tomcat est en lecture libre";
 script_name(english:name["english"], francais:name["francais"]);
 
 desc["english"] = "
The page  /admin/contextAdmin/contextAdmin.html
can be accessed.

This allows an attacker to add context to your Tomcat
web server, and potentially to read arbitrary files 
on this server.

Solution : restrict access to /admin or remove this
context, and do not run TomCat as root.
Risk factor : Serious";


 desc["francais"] = "
La page  /admin/contextAdmin/contextAdmin.html
est en lecture libre.

Cela permet � un pirate d'ajouter des contextes
� ce serveur Tomcat, et potentiellement d'obtenir
la possibilit� de lire des fichiers arbitraires
sur ce serveur.


Solution : restreignez l'acc�s � /admin ou
retirez completement ce contexte et ne
faites pas tourner tomcat en tant que root.
Facteur de risque : Serieux";


 script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "Checks for the presence of /admin";
 summary["francais"] = "V�rifie la pr�sence de /admin";
 
 script_summary(english:summary["english"], francais:summary["francais"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 2000 Renaud Deraison",
		francais:"Ce script est Copyright (C) 2000 Renaud Deraison");
 family["english"] = "CGI abuses";
 family["francais"] = "Abus de CGI";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes", "no404.nasl", "http_version.nasl");
 script_require_ports("Services/www", 8080);
 script_require_keys("www/apache");
 exit(0);
}

#
# The script code starts here
#
include("http_func.inc");

port = get_kb_item("Services/www");
if(!port)port = 8080;

if(!get_port_state(port))exit(0);

soc = http_open_socket(port);
if(soc)
{
 req = http_get(item:"/admin/contextAdmin/contextAdmin.html", port:port);
 send(socket:soc, data:req);
 r = http_recv(socket:soc);
 http_close_socket(soc);
 if(ereg(pattern:"HTTP/[0-9].[0-9] 200 ", string: r))
 {
  if("Servlet-Engine: Tomcat" >< r)
  {
   security_hole(port);
  }
 }
}
