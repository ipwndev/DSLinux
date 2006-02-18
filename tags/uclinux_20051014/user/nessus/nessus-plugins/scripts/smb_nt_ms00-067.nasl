#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10519);
 script_version ("$Revision$");
 script_bugtraq_id(1683);
 script_cve_id("CVE-2000-0834");

 name["english"] =  "Telnet Client NTLM Authentication Vulnerability";
 name["francais"] = "Telnet Client NTLM Authentication Vulnerability";
 
 script_name(english:name["english"],
 	     francais:name["francais"]);
 
 desc["english"] = "
The hotfix for the 'Telnet Client NTLM Authentication'
problem has not been applied.

This vulnerability may, under certain circumstances, allow a malicious
user to obtain cryptographically protected logon credentials from
another user.
Solution : See http://www.microsoft.com/technet/security/bulletin/ms00-067.asp
Risk factor : Medium";


 desc["francais"] = "
Le hotfix pour le probl�me d'authentification NTLM du client telnet
n'a pas �t� install�.

Cette vulnerabilit� permet � un pirate, dans certaines circonstances,
d'obtenir le mot de passe crypt� d'un autre utilisateur. 

Solution : cf http://www.microsoft.com/technet/security/bulletin/ms00-067.asp
Facteur de risque : Moyen";


 script_description(english:desc["english"],
 		    francais:desc["francais"]);
 
 summary["english"] = "Determines whether the hotfix Q272743 is installed";
 summary["francais"] = "D�termine si le hotfix Q272743 est install�";
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
access = get_kb_item("SMB/registry_access");
if(!access)exit(0);

port = get_kb_item("SMB/transport");
if(!port)port = 139;

#---------------------------------------------------------------------#
# Here is our main()                                                  #
#---------------------------------------------------------------------#

version = get_kb_item("SMB/WindowsVersion");
if(version == "5.0")
{
 sp = get_kb_item("SMB/Win2K/ServicePack");
 if(ereg(string:sp, pattern:"Service Pack [2-9]"))
	exit(0);
	
	
	
 key = "SOFTWARE\Microsoft\Windows NT\CurrentVersion\HotFix\Q272743";
 item = "Comments";
 value = registry_get_sz(key:key, item:item);
 if(!value)security_hole(port);
}
