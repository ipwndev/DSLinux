
#
# This script was written by Renaud Deraison <deraison@cvs.nessus.org>
#
# See the Nessus Scripts License for details
#

if(description)
{
 script_id(11365);
 script_cve_id("CAN-2002-0257");
 script_bugtraq_id(4069);
 script_version ("$Revision$");

 name["english"] = "Auction Deluxe XSS";
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote Auction Deluxe server is vulnerable to 
a cross site scripting attack.

As a result, a user could easily steal the cookies
of your legitimate users and impersonate them.

Solution : Upgrade to Auction Deluxe 3.30 or newer
Risk factor : Medium";




 script_description(english:desc["english"]);
 
 summary["english"] = "Checks for auction.pl";
 
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 2003 Renaud Deraison",
		francais:"Ce script est Copyright (C) 2003 Renaud Deraison");
 family["english"] = "CGI abuses";
 family["francais"] = "Abus de CGI";
 script_family(english:family["english"], francais:family["francais"]);
 script_dependencie("find_service.nes", "no404.nasl", "cross_site_scripting.nasl");
 script_require_ports("Services/www", 80);
 exit(0);
}


include("http_func.inc");
include("http_keepalive.inc");

port  = get_kb_item("Services/www");
if(!port) port = 80;
if(!get_port_state(port))exit(0);
if(get_kb_item(string("www/", port, "/generic_xss"))) exit(0);

dir = make_list(cgi_dirs());
foreach d (dir)
{
 if(isnull(dirs))dirs = make_list(string(d, "/auctiondeluxe"));
 else dirs = make_list(dirs, string(d, "/auctiondeluxe"));
}


foreach dir (make_list(dirs, ""))
{
 req = http_get(item:string(dir, "/auction.pl?searchstring=<script>foo</script>"), port:port);
 res = http_keepalive_send_recv(port:port, data:req, bodyonly:TRUE);
 if(res == NULL) exit(0);
 if(!ereg(pattern:"^HTTP/[0-9]\.[0-9] 200 ", string:res))exit(0);

 str = egrep(pattern:"<script>foo</script>", string:res, icase:TRUE);
 if(str)
 {
    security_warning(port);
    exit(0);
 }
}
