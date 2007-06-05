# rpm spec file for retawq version 0.2.6b (<http://retawq.sourceforge.net/>)
# Copyright (C) 2001-2005 Arne Thomassen <arne@arne-thomassen.de>
# Note that this is rather a skeleton than a complete spec file.

Name: retawq
Version: 0.2.6b
Release: 0
License: GPL
URL: http://retawq.sourceforge.net/
Source: ftp://ftp.sourceforge.net/pub/sourceforge/retawq/retawq-0.2.6b.tar.gz
Copyright: Arne Thomassen <arne@arne-thomassen.de>
Group: Applications/Internet
Summary: a multi-threaded web browser for text terminals

%doc README INSTALL COPYING
%docdir docu/

%description
retawq is an interactive, multi-threaded network client (web browser) for text
terminals on computers with Unix-like operating systems. It is written in C,
fast, small, nicely configurable, and comfortable; e.g. the low-level network
communications are performed in a non-blocking way, and you can keep open as
many "virtual windows" as you want and work simultaneously in two of them in a
split-screen mode.
