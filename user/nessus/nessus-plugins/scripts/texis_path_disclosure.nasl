#
# This script is (C) Renaud Deraison
#




if(description)
{
 script_id(11401);
 script_version ("$Revision$");
 script_cve_id("CAN-2002-0266");
 script_bugtraq_id(4035);
 

 name["english"] = "texi.exe path disclosure";
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote CGI 'texis.exe' discloses the physical path of the remote
web server when requested a non-existing file.

Solution : Upgrade to the latest version
Risk factor : Low";

 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for texis.exe";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 2003 Renaud Deraison");
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


foreach d (make_list("", cgi_dirs()))
{
req = http_get(item:string(d, "/texis.exe/nessus"), port:port);
res = http_keepalive_send_recv(port:port, data:req);

if ( res == NULL ) exit (0);
if(egrep(pattern:"[a-z]:\\.*\\nessus", string:res)) {
  	security_warning(port);
	exit(0);
}
}
