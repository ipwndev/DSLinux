#
# This script was written by Renaud Deraison
#
# See the Nessus Scripts License for details
#
# 
# This is the "check" for an old flaw (published in March 2002). We can't
# actually determine the version of the remote mod_frontpage, so we issue
# an alert each time we detect it as running.
#
# Mandrake's Security Advisory states that the flaw is remotely exploitable,
# while FreeBSD's Security advisory (FreeBSD-SA-02:17) claims this is only
# locally exploitable. 
#
# In either case, we can't remotely determine the version of the server, so
# 
# Ref:
# From: FreeBSD Security Advisories <security-advisories@freebsd.org>
# To: FreeBSD Security Advisories <security-advisories@freebsd.org>   
# Subject: FreeBSD Ports Security Advisory FreeBSD-SA-02:17.mod_frontpage
# Message-Id: <200203121428.g2CES9U64467@freefall.freebsd.org>

if(description)
{
 script_id(11303);
 
 script_cve_id("CAN-2002-0427");
 script_bugtraq_id(4251);
 
 script_version("$Revision$");
 
 name["english"] = "mod_frontpage installed";

 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is using the Apache mod_frontpage module.

mod_frontpage older than 1.6.1 is vulnerable to a buffer
overflow which may allow an attacker to gain root access.

*** Since Nessus was not able to remotely determine the version
*** of mod_frontage you are running, you are advised to manually
*** check which version you are running as this might be a false
*** positive.

If you want the remote server to be remotely secure, we advise
you do not use this module at all.


Solution : Disable this module
Risk factor : High";


 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for the presence of mod_frontpage";
 
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2003 Renaud Deraison");
 family["english"] = "CGI abuses";
 script_family(english:family["english"]);
 script_dependencie("find_service.nes", "no404.nasl");
 script_require_ports("Services/www", 80);
 script_require_keys("www/apache");
 exit(0);
}

#
# The script code starts here
#

include("http_func.inc");

port = get_kb_item("Services/www");
if(!port)port = 80;
if(get_port_state(port))
{
 banner = get_http_banner(port:port);
 if(!banner)exit(0);

 
 if(egrep(pattern:"^Server:.*Apache.*FrontPage.*", string:banner))
 {
   security_hole(port);
 }
}
