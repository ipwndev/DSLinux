#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10274);
 script_version ("$Revision$");
 script_bugtraq_id(952);
 script_cve_id("CVE-2000-0113");
 
 name["english"] = "SyGate Backdoor";
 script_name(english:name["english"]);
 
 desc["english"] = "
SyGate engine remote controller seems to be running on
this port. It may be used by malicious users which
are on the same subnet as yours to reconfigure your
Sybase engine.

Risk factor : Serious";

 script_description(english:desc["english"]);
 
 summary["english"] = "Detects whether SyGate remote controller is running";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2000 Renaud Deraison");
 family["english"] = "Backdoors";
 script_family(english:family["english"]);
 script_require_ports(7323);
 exit(0);
}

#
# The script code starts here
#

port = 7323;
if (get_port_state(port))
{
 soc = open_sock_tcp(port);

 if (soc)
 {
   banner = telnet_init(soc);
   if("yGate" >< banner)security_hole(port);
 }
 close(soc);
}
