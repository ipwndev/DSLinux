# gtkBitchX.spec.  Generated from gtkBitchX.spec.in by configure.

%define ver	1.1-final
%define rel	1
%define ser	2
%define pfx	/usr/local

Summary: The Ultimate IRC Client
Name: gtkBitchX
Version: %{ver}
Release: %{rel}
Serial: %{ser}
Prefix: %{pfx}
Packager: David Walluck <david@bitchx.org>
Copyright: BSD-Style
Group: Networking/IRC
Source: ftp://ftp.bitchx.org/pub/source/BitchX-%{version}.tar.bz2
Url: http://www.bitchx.org/gtk/
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Requires: ncurses, XFree86-libs, glib >= 1.2.0, gtk+ >= 1.2.0, gnome-libs >= 1.0.0, tcl, openssl, esound >= 0.2.5, audiofile >= 0.1.5, BitchX-common = %{version}
BuildRequires: ncurses-devel, XFree86-libs, XFree86-devel, glib-devel >= 1.2.0, gtk+-devel >= 1.2.0, gnome-libs-devel >= 1.0.0, tcl, openssl-devel, esound-devel >= 0.2.5, audiofile-devel >= 0.1.5

%description 
BitchX is a VERY heavily modified ircII client. It includes many things such
as built in CDCC (XDCC) offering, built in flood protection, etc. It is easier
to script things in BitchX because unlike plain vanilla ircII, half the
script does not have to be devoted to changing the appearance of ircII. It also
includes many other new features, such as port scanning, a CD player, a mail
client, etc.

This version of BitchX contains GTK support and, as such, requires X to run.

%prep
%setup -n BitchX

%build
%ifarch i386 i486 i586 i686 k6 k7
CFLAGS="$RPM_OPT_FLAGS" LDFLAGS="-s" \
	./configure --prefix=%{prefix} --with-tcl --with-ssl --with-gtk --enable-sound --with-plugins=nicklist --enable-ipv6
%else
CFLAGS="$RPM_OPT_FLAGS" LDFLAGS="-s" \
	./configure --prefix=%{prefix} --with-tcl --with-ssl --with-gtk --enable-sound --with-plugins=nicklist --enable-ipv6
%endif

make

%install
rm -rf $RPM_BUILD_ROOT

sed -e 's/loadacts = \[NO\]/loadacts = \[YES\]/' -e 's/\$exedir\/fonts/\%{prefix}\/share\/figlet/' < script/menu.bx > script/menu.bx.new
install -m 644 script/menu.bx.new script/menu.bx
rm -rf script/menu.bx.new

make prefix=$RPM_BUILD_ROOT/%{prefix} install
ln -sf gtkBitchX-%{version} $RPM_BUILD_ROOT/%{prefix}/bin/gtkBitchX

cat > gtkBitchX.menu << EOF
?package(gtkBitchX): \\
	needs="X11" \\
	section="Networking/IRC" \\
	title="gtkBitchX" \\
	longtitle="The Ultimate IRC Client" \\
	command="gtkBitchX" \\
	icon="BitchX.xpm"
EOF
install -d $RPM_BUILD_ROOT/usr/lib/menu
install -m 644 gtkBitchX.menu $RPM_BUILD_ROOT/usr/lib/menu/gtkBitchX

%clean
rm -rf $RPM_BUILD_ROOT

%post
%{update_menus}

%postun
%{clean_menus}

%files
%defattr(-,root,root)
%{prefix}/bin/gtkBitchX
%{prefix}/bin/gtkBitchX-%{version}
%{prefix}/lib/bx/script/menu.bx
%{prefix}/lib/bx/script/actplug.gmz
%{prefix}/lib/bx/plugins/nicklist.so
/usr/lib/menu/gtkBitchX

%changelog
* Mon Oct 18 1999 David Walluck <david@bitchx.org> 1.0c16-1
- initial release of spec for inclusion in BitchX
