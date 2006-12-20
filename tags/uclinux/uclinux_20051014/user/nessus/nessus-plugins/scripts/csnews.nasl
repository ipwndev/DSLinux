#
# This script was written by John Lampe...j_lampe@bellsouth.net 
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(11726);
 script_version ("$Revision$");
 script_cve_id("CVE-2002-0923");
 script_bugtraq_id(4994);
 
 
 name["english"] = "CSNews.cgi vulnerability";
 name["francais"] = "CSNews.cgi vulnerability";
 script_name(english:name["english"], francais:name["francais"]);
 
 desc["english"] = "
The CSNews.cgi exists on this webserver. Some versions of this file 
are vulnerable to remote exploit.

An attacker may make use of this file to gain access to
confidential data or escalate their privileges on the Web
server.

Solution : remove it from the cgi-bin or scripts directory.
Risk factor : Serious";


 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for the csnews.cgi file";
 
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 2003 John Lampe",
		francais:"Ce script est Copyright (C) 2003 John Lampe");
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

include("http_func.inc");
include("http_keepalive.inc");

port = get_kb_item("Services/www");
if(!port) port = 80;
if(!get_port_state(port))exit(0);

flag = 0;
directory = "";

foreach dir (cgi_dirs()) {
   if(is_cgi_installed_ka(item:string(dir, "/csNews.cgi"), port:port)) {
  	flag = 1;
  	directory = dir;
  	break;
   } 
}
 
if (flag) security_hole(port);
