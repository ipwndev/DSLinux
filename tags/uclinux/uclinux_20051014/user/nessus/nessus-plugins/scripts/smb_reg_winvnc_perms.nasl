#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10553);
 script_version ("$Revision$");
 script_bugtraq_id(1961);
 script_cve_id("CVE-2000-1164");

 name["english"] = "SMB Registry : permissions of WinVNC's key";
 name["francais"] = "V�rification des permissions de la cl� de la registry WinVNC";
 
 script_name(english:name["english"],
 	     francais:name["francais"]);
 
 desc["english"] = "

The registry key HKLM\Software\ORL\WinVNC3
is writeable and/or readable by users who are not in the admin group.

This key contains the VNC password of this host, as well
as other configuration setup.

As this program allows remote access to this computer with
the privileges of the currently logged on users, you should
fix this problem.


Solution : use regedt32 and set the permissions of this
key to :

	- admin group  : Full Control
	- system       : Full Control
	- everyone     : No access
	
Risk factor : High";


 desc["francais"] = "
 
La cl� HKLM\Software\ORL\WinVNC3
de la base de registre peut etre acc�d�e en �criture et/ou lecture
par des utilisateurs n'�tant pas membres du groupe admin.

Cette cl� contient le mot de passe du serveur VNC de cette
machine, ainsi que d'autres param�tres.

Comme ce programme permet d'acc�der � distance � cette machine
avec les privil�ges de l'utilisateur de la console, vous
devriez fixer ce probl�me.

Solution : utilisez regedt32 et changez les permissions
de cette cl� en :

	- groupe admin  : control total
	- sytem         : control total
	- tout le monde : pas d'acc�s
	
	
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



key = "Software\ORL\WinVNC3";

val = registry_get_acl(key:key);
if(!val)exit(0);

if(registry_key_writeable_by_non_admin(security_descriptor:val))
 security_hole(get_kb_item("SMB/transport"));

