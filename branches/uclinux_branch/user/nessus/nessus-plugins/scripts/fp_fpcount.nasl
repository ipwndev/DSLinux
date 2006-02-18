#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
#
# See the Nessus Scripts License for details
#
# Added some extra checks. Axel Nennker axel@nennker.de

if(description)
{
 script_id(11370);
 script_version ("$Revision$");
 script_cve_id("CAN-1999-1376");

 name["english"] = "fpcount.exe overflow";
 name["francais"] = "d�passement de buffer dans fpcount.exe";

 script_name(english:name["english"],
	     francais:name["francais"]);
 
 # Description
 desc["english"] = "
There might be a buffer overflow in the remote
fpcount.exe cgi.

*** Nessus did not actually check for this flaw,
*** but solely relied on the presence of this CGI
*** instead

An attacker may use it to execute arbitrary code
on this host.

Solution : delete it
Risk factor : High";


 script_description(english:desc["english"]);

 # Summary
 summary["english"] = "Is fpcount.exe installed ?";
 script_summary(english:summary["english"]);

 # Category
 script_category(ACT_GATHER_INFO); 

 # Dependencie(s)
 script_dependencie("find_service.nes", "no404.nasl");
 
 # Family
 family["english"] = "CGI abuses";
 family["francais"] = "Abus de CGI";
 script_family(english:family["english"],
 	       francais:family["francais"]);
 
 # Copyright
 script_copyright(english:"This script is Copyright (C) 2003 Renaud Deraison",
 		  francais:"Ce script est Copyright (C) 2003 Renaud Deraison");
 
 script_require_ports("Services/www", 80);
 exit(0);
}

# The attack starts here
include("http_func.inc");
include("http_keepalive.inc");

port = get_kb_item("Services/www");
if(!port)port = 80;


req = http_get(item:"/_vti_bin/fpcount.exe", port:port);
res = http_keepalive_send_recv(port:port, data:req);

if( res == NULL ) exit(0);
if(("Microsoft-IIS/4" >< res) && ("HTTP/1.1 502 Gateway" >< res) )
	security_hole(port);
