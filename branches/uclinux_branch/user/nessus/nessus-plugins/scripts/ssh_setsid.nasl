#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#
# Ref: http://www.ssh.com/company/newsroom/article/286/
#
# Note: This is about SSH.com's SSH, not OpenSSH !!
#

if(description)
{
 script_id(11169);
 script_version ("$Revision$");
 
 
 name["english"] = "SSH setsid() vulnerability";
 script_name(english:name["english"]);
 
 desc["english"] = "
You are running a version of SSH which is 
older than version 3.1.5 or 3.2.2.

There is a bug in that version which may allow
a user to obtain higher privileges due to a flaw
in the way setsid() is used.


Solution : Upgrade to the latest version of SSH
See also : http://www.ssh.com/company/newsroom/article/286/
Risk factor : High";
	
	

 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for the remote SSH version";
 summary["francais"] = "V�rifie la version de SSH";
 script_summary(english:summary["english"], francais:summary["francais"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 2002 Renaud Deraison",
		francais:"Ce script est Copyright (C) 2002 Renaud Deraison");
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


banner = tolower(banner);

if("f-secure" >< banner)exit(0);

if(ereg(pattern:"^ssh-.*-2\.0\.1[0-3][^0-9].*$", string:banner))
	security_hole(port);
	
if(ereg(pattern:"^ssh-.*-3\.1\.[0-4][^0-9].*$", string:banner))
	security_hole(port);
	
if(ereg(pattern:"^ssh-.*-3\.2\.[0-1][^0-9].*$", string:banner))
	security_hole(port);	

