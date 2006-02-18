#
# This script was written by Javier Fernandez-Sanguino Pe�a <jfs@computer.org>
# based on the iis_samples.nasl script written by Renaud Deraison
# Script was modified by Jasmin Amidzic <jasminsabina@yahoo.com>.
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(10629);
 script_version ("$Revision$");
 script_bugtraq_id(881);
 script_cve_id("CAN-2000-0021", "CAN-2002-0664");

 name["english"] = "Lotus Domino administration databases";
 script_name(english:name["english"]);
 
 desc["english"] = "
This script determines if some default databases can be read
remotely.

An anonymous user can retrieve information from this
Lotus Domino server: users, databases, configuration
of servers (including operating system and hard
disk partitioning), logs of access to users (which
could expose sensitive data if GET html forms are used)..

This issues are discussed in  'Lotus White Paper:
A Guide to Developing Secure Domino Applications' (december 1999)
http://www.lotus.com/developers/devbase.nsf/articles/doc1999112200

Solution: verify all the ACLs for these databases and remove those not needed
Risk factor : Medium/Serious";
# This really could be high if, for example some 
# sensitive data, but same databases do not give
# much information. Make separate tests for each?


 script_description(english:desc["english"]);
 
 summary["english"] = "Checks if Lotus Domino administration databases can be anonymously accessed";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 2001 Javier Fern�ndez-Sanguino Pe�a",
		francais:"Ce script est Copyright (C) 2001 Javier Fern�ndez-Sanguino Pe�a");
# Maybe instead of CGI abuses this family should be called HTTP server abuses
 family["english"] = "CGI abuses";
 family["francais"] = "Abus de CGI";
 script_family(english:family["english"], francais:family["francais"]);
# This should also depend on finding a Lotus Domino server
 script_dependencie("find_service.nes", "http_version.nasl", "no404.nasl");
 script_require_ports("Services/www", 80);
 script_require_keys("www/domino");
 exit(0);
}

#
# The script code starts here
#
include("http_func.inc");
include("http_keepalive.inc");

auth = NULL;

function test_cgi(port, db, output)
{
 ok = is_cgi_installed_ka(port:port, item:db);
 if(ok)
  {
  	# Check that the remote db is not actually password protected
  	req = http_get(item:db, port:port);
	r = http_keepalive_send_recv(port:port, data:req);
	
	if("Please identify yourself" >!< r &&
	   'type="password"' >!< r)
		{
		report = string(report, ". ", db, " this must be considered a security risk since ", output,"\n");
		set_kb_item(name:string("www/domino/", port, "/db"), value:db);
		}
	auth += ". " + db + '\n';
  }
 return(0);
}
 
 
report = "";

port = get_kb_item("Services/www");
if(!port)port = 80;
if(get_port_state(port))
{
 soc = open_sock_tcp(port);
 if(!soc)exit(0);
 
 req = http_head(item:"/", port:port);
 send(socket:soc, data:req);
 r = http_recv(socket:soc);
 close(soc);
 

 
 
  test_cgi(port:port, 
 	  db:"/log.nsf",
	  output:"the server log can be retrieved");
 
  test_cgi(port:port, 
 	  db:"/setup.nsf",
	  output:"the server might be configured remotely or the current setup might be downloaded");
	  
  test_cgi(port:port, 
 	  db:"/catalog.nsf",
	  output:"the list of databases in the server can be retrieved");
 
  test_cgi(port:port, 
 	  db:"/statrep.nsf",
	  output:"the reports generated by administrators can be read anoymously");

  test_cgi(port:port, 
 	  db:"/names.nsf",
	  output:"the users and groups in the server can be accessed anonymously, in some cases, access to the hashed passwords will be possible");
	  
  test_cgi(port:port, 
 	  db:"/domlog.nsf",
	  output:"the logs of the domain servers  can be read anonymously");

  test_cgi(port:port, 
 	  db:"/webadmin.nsf",
	  output:"the server administration database can be read anonymously");

  test_cgi(port:port, 
 	  db:"/cersvr.nsf",
	  output:"the information on the server certificates can be read anonymously");
	  
  test_cgi(port:port, 
 	  db:"/events4.nsf",
	  output:"the list of events that have taken place can be read anonymously, this might lead to information disclosure of users and hidden databases");

  test_cgi(port:port,
  	   db:"/zmevladm.nsf",
	   output:"it provides arbitrary users with Manager level access, which allows the users to read or modify the import/export scripts");

 # We should add more info here on the output: on how this database
 # affects the server
 
 
  foreach db (make_list("/mab.nfs", "/ntsync4.nsf", "/collect4.nsf", 
  		 	"/mailw46.nsf", "/bookmark.nsf", "/agentrunner.nsf",
			"/mail.box", "/admin4.nsf", "/catalog.nsf", 
			"/AgentRunner.nsf", "/certlog.nsf", "/cpa.nsf",
			"/domcfg.nsf", "/domguide.nsf", "/domlog.nsf",
			"/doc/dspug.nsf", "/doc/helpadmn.nsf",
			"/doc/javapg.nsf", "/doc/readmec.nsf",
			"/doc/readmes.nsf", "/doc/svrinst.nsf", 
			"/doc/wksinst.nsf", "/archive/a_domlog.nsf",
			"/archive/l_domlog.nsf", "/help/decsdoc.nsf",
			"/help/dols_help.nsf", "/help/help5_admin.nsf",
			"/help/help5_client.nsf", "/help/help5_designer.nsf",
			"/help/lccon.nsf", "/help/lsxlc.nsf", 
			"/help4.nsf", "/homepage.nsf", "/sample/faqw46.nsf",
			"/sample/framew46.nsf", "/smtpibwq.nsf", 
			"/smtpobwq.nsf", "/smtptbls.nsf", "/statmail.nsf",
			"/statrep.nsf", "/stats675.nsf", "/lccon.nsf", 
			"/loga4.nsf", "/helplt4.nsf", "/qstart.nsf", 
			"/quickstart/qstart50.nsf", "/quickstart/wwsample.nsf",
			"/mtabtbls.nsf", "/names.nsf", "/proghelp/KBCCV11.NSF",
			"/doladmin.nsf", "/busytime.nsf", "/reports.nsf",
			"/iNotes/Forms5.nsf", "/mail/admin.nsf",
			"/software.nsf", "/domino.nsf", "/books.nsf",
			"/default.nsf", "/db.nsf", "/database.nsf",
			"/users.nsf", "/groups.nsf", "/group.nsf", "/user.nsf",
			"/ldap.nsf", "/notes.nsf", "/secret.nsf",
			"/accounts.nsf", "/products.nsf", "/account.nsf", 
			"/secure.nsf", "/hidden.nsf", "/public.nsf", 
			"/private.nsf", "/welcome.nsf", "/calendar.nsf",
			"/nntppost.nsf", "/help/readme.nsf", "/help/help6_client.nsf",
			"/help/help6_designer.nsf", "/help/help6_admin.nsf",
			"/certsrv.nsf", "/dbdirman.nsf", "/lndfr.nsf",
			"/home.nsf" ))
 
  	test_cgi(port:port, 
 	  db:db,
	  output:"this database can be read anonymously");


 if(report)
  {
  report = string("We found the following domino databases :\n", report);
  security_hole(port:port, data:report);
  }

  if(auth)
  {
   security_warning(data:'The following databases exists but are password-protected:\n'+auth, port:port);
  }
    exit(0);

}




