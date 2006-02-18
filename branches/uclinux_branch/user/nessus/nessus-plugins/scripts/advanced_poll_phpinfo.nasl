#
# This script was written by Renaud Deraison
#

if(description)
{
 script_version ("$Revision$");
 script_id(11487);
 script_bugtraq_id(7171);
 
 name["english"] = "Advanced Poll info.php";

 script_name(english:name["english"]);

 desc["english"] = "
The remote host is running Chien Kien Uong's Advanced Poll,
a simple Poll system using PHP.

By default, this utility includes two files called info.php,
located in [path to poll]/db/misc/info.php and 
[path to poll]/text/misc/info.php.

This files make a call to phpinfo() which display a lot of information
about the remote host and how PHP is configured.

An attacker may use this flaw to gain a more intimate knowledge
about the remote host and better prepare its attacks.

Solution : Delete these two files
Risk factor : Low";


 script_description(english:desc["english"]);
 summary["english"] = "Checks for the presence of info.php";
 summary["francais"] = "V�rifie la pr�sence de info.php";
 script_summary(english:summary["english"], francais:summary["francais"]);
 script_category(ACT_GATHER_INFO);
 script_copyright(english:"This script is Copyright (C) 2003 Renaud Deraison",
                francais:"Ce script est Copyright (C) 2003 Renaud Deraison");
 family["english"] = "CGI abuses";
 family["francais"] = "Abus de CGI";
 script_family(english:family["english"], francais:family["francais"]);
 script_require_ports("Services/www", 80);
 script_dependencies("http_version.nasl", "no404.nasl");
 exit(0);
}

#
# The script code starts here
#
include("http_func.inc");
include("http_keepalive.inc");

port = get_kb_item("Services/www");
if(!port) port=80;



foreach dir (make_list("", "/poll", cgi_dirs()))
{
 req = http_get(item:string(dir, "/db/misc/info.php"), port:port);
 res = http_keepalive_send_recv(port:port, data:req);
 if( res == NULL ) exit(0);
 if("<title>phpinfo()</title>" >< res)
 	{
	security_warning(port);
	exit(0);
	}

  req = http_get(item:string(dir, "/text/misc/info.php"), port:port);
  res = http_keepalive_send_recv(port:port, data:req);
  if( res == NULL ) exit(0);
  if("<title>phpinfo()</title>" >< res)	
  	{
	security_warning(port);
	exit(0);
	}
}
