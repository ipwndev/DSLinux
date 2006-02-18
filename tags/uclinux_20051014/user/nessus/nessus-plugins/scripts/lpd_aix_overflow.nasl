#
# This script was written by Renaud Deraison
#
# See the Nessus Scripts License for details
#
# This is a check for an OLD flaw

if(description)
{
   script_id(11355);
   script_version ("$Revision$");
   script_cve_id("CAN-2001-0671");
   name["english"] = "Buffer overflow in AIX lpd";
  
   script_name(english:name["english"]);
 
   desc["english"] = "
The remote lpd daemon seems to be vulnerable to
various buffer overrflows in the functions send_status(),
kill_print() and chk_fhost().

*** Nessus solely relied on the version number of the remote
*** operating system to issue this warning, so this might be a
*** false positive

See also : http://www.cert.org/advisories/CA-2001-30.html
Solution : Upgrade AIX
Risk factor : High";


   script_description(english:desc["english"]);
 
   summary["english"] = "Determines if lpd is running";
   script_summary(english:summary["english"]);
 
   script_category(ACT_GATHER_INFO);
 
   script_copyright(english:"This script is Copyright (C) 2003 Renaud Deraison");
   script_family(english:"Gain root remotely");
   script_require_ports("Services/lpd", 515);
   script_dependencies("find_service.nes", "nmap_osfingerprint.nes");
 
   exit(0);
}



#
# The code starts here
#

os = get_kb_item("Host/OS");
if(!os)exit(0);
if("AIX" >!< os)exit(0);
if(!ereg(pattern:"AIX (5\.1|4\.3)", string:os))exit(0);

port = get_kb_item("Services/lpd");
if(!port)port = 515;

soc = open_sock_tcp(port);
if(!soc)exit(0);
else security_hole(port);
