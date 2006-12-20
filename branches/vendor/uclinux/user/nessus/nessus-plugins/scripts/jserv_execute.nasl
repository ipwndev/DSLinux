#
# Written by Michael Scheidell <scheidell at secnap.net>
# based on a script written by Hendrik Scholz <hendrik@scholz.net> 
#
# See the Nessus Scripts License for details
#
#

if(description)
{
 script_id(10925);
 script_version("$Revision$");
 script_cve_id("CAN-2001-0307");
 
 name["english"] = "Oracle Jserv Executes outside of doc_root";
 script_name(english:name["english"]);
 
 desc["english"] = "

Detects Vulnerability in the execution of JSPs outside
doc_root.

A potential security vulnerability has been discovered in
Oracle JSP releases 1.0.x through 1.1.1 (in
Apache/Jserv). This vulnerability permits access to and
execution of unintended JSP files outside the doc_root in
Apache/Jserv. For example, accessing
http://www.example.com/a.jsp//..//..//..//..//..//../b.jsp
will execute b.jsp outside the doc_root instead of a.jsp
if there is a b.jsp file in the matching directory.

Further, Jserv Releases 1.0.x - 1.0.2 have additional
vulnerability:

Due to a bug in Apache/Jserv path translation, any
URL that looks like:
http://host:port/servlets/a.jsp, makes Oracle JSP
execute 'd:\servlets\a.jsp' if such a directory
path actually exists. Thus, a URL virtual path, an
actual directory path and the Oracle JSP name
(when using Oracle Apache/JServ) must match for
this potential vulnerability to occur.

Vulnerable systems:
Oracle8i Release 8.1.7, iAS Release version 1.0.2
Oracle JSP, Apache/JServ Releases version 1.0.x - 1.1.1

Solution:
Upgrade to OJSP Release 1.1.2.0.0, available on Oracle
Technology Network's OJSP web site.

Risk factor : High";

 script_description(english:desc["english"]);
 
 summary["english"] = "Oracle Jserv Server type and version";
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2002 Michael Scheidell");
 family["english"] = "General";
 script_family(english:family["english"]);

 script_dependencie("find_service.nes", "httpver.nasl", "no404.nasl",  "http_version.nasl");
 script_require_ports("Services/www", 80);
 script_require_keys("www/apache");
 exit(0);
}

#
# The script code starts here
#
include("http_func.inc");

 port = get_kb_item("Services/www");
 if (!port) port = 80;

 if (get_port_state(port))
 {
  soctcp80 = open_sock_tcp(port);

  if (soctcp80)
  {
   data = http_get(item:"/", port:port);
   resultsend = send(socket:soctcp80, data:data);
   resultrecv = http_recv_headers(soctcp80);
   str = egrep(pattern:"apachejserv/1\.",string:tolower(resultrecv));


   if(ereg(pattern:".*apachejserv/1\.(0|1\.[0-1])",string:str))
      security_hole(port);
  }
  close(soctcp80);
 }
