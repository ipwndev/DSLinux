# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10911);
 script_version("$Revision$");
 name["english"] = "Local users information : automatically disabled accounts";

 script_name(english:name["english"]);
 
 desc["english"] = "
This script displays the names of the automatically disabled
accounts.

These accounts may have been victim to brute force
attacks attempts.

Risk factor : Low";



 script_description(english:desc["english"]);
 
 summary["english"] = "Lists the users that have special privileges";

 
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 2002 Renaud Deraison");
 family["english"] = "Windows : User management";
 script_family(english:family["english"]);
 script_dependencies("smb_netusergetinfo_local.nasl");
 
 exit(0);
}


port = get_kb_item("SMB/transport");
if(!port)port = 139;


logins = "";
count = 1;
login = get_kb_item(string("SMB/LocalUsers/", count));
while(login)
{
 acb = get_kb_item(string("SMB/LocalUsers/", count, "/Info/ACB"));
 if(acb)
 {
  if(acb & 0x0400){
  	logins = string(logins, login, "\n");
	}
 }
 count = count + 1;
 login = get_kb_item(string("SMB/LocalUsers/", count));
}

if(logins)
{
 rep = string("The following local accounts were disabled automatically by the system:\n\n",
  logins,
  "\n\nThis probably means that these accounts were subject to brute force attacks\n",
  "Risk factor : Low");
 security_note(port:port, data:rep);
}
