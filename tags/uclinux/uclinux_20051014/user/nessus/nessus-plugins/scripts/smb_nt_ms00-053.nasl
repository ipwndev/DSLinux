#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10485);
 script_version ("$Revision$");
 script_bugtraq_id(1535);
 script_cve_id("CVE-2000-0737");

 name["english"] =  "Service Control Manager Named Pipe Impersonation patch";
 name["francais"] = "Service Control Manager Named Pipe Impersonation patch";
 
 script_name(english:name["english"],
 	     francais:name["francais"]);
 
 desc["english"] = "
The hotfix for the 'Service Control Manager Named Pipe Impersonation'
problem has not been applied.

This vulnerability allows a malicious user, who has the
right to log on this host locally, to gain additional privileges.

Solution : See http://www.microsoft.com/technet/security/bulletin/ms00-053.asp
Risk factor : Medium";


 desc["francais"] = "
Le hotfix pour le probl�me de spoof du protocole du
serveur de noms NetBIOS n'a pas �t� appliqu�.

Cette vuln�rabilit� permet � un utilisateur malicieux ayant
le droit de se logguer sur ce serveur locallement d'obtenir
plus de droits sur celui-ci.

Solution : cf http://www.microsoft.com/technet/security/bulletin/ms00-053.asp
Facteur de risque : Moyen";


 script_description(english:desc["english"],
 		    francais:desc["francais"]);
 
 summary["english"] = "Determines whether the hotfix Q269523 is installed";
 summary["francais"] = "D�termine si le hotfix Q269523 est install�";
 script_summary(english:summary["english"],
 		francais:summary["francais"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2000 Renaud Deraison");
 family["english"] = "Windows";
 script_family(english:family["english"]);
 
 script_dependencies("netbios_name_get.nasl",
 		     "smb_login.nasl", "smb_registry_access.nasl",
		     "smb_reg_service_pack_W2K.nasl"
		     );
 script_require_keys("SMB/name", "SMB/login", "SMB/password", "SMB/registry_access");
 script_require_ports(139, 445);
 exit(0);
}


include("smb_nt.inc");
port = get_kb_item("SMB/transport");
if(!port)port = 139;

#---------------------------------------------------------------------#
# Here is our main()                                                  #
#---------------------------------------------------------------------#

access = get_kb_item("SMB/registry_access");
if(!access)exit(0);

version = get_kb_item("SMB/WindowsVersion");
if(version == "5.0")
{
 sp = get_kb_item("SMB/Win2K/ServicePack");
 if(ereg(string:sp, pattern:"Service Pack [2-9]"))
	exit(0);
	
	
 key = "SOFTWARE\Microsoft\Windows NT\CurrentVersion\HotFix\Q269523";
 item = "Comments";
 value = registry_get_sz(item:item, key:key);
 if(!value)security_hole(port);
}
