#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10429);
 script_version ("$Revision$");
 script_cve_id("CAN-1999-0589");
 name["english"] = "SMB Registry : permissions of winlogon";
 name["francais"] = "V�rification des permissions de winlogon";
 
 script_name(english:name["english"],
 	     francais:name["francais"]);
 
 desc["english"] = "

The registry key HKLM\Software\Microsoft\Windows NT\CurrentVersion\Winlogon
is writeable by users who are not in the admin group.

This key contains a value which defines which program should
be run when a user logs on.

As this program runs in the SYSTEM context, the users who 
have the right to change the value of this key
can gain more privileges on this host.


Solution : use regedt32 and set the permissions of this
key to :

	- admin group  : Full Control
	- system       : Full Control
	- everyone     : Read
	
Risk factor : High";


 desc["francais"] = "
 
La cl� HKLM\Software\Microsoft\Windows NT\CurrentVersion\Winlogon
de la base de registre peut etre acc�d�e en �criture
par des utilisateurs n'�tant pas membres du groupe admin.

Cette cl� contient une valeur indiquant quel programme devrait
etre lanc� lorsqu'un utilisateur se loggue. 

Comme ce programme est lanc� dans le contexte SYSTEM, les 
utilisateurs qui ont le droit de changer cette valeur
peuvent obtenir plus de privil�ges sur ce syst�me.

Solution : utilisez regedt32 et changez les permissions
de cette cl� en :

	- groupe admin  : control total
	- sytem         : control total
	- tout le monde : lecture
	
	
Facteur de risque : Elev�";


 script_description(english:desc["english"],
 		    francais:desc["francais"]);
 
 summary["english"] = "Determines the access rights of a remote key";
 summary["francais"] = "D�termine les droits d'acc�s d'une cl� distante";
 script_summary(english:summary["english"],
 		francais:summary["francais"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2000 Renaud Deraison");
 family["english"] = "Windows";
 script_family(english:family["english"]);
 
 script_dependencies("netbios_name_get.nasl",
 		     "smb_login.nasl", "smb_registry_access.nasl");
 script_require_keys("SMB/transport", "SMB/name", "SMB/login", "SMB/password", "SMB/registry_access");
 script_require_ports(139, 445);
 exit(0);
}


include("smb_nt.inc");

key = "Software\Microsoft\Windows NT\CurrentVersion\Winlogon";
val = registry_get_acl(key:key);
if(!val)exit(0);

if(registry_key_writeable_by_non_admin(security_descriptor:val))
 security_hole(get_kb_item("SMB/transport"));

