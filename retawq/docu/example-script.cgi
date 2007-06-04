#! /usr/bin/perl

# This is a script example for the local CGI feature of the web browser retawq
# (<http://retawq.sourceforge.net/>).

# This is a Perl script (expecting a Perl interpreter at /usr/bin/perl), but
# retawq can use "scripts" (arbitrary programs) in any interpreted or compiled
# programming language. The script only has to be executable; so you might have
# to apply a command like "chmod u+x <scriptname>"; cf. retawq/docu/scheme.html
# for more information.

# Local CGI scripts are supported since retawq version 0.1.2. retawq currently
# uses the Common Gateway Interface version 1.1; for information about this,
# read <http://www.w3.org/CGI/>.

# The header of a CGI script for retawq is exactly the same as a normal HTTP
# header with one exception: the first header line (e.g. "HTTP/1.0 200 OK") is
# left out; instead, the status code is given in a "Status" header line, e.g.
# "Status: 200" if the script was able to do what it should or "Status: 500" if
# something went wrong. For possible status codes, read RFC2616, 6.1.1 and 10
# (<ftp://ftp.rfc-editor.org/in-notes/rfc2616.txt>).

# Each header line is terminated with a "\r\n"; the end of the header is
# indicated by an empty header line:

print "Status: 200\r\nContent-Type: text/plain\r\n\r\n";

# After the header, let's print a little message:

print "Hold Orwell!"; # (These letters are better known as "Hello World!":-)
exit(0);

# ...and that's it. If you want to generate an HTML page instead of plain text,
# use the content type "text/html" instead of "text/plain" like so:

print "Status: 200\r\nContent-Type: text/html\r\n\r\n";
print "<html><head><title>Oh well, lord!</title></head>
<body>Old hellrow!</body></html>";
exit(0);
