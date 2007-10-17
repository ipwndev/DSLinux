# $Revision: 1.14 $, $Date: 2003-11-17 21:43:05 +0100 (Mon, 17 Nov 2003) $
Name: 		irssi
Version: 	0.8.12
Release: 	1
Vendor: 	Timo Sirainen <cras@irssi.org>
Summary:  	Irssi is a IRC client
License: 	GPL
Group: 		Applications/Communications
URL: 		http://www.irssi.org/
Source0: 	http://www.irssi.org/files/%{name}-%{version}.tar.gz
BuildRequires:  glib-devel >= 1.2.0, ncurses-devel
BuildRoot:      /tmp/%{name}-%{version}-root

%define		_sysconfdir	/etc
%define		configure { CFLAGS="${CFLAGS:-%optflags}" ; export CFLAGS ;  CXXFLAGS="${CXXFLAGS:-%optflags}" ; export CXXFLAGS ;  FFLAGS="${FFLAGS:-%optflags}" ; export FFLAGS ; ./configure %{_target_platform} 	--prefix=%{_prefix} --exec-prefix=%{_exec_prefix} --bindir=%{_bindir} --sbindir=%{_sbindir} --sysconfdir=%{_sysconfdir} --datadir=%{_datadir} --includedir=%{_includedir} --libdir=%{_libdir} --libexecdir=%{_libexecdir} --localstatedir=%{_localstatedir} --sharedstatedir=%{_sharedstatedir} --mandir=%{_mandir} --infodir=%{_infodir} }

%description
Irssi is a modular IRC client that currently has only text mode user
interface, but 80-90% of the code isn't text mode specific so other UI
could be created pretty easily. Also, Irssi isn't really even IRC
specific anymore, there's already a working SILC module available.
Support for other protocols like ICQ could be created some day too.

More information can be found at http://irssi.org/.

%prep
%setup -q

%build
%configure \
	--with-plugins \
	--enable-ipv6 \
	--with-textui \
	--with-imlib \
	--with-bot \
	--with-socks \
	--with-proxy \
	--with-perl="yes" \
	--with-ncurses
make

%install
rm -rf ${RPM_BUILD_ROOT}
make DESTDIR=${RPM_BUILD_ROOT} PREFIX=${RPM_BUILD_ROOT}%{_prefix} PERL_INSTALL_ROOT=${RPM_BUILD_ROOT} install
strip ${RPM_BUILD_ROOT}%{_bindir}/*
strip ${RPM_BUILD_ROOT}%{_libdir}/irssi/modules/lib*.so*

rm -f ${RPM_BUILD_ROOT}%{_libdir}/irssi/modules/*.{a,la} \
	${RPM_BUILD_ROOT}%{perl_archlib}/auto/Irssi/.packlist \
	${RPM_BUILD_ROOT}%{perl_archlib}/auto/Irssi/*/.packlist \
	${RPM_BUILD_ROOT}%{perl_archlib}/perllocal.pod
rm -rf ${RPM_BUILD_ROOT}%{_docdir}/irssi/

%clean
rm -rf ${RPM_BUILD_ROOT}
rm -rf ${RPM_BUILD_DIR}/%{name}-%{version}

%files 
%defattr(-, root, root, 0755)
%doc AUTHORS ChangeLog COPYING NEWS README TODO
%doc docs/*.txt docs/*.html
%doc %{_mandir}/man1/*
%config(noreplace) %{_sysconfdir}/*
%{_bindir}/*
%{_libdir}/irssi/
%{perl_archlib}
%{_datadir}/irssi/

%changelog
* Mon Nov 17 2003 Robert Scheck <irssi@robert-scheck.de>
  Fixed many things for better rebuilding and a good package

* Fri Aug 17 2001 - Joose Vettenranta <joose@iki.fi>
  Created new spec file from spec file founded in irssi-0.7.98.3
