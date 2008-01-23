%define initdir %{_sysconfdir}/rc.d/init.d
%define auth %(test -f /etc/pam.d/system-auth && echo /etc/pam.d/system-auth || echo)

Summary: The Samba SMB server.
Name: samba
Version: 3.0.13
Release: 1
License: GNU GPL Version 2
Group: System Environment/Daemons
URL: http://www.samba.org/

Source: ftp://www.samba.org/pub/samba/%{name}-%{version}.tar.bz2

# Red Hat specific replacement-files
Source1:  samba.log
Source2:  samba.xinetd
Source4:  samba.sysconfig
Source5:  smb.init
Source6:  winbind.init
Source7:  samba.pamd
Source8:  smbprint
Source9:  smbusers
Source10: smb.conf

# Don't depend on Net::LDAP
Source999: filter-requires-samba.sh

# generic patches

Requires: pam >= 0.64 %{auth} samba-common = %{version} 
Requires: logrotate >= 3.4 initscripts >= 5.54-1 
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Prereq: /sbin/chkconfig /bin/mktemp /usr/bin/killall
Prereq: fileutils sed /etc/init.d 
BuildRequires: pam-devel, readline-devel, ncurses-devel, fileutils, libacl-devel, openldap-devel, krb5-devel, cups-devel


# Working around perl dependency problem from docs
%define __perl_requires %{SOURCE999}

%description
Samba is the protocol by which a lot of PC-related machines share
files, printers, and other information (such as lists of available
files and printers). The Windows NT, OS/2, and Linux operating systems
support this natively, and add-on packages can enable the same thing
for DOS, Windows, VMS, UNIX of all kinds, MVS, and more. This package
provides an SMB server that can be used to provide network services to
SMB (sometimes called "Lan Manager") clients. Samba uses NetBIOS over
TCP/IP (NetBT) protocols and does NOT need the NetBEUI (Microsoft Raw
NetBIOS frame) protocol.

%package client
Summary: Samba (SMB) client programs.
Group: Applications/System
Requires: samba-common = %{version}
Obsoletes: smbfs

%description client
The samba-client package provides some SMB clients to compliment the
built-in SMB filesystem in Linux. These clients allow access of SMB
shares and printing to SMB printers.

%package common
Summary: Files used by both Samba servers and clients.
Group: Applications/System

%description common
Samba-common provides files necessary for both the server and client
packages of Samba.

%package swat
Summary: The Samba SMB server configuration program.
Group: Applications/System
Requires: samba = %{version} xinetd

%description swat
The samba-swat package includes the new SWAT (Samba Web Administration
Tool), for remotely managing Samba's smb.conf file using your favorite
Web browser.

%prep
%setup -q

# copy Red Hat specific scripts
cp %{SOURCE5} packaging/Fedora/
cp %{SOURCE6} packaging/Fedora/
cp %{SOURCE7} packaging/Fedora/
cp %{SOURCE8} packaging/Fedora/winbind.init

%build

cd source
%ifarch i386 sparc
RPM_OPT_FLAGS="$RPM_OPT_FLAGS -D_FILE_OFFSET_BITS=64"
%endif
%ifarch ia64
libtoolize --copy --force     # get it to recognize IA-64
autoheader                                               
autoconf
EXTRA="-D_LARGEFILE64_SOURCE"
%endif

## run autogen if missing the configure script
if [ ! -f "configure" ]; then
        ./autogen.sh
fi

CFLAGS="$RPM_OPT_FLAGS" ./configure \
	--prefix=%{_prefix} \
	--localstatedir=/var \
	--sysconfdir=/etc \
	--with-privatedir=%{_sysconfdir}/samba \
	--with-fhs \
	--with-quotas \
	--with-smbmount \
	--with-pam \
	--with-pam_smbpass \
	--with-syslog \
	--with-utmp \
	--with-sambabook=%{_datadir}/swat/using_samba \
	--with-swatdir=%{_datadir}/swat \
	--with-libsmbclient \
	--with-acl-support \
	--with-shared-modules=idmap_rid \
	--enable-cups=yes
make showlayout
make proto
make %{?_smp_mflags} all modules nsswitch/libnss_wins.so debug2html 


%install
rm -rf $RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT/sbin
mkdir -p $RPM_BUILD_ROOT/usr/{sbin,bin}
mkdir -p $RPM_BUILD_ROOT/%{initdir}
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/{pam.d,logrotate.d}
mkdir -p $RPM_BUILD_ROOT/var/{log,spool,lib}/samba
mkdir -p $RPM_BUILD_ROOT/%{_datadir}/swat/using_samba
mkdir -p $RPM_BUILD_ROOT/%{_datadir}/samba/codepages 

cd source

make DESTDIR=$RPM_BUILD_ROOT \
	install

cd ..

# Install other stuff
install -m644 %{SOURCE10} $RPM_BUILD_ROOT%{_sysconfdir}/samba/smb.conf
install -m644 %{SOURCE9} $RPM_BUILD_ROOT/etc/samba/smbusers
install -m755 %{SOURCE8} $RPM_BUILD_ROOT%{_bindir}
install -m644 %{SOURCE7} $RPM_BUILD_ROOT/etc/pam.d/samba
install -m644 %{SOURCE1} $RPM_BUILD_ROOT/etc/logrotate.d/samba
install -m755 source/script/mksmbpasswd.sh $RPM_BUILD_ROOT%{_bindir}

install -m755 %{SOURCE5} $RPM_BUILD_ROOT%{initdir}/smb
install -m755 %{SOURCE6} $RPM_BUILD_ROOT%{initdir}/winbind
ln -s ../..%{initdir}/smb  $RPM_BUILD_ROOT%{_sbindir}/samba
ln -s ../..%{initdir}/winbind  $RPM_BUILD_ROOT%{_sbindir}/winbind

ln -s ../usr/bin/smbmount $RPM_BUILD_ROOT/sbin/mount.smb
## Samba's Makefile is breaking this currently.  Remove it and set our own
/bin/rm -f $RPM_BUILD_ROOT/sbin/mount.smbfs
ln -s ../usr/bin/smbmount $RPM_BUILD_ROOT/sbin/mount.smbfs

echo 127.0.0.1 localhost > $RPM_BUILD_ROOT%{_sysconfdir}/samba/lmhosts


# pam_smbpass
mkdir -p $RPM_BUILD_ROOT/%{_lib}/security
mv source/bin/pam_smbpass.so $RPM_BUILD_ROOT/%{_lib}/security/pam_smbpass.so

# winbind
mkdir -p $RPM_BUILD_ROOT/%{_lib}/security
install -m 755 source/nsswitch/pam_winbind.so $RPM_BUILD_ROOT/%{_lib}/security/pam_winbind.so
install -m 755 source/nsswitch/libnss_winbind.so $RPM_BUILD_ROOT/%{_lib}/libnss_winbind.so
install -m 755 source/nsswitch/libnss_wins.so $RPM_BUILD_ROOT/%{_lib}/libnss_wins.so
( cd $RPM_BUILD_ROOT/%{_lib}; 
  ln -sf libnss_winbind.so  libnss_winbind.so.2;
  ln -sf libnss_wins.so  libnss_wins.so.2 )

# libsmbclient

# make install puts libsmbclient.so in the wrong place on x86_64
rm -f $RPM_BUILD_ROOT/usr/lib || true
mkdir -p $RPM_BUILD_ROOT%{_libdir} $RPM_BUILD_ROOT%{_includedir}
install -m 755 source/bin/libsmbclient.so $RPM_BUILD_ROOT%{_libdir}/libsmbclient.so
install -m 755 source/bin/libsmbclient.a $RPM_BUILD_ROOT%{_libdir}/libsmbclient.a
install -m 644 source/include/libsmbclient.h $RPM_BUILD_ROOT%{_includedir}
rm -f $RPM_BUILD_ROOT%{_libdir}/samba/libsmbclient.*

mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/xinetd.d
install -m644 %{SOURCE2} $RPM_BUILD_ROOT%{_sysconfdir}/xinetd.d/swat

mkdir -p $RPM_BUILD_ROOT%{_sysconfdir}/sysconfig
install -m644 %{SOURCE4} $RPM_BUILD_ROOT%{_sysconfdir}/sysconfig/samba

##
## Clean out man pages for tools not installed here
##
rm -f $RPM_BUILD_ROOT/%{_mandir}/man1/editreg.1*
rm -f $RPM_BUILD_ROOT%{_mandir}/man1/log2pcap.1*
rm -f $RPM_BUILD_ROOT%{_mandir}/man1/smbsh.1*
rm -f $RPM_BUILD_ROOT%{_mandir}/man1/smbget.1*
rm -f $RPM_BUILD_ROOT%{_mandir}/man5/smbgetrc.5*
rm -f $RPM_BUILD_ROOT/%{_mandir}/man8/mount.cifs.8*

%clean
rm -rf $RPM_BUILD_ROOT

%post
/sbin/chkconfig --add smb

%preun
if [ $1 = 0 ] ; then
    /sbin/chkconfig --del smb
    rm -rf /var/log/samba/* /var/cache/samba/*
    /sbin/service smb stop >/dev/null 2>&1
fi
exit 0

%postun
if [ "$1" -ge "1" ]; then
	%{initdir}/smb condrestart >/dev/null 2>&1
fi	


%post swat
# Add swat entry to /etc/services if not already there.
if [ ! "`grep ^\s**swat /etc/services`" ]; then
        echo 'swat        901/tcp     # Add swat service used via inetd' >> /etc/services
fi

%post common
/sbin/chkconfig --add winbind
/sbin/ldconfig

%preun common
if [ $1 = 0 ] ; then
    /sbin/chkconfig --del winbind
    /sbin/service winbind stop >/dev/null 2>&1
fi
exit 0

%postun common -p /sbin/ldconfig

%triggerpostun -- samba < 1.9.18p7
if [ $1 != 0 ]; then
    /sbin/chkconfig --add smb
fi

%triggerpostun -- samba < 2.0.5a-3
if [ $1 != 0 ]; then
    [ ! -d /var/lock/samba ] && mkdir -m 0755 /var/lock/samba
    [ ! -d /var/spool/samba ] && mkdir -m 1777 /var/spool/samba
    chmod 644 /etc/services
    [ -f /etc/inetd.conf ] && chmod 644 /etc/inetd.conf
fi

%files
%defattr(-,root,root)
%doc README COPYING Manifest 
%doc WHATSNEW.txt Roadmap
%doc docs
%doc examples/autofs examples/LDAP examples/libsmbclient examples/misc examples/printer-accounting
%doc examples/printing

%attr(755,root,root) /%{_lib}/security/pam_smbpass.so
%{_sbindir}/smbd
%{_sbindir}/nmbd
# %{_bindir}/make_unicodemap
%{_bindir}/mksmbpasswd.sh
%{_bindir}/smbcontrol
%{_bindir}/smbstatus
# %{_bindir}/smbadduser
%{_bindir}/tdbbackup
%{_bindir}/tdbtool
%config(noreplace) %{_sysconfdir}/sysconfig/samba
%config(noreplace) %{_sysconfdir}/samba/smbusers
%attr(755,root,root) %config %{initdir}/smb
%config(noreplace) %{_sysconfdir}/logrotate.d/samba
%config(noreplace) %{_sysconfdir}/pam.d/samba
# %{_mandir}/man1/make_unicodemap.1*
%{_mandir}/man1/smbcontrol.1*
%{_mandir}/man1/smbstatus.1*
%{_mandir}/man5/smbpasswd.5*
%{_mandir}/man7/samba.7*
%{_mandir}/man8/nmbd.8*
%{_mandir}/man8/pdbedit.8*
%{_mandir}/man8/smbd.8*
%{_mandir}/man8/pam_winbind.8*
%{_mandir}/man8/tdbbackup.8*
#%{_mandir}/ja/man1/smbstatus.1*
#%{_mandir}/ja/man5/smbpasswd.5*
#%{_mandir}/ja/man7/samba.7*
#%{_mandir}/ja/man8/smbd.8*
#%{_mandir}/ja/man8/nmbd.8*
%{_libdir}/samba/vfs

%attr(0700,root,root) %dir /var/log/samba
%attr(1777,root,root) %dir /var/spool/samba

%files swat
%defattr(-,root,root)
%config(noreplace) %{_sysconfdir}/xinetd.d/swat
%{_datadir}/swat
%{_sbindir}/swat
%{_mandir}/man8/swat.8*
#%{_mandir}/ja/man8/swat.8*
%attr(755,root,root) %{_libdir}/samba/*.msg

%files client
%defattr(-,root,root)
/sbin/mount.smb
/sbin/mount.smbfs
%{_libdir}/samba/lowcase.dat
%{_libdir}/samba/upcase.dat
%{_libdir}/samba/valid.dat
%{_bindir}/rpcclient
%{_bindir}/smbcacls
%{_bindir}/smbmount
%{_bindir}/smbmnt
%{_bindir}/smbumount
%{_bindir}/findsmb
%{_bindir}/tdbdump
%{_mandir}/man8/tdbdump.8*
%{_mandir}/man8/smbmnt.8*
%{_mandir}/man8/smbmount.8*
%{_mandir}/man8/smbumount.8*
%{_mandir}/man8/smbspool.8*
%{_bindir}/nmblookup
%{_bindir}/smbclient
%{_bindir}/smbprint
%{_bindir}/smbspool
%{_bindir}/smbtar
%{_bindir}/net
%{_bindir}/smbtree
%{_mandir}/man1/findsmb.1*
%{_mandir}/man1/nmblookup.1*
%{_mandir}/man1/rpcclient.1*
%{_mandir}/man1/smbcacls.1*
%{_mandir}/man1/smbclient.1*
%{_mandir}/man1/smbtar.1*
%{_mandir}/man1/smbtree.1*
%{_mandir}/man8/net.8*
#%{_mandir}/ja/man1/smbtar.1*
#%{_mandir}/ja/man1/smbclient.1*
#%{_mandir}/ja/man1/nmblookup.1*

%files common
%defattr(-,root,root)
/%{_lib}/libnss_wins.so*
/%{_lib}/libnss_winbind.so*
/%{_lib}/security/pam_winbind.so
%{_libdir}/libsmbclient.a
%{_libdir}/libsmbclient.so
%{_libdir}/samba/charset/CP*.so
%{_libdir}/samba/idmap/idmap*.so
%{_includedir}/libsmbclient.h
%{_bindir}/testparm
%{_bindir}/testprns
%{_bindir}/smbpasswd
# %{_bindir}/make_printerdef
%{_bindir}/wbinfo
# %{_bindir}/editreg
%{_bindir}/ntlm_auth
%{_bindir}/pdbedit
%{_bindir}/profiles
%{_bindir}/smbcquotas
#%{_bindir}/vfstest
%{_sbindir}/winbindd
%config(noreplace) %{_sysconfdir}/samba/smb.conf
%config(noreplace) %{_sysconfdir}/samba/lmhosts
%dir %{_datadir}/samba
%dir %{_datadir}/samba/codepages
%dir %{_sysconfdir}/samba
%{initdir}/winbind
# %{_datadir}/samba/codepages/*
# %{_mandir}/man1/make_smbcodepage.1*
%{_mandir}/man1/ntlm_auth.1*
%{_mandir}/man1/profiles.1*
%{_mandir}/man1/smbcquotas.1*
%{_mandir}/man1/testparm.1*
%{_mandir}/man1/testprns.1*
%{_mandir}/man5/smb.conf.5*
%{_mandir}/man5/lmhosts.5*
%{_mandir}/man8/smbpasswd.8*
%{_mandir}/man1/wbinfo.1*
%{_mandir}/man8/winbindd.8*
%{_mandir}/man1/vfstest.1*

# #%lang(ja) %{_mandir}/ja/man1/make_smbcodepage.1*
#%lang(ja) %{_mandir}/ja/man1/testparm.1*
#%lang(ja) %{_mandir}/ja/man1/testprns.1*
#%lang(ja) %{_mandir}/ja/man5/smb.conf.5*
#%lang(ja) %{_mandir}/ja/man5/lmhosts.5*
#%lang(ja) %{_mandir}/ja/man8/smbpasswd.8*

%changelog
* Fri Jan 16 2004 Gerald (Jerry) Carter <jerry@samba,org>
- Removed ChangeLog entries since they are kept in CVS



