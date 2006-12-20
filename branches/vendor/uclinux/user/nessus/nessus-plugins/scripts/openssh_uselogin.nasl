#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10439);
 script_version ("$Revision$");
 script_bugtraq_id(1334);
 script_cve_id("CVE-2000-0525");
 name["english"] = "OpenSSH < 2.1.1 UseLogin feature";
 name["francais"] = "OpenSSH < 2.1.1 UseLogin feature";
 script_name(english:name["english"], francais:name["francais"]);
 
 desc["english"] = "
You are running a version of OpenSSH which is older
than 2.1.1.

If the UseLogin option is enabled, then sshd
does not switch to the uid of the user logging
in. Instead, sshd relies on login(1) to do the
job. However, if the user specifies a command
for remote execution, login(1) cannot be used
and sshd fails to set the correct user id,
so the command is run with the same privilege as sshd
(usually root privileges).

*** Note that Nessus did not determine whether the UseLogin
*** option was activated or not, so this message may
*** be a false alarm

Solution : Upgrade to OpenSSH 2.1.1 or make sure
that the option UseLogin is set to no in sshd_config

Risk factor : High";
	
	
 desc["francais"] = "
Vous faites tourner une version d'OpenSSH plus
ancienne que la version 2.1.1.

Si l'option UseLogin est activ�e, alors sshd 
ne change pas d'uid lorsqu'un utilisateur
se loggue. A la place, il utilise login(1)
pour ceci. Cependant, si l'utilisateur
sp�cifie une commande � executer, alors login(1)
ne peut etre utilis� et sshd ne change pas d'uid
pour executer la commande qui est ainsi execut�e
avec ses privil�ges (root le plus souvent)

*** Notez que Nessus ne peut determiner si l'option UseLogin
*** est activ�e dans le fichier de config, donc ce message
*** peut etre une fausse alerte.

Solution : Mettez OpenSSH � jour en version 2.1.1
ou v�rifiez que l'option UseLogin est mise � 'no' dans
sshd_config

Facteur de risque : Elev�";


 script_description(english:desc["english"], francais:desc["francais"]);
 
 summary["english"] = "Checks for the remote OpenSSH version";
 summary["francais"] = "V�rifie la version de OpenSSH";
 script_summary(english:summary["english"], francais:summary["francais"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 2000 Renaud Deraison",
		francais:"Ce script est Copyright (C) 2000 Renaud Deraison");
 family["english"] = "Gain root remotely";
 family["francais"] = "Passer root � distance";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes");
 script_require_ports("Services/ssh", 22);
 exit(0);
}

#
# The script code starts here
#

port = get_kb_item("Services/ssh");
if(!port)port = 22;

key = string("ssh/banner/", port);
banner = get_kb_item(key);


if(!banner)
{
  if(get_port_state(port))
  {
    soc = open_sock_tcp(port);
    if(!soc)exit(0);
    banner = recv_line(socket:soc, length:1024);
    close(soc);
  }
}

if(!banner)exit(0);

b = banner - string("\r\n");

if("OpenSSH" >< b){
 if(ereg(pattern:"SSH-.*-OpenSSH[-_]((1\.*)|(2\.[0-1]))", string:b))
 {
  security_hole(port);
 }
}
