#
# (C) Tenable Network Security
#
#
# The following HTTP requests have been provided as examples by 
# David Litchfield (david@nextgenss.com): 
#
# GET / HTTP/1.1 
# Host: iis-server 
# Authorization: Basic cTFraTk6ZDA5a2xt 

# GET / HTTP/1.1 
# Host: iis-server 
# Authorization: Negotiate TlRMTVNTUAABAAAAB4IAoAAAAAAAAAAAAAAAAAAAAAA=


if(description)
{
 script_id(11871);
 script_version("$Revision$");
 script_cve_id("CAN-2002-0419");          
 name["english"] = "Find if IIS server allows BASIC and/or NTLM authentication";

 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host appears to be running a version of IIS which allows remote 
users to determine which authentication schemes are required for confidential 
webpages. 

That is, by requesting valid webpages with purposely invalid credentials, you 
can ascertain whether or not the authentication scheme is in use.  This can 
be used for brute-force attacks against known UserIDs.

Solution : None at this time
Risk Factor : Low";

 script_description(english:desc["english"]);
 
 summary["english"] = "Find IIS authentication scheme";
 
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 
 script_copyright(english:"This script is Copyright (C) 2003 Tenable Network Security");
 family["english"] = "Misc.";
 script_family(english:family["english"]);
 script_dependencie("find_service.nes", "no404.nasl", "http_version.nasl");
 script_require_keys("www/iis");
 script_require_ports("Services/www", 80);
 exit(0);
}

#
# The script code starts here
#
include("http_func.inc");

port = get_kb_item("Services/www");
if(!port)port = 80;
if(!get_port_state(port))exit(0);



auth[0] = "- IIS Basic authentication";
auth[1] = "- IIS NTLM authentication";
req[0] = string("GET / HTTP/1.1\r\nHost: ", get_host_name(), "\r\nAuthorization: Basic cTFraTk6ZDA5a2xt\r\n\r\n");
req[1] = string ("GET / HTTP/1.1\r\nHost: ", get_host_name(), "\r\nAuthorization: Negotiate TlRMTVNTUAABAAAAB4IAoAAAAAAAAAAAAAAAAAAAAAA=\r\n\r\n");
flag=0;

mywarning = string("
The remote host appears to be running a version of IIS which allows remote 
users to determine which authentication schemes are required for confidential 
webpages. 

Specifically, the following methods are enabled on the remote webserver:\n");



for (i=0; req[i]; i++) {
  soc = http_open_socket(port);
  if (soc) {
    send(socket:soc, data:req[i]);
    r = http_recv(socket:soc);
    if (r =~ "401 Unauthorized" && egrep(string:r, pattern:"^Server:.*IIS")) {
        mywarning = mywarning + string(auth[i], " is enabled\n"); 
        flag++;
    }
    close(soc);
  }
}

mywarning += string("\n\nSolution : None at this time\nRisk Factor : Low");

if (flag) security_note(port:port, data:mywarning);
exit(0);



