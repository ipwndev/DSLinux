# BitchX.spec.  Generated from BitchX.spec.in by configure.

%define ver	1.1-final
%define rel	1
%define ser	2
%define pfx	/usr/local

Summary: The Ultimate IRC Client
Name: BitchX
Version: %{ver}
Release: %{rel}
Serial: %{ser}
Prefix: %{pfx}
Packager: David Walluck <david@bitchx.org>
Copyright: BSD-Style
Group: Networking/IRC
Source: ftp://ftp.bitchx.org/pub/source/BitchX-%{version}.tar.bz2
Url: http://www.bitchx.org/
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Requires: ncurses, tcl, BitchX-common = %{version}
BuildRequires: ncurses-devel, tcl

%description 
BitchX is a VERY heavily modified ircII client. It includes many things such 
as built in CDCC (XDCC) offering, built in flood protection, etc. It is easier
to script things in BitchX because unlike plain vanilla ircII, half the 
script does not have to be devoted to changing the appearance of ircII. It also
includes many other new features, such as port scanning, a CD player, a mail
client, etc.

%package common
Summary: Help files, documentation, and extras for the BitchX IRC client
Group: Networking/IRC

%description common 
These are the help files and other documentation for the BitchX IRC client. It
is required that you install this package so that you at least have all of the
files needed by BitchX. 

This package requires either the BitchX or the gtkBitchX package to do anything
useful.

%prep
%setup -n BitchX
find . -type f -name '*~' | xargs rm -rf
find . -type d -name 'CVS' | xargs rm -rf

%build
%ifarch i386 i486 i586 i686 k6 k7
CFLAGS="$RPM_OPT_FLAGS" LDFLAGS="-s" \
	./configure --prefix=%{prefix} --with-tcl --with-plugins
%else
CFLAGS="$RPM_OPT_FLAGS" LDFLAGS="-s" \
	./configure --prefix=%{prefix} --with-plugins
%endif

make

%install
rm -rf $RPM_BUILD_ROOT

rm -f script/fserve+vfs.tar.gz
rm -f script/actplug.gmz
rm -f script/menu.bx

make prefix=$RPM_BUILD_ROOT/%{prefix} install
ln -sf BitchX-%{version} $RPM_BUILD_ROOT/%{prefix}/bin/BitchX

cat > BitchX.menu << EOF
?package(BitchX): \\
	needs="X11" \\
	section="Networking/IRC" \\
	title="BitchX" \\
	longtitle="The Ultimate IRC Client" \\
	command="rxvt -geometry 80x24 -bg black -fg white -fn vga -e BitchX" \\
	icon="BitchX.xpm"
EOF
install -d $RPM_BUILD_ROOT/usr/lib/menu
install -m 644 BitchX.menu $RPM_BUILD_ROOT/usr/lib/menu/BitchX

install -d $RPM_BUILD_ROOT/usr/share/icons
install -m 664 doc/BitchX.xpm $RPM_BUILD_ROOT/usr/share/icons
install -d $RPM_BUILD_ROOT/usr/share/icons/mini
install -m 664 doc/BitchX-mini.xpm $RPM_BUILD_ROOT/usr/share/icons/mini/BitchX.xpm

cat >ircII.servers <<EOF
EOF
install -m 644 ircII.servers $RPM_BUILD_ROOT/%{prefix}/lib/bx

%clean
rm -rf $RPM_BUILD_ROOT

%post
%{update_menus}

%postun
%{clean_menus}

%files
%defattr(-,root,root)
%{prefix}/bin/BitchX
%{prefix}/bin/BitchX-%{version}
%{prefix}/lib/bx/wserv
%{prefix}/bin/scr-bx 
/usr/lib/menu/BitchX

%files common
%defattr(-,root,root)
%doc ChangeLog INSTALL README README.GTK README.autoconf doc/*
%dir %{prefix}/lib/bx
%config(noreplace) %{prefix}/lib/bx/ircII.servers
%{prefix}/lib/bx/BitchX.help
%{prefix}/lib/bx/BitchX.ircnames
%{prefix}/lib/bx/BitchX.kick
%{prefix}/lib/bx/BitchX.quit
%{prefix}/lib/bx/help
%{prefix}/lib/bx/plugins
%dir %{prefix}/lib/bx/script
%config(noreplace) %{prefix}/lib/bx/script/bxglobal
%{prefix}/lib/bx/script/bxtcl.tcl
%{prefix}/lib/bx/script/file.tcl
%{prefix}/lib/bx/script/fserve.irc
%{prefix}/lib/bx/translation
/usr/share/icons/BitchX.xpm
/usr/share/icons/mini/BitchX.xpm
%{prefix}/man/man1/*

%changelog
* Tue Aug 29 2000 David Walluck <david@bitchx.org> 1.0c17-1
- update to 1.0c17

* Mon Oct 18 1999 David Walluck <david@bitchx.org> 1.0c16-1
- initial release of spec for inclusion in BitchX
