#
# (C) Tenable Network Security
#
# Ref: 
# Date: 9 May 2003 16:58:36 -0000
# From: Charles Reinold <creinold@hotmail.com>
# To: bugtraq@securityfocus.com
# Subject: ttcms and ttforum exploits
#

if (description)
{
 script_id(11615);
 script_bugtraq_id(7543, 7542);
 script_version ("$Revision$");

 script_name(english:"ttforum multiple flaws");
 desc["english"] = "
The remote host is running ttforum.

This set of CGI is vulnerable to various attacks which
may allow an attacker to execute arbitrary code on this
host or gain administrative privileges on this forum.

Solution: Disable this forum or upgrade to a fixed version
Risk factor : High";

 script_description(english:desc["english"]);
 script_summary(english:"Determines if ttforum is vulnerable to code injection");
 script_category(ACT_GATHER_INFO);
 script_family(english:"CGI abuses", francais:"Abus de CGI");
 script_copyright(english:"This script is Copyright (C) 2003 Tenable Network Security");
 script_dependencie("find_service.nes", "no404.nasl");
 script_require_ports("Services/www", 80);
 exit(0);
}

include("http_func.inc");
include("http_keepalive.inc");

port = get_kb_item("Services/www");
if (!port) port = 80;
if(!get_port_state(port))exit(0);


dir = make_list("/modules/forum", "/ttforum", cgi_dirs(), "");
		


foreach d (dir)
{
 url = string(d, '/index.php?board=10;action=news;ext=help;template=http://xxxxxxxxxxxx');
 req = http_get(item:url, port:port);
 buf = http_keepalive_send_recv(port:port, data:req);
 if( buf == NULL ) exit(0);
 
 if("php_network_getaddresses: getaddrinfo" >< buf)
   {
    security_hole(port);
    exit(0);
   }
}
