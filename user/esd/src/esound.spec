# Note that this is NOT a relocatable package
Summary:          Allows several audio streams to play on a single audio device.
Name:             esound
Version:          0.2.38
Release:          1
License:          GPL
Group:            System Environment/Daemons
Source:           esound-%{version}.tar.gz
BuildRoot:        %{_tmppath}/%{name}-%{version}-root
Requires:         audiofile >= 0.2.3
BuildRequires:    audiofile-devel >= 0.2.3

%description
EsounD, the Enlightened Sound Daemon, is a server process that mixes
several audio streams for playback by a single audio device. For
example, if you're listening to music on a CD and you receive a
sound-related event from ICQ, the two applications won't have to
queue for the use of your sound card.

Install esound if you'd like to let sound applications share your
audio device. You'll also need to install the audiofile package.

%package devel
Summary:          Development files for EsounD applications.
Group:            Development/Libraries
Requires:         esound = %{version}
Requires:         audiofile-devel >= 0.2.3
BuildRequires:    audiofile-devel >= 0.2.3


%description devel
The esound-devel package includes the libraries, include files and
other resources needed to develop EsounD applications.

%prep
%setup -q

%build
%configure

make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc AUTHORS COPYING ChangeLog docs/esound.sgml docs/html docs/esound.ps
%doc INSTALL NEWS README TIPS TODO
%config(noreplace) /etc/*
%{_bindir}/*
%{_libdir}/*.so.*

%files devel
%defattr(-, root, root)
%{_libdir}/*a
%{_libdir}/*.so
%{_includedir}/*
/usr/share/aclocal/*
%{_libdir}/pkgconfig/*

%changelog
* Fri Mar 15 2002 Chris Chabot <chabotc@reviewboard.com>
- Put .pc file in file list
- Added audiofile to requires / build requires
- Cleaned up formatting

* Wed Jan 09 2002 Tim Powers <timp@redhat.com>
- automated rebuild

* Sun Aug 26 2001 Elliot Lee <sopwith@redhat.com> 0.2.22-5
- Remove useless URL: (#48441)

* Fri Jul 13 2001 Alexander Larsson <alexl@redhat.com>
- Add nohang patch that fixes "starting esd hangs for 10 seconds".

* Fri Jul  6 2001 Trond Eivind Glomsrød <teg@redhat.com>
- Use %%{_tmppath}
- Add BuildRequires
- Don't strip explicitly
- Make the esound-devel depend on esound with version
- s/Copyright/License/
- it isn't relocatable, don't pretend it is
- make /etc/esd.conf noreplace

* Sun Jun 24 2001 Elliot Lee <sopwith@redhat.com>
- Bump release + rebuild.

* Tue Nov 30 2000  Elliot Lee <sopwith@redhat.com> 0.2.22-1
- Update to 0.2.22

* Tue Oct 3 2000  Elliot Lee <sopwith@redhat.com> 0.2.20-1
- Update to 0.2.20

* Fri Aug 11 2000 Jonathan Blandford <jrb@redhat.com>
- Up Epoch and release

* Wed Jul 19 2000 Havoc Pennington <hp@redhat.com>
- Remove error spew when /dev/dsp is absent.

* Tue Jul 18 2000 Elliot Lee <sopwith@redhat.com> 0.2.19-1
- New version

* Wed Jul 12 2000 Prospector <bugzilla@redhat.com>
- automatic rebuild

* Mon Jul 10 2000 Elliot Lee <sopwith@redhat.com> 0.2.18-4
- Pass a prefix of /usr to configure, NOT %prefix

* Thu Jun 29 2000 Dave Mason <dcm@redhat.com> 0.2.18-3
- fixed Doc Dir

* Sat Jun  3 2000 BIll Nottingham <notting@redhat.com> 0.2.18-2
- rebuild. Apparently the compiler ate this last time.

* Tue Apr 4 2000 Elliot Lee <sopwith@redhat.com> 0.2.18-1
- Update to 0.2.18

* Mon Aug 30 1999 Elliot Lee <sopwith@redhat.com> 0.2.13-1
- Update to 0.2.13
- Merge in changes from RHL 6.0 spec file.

* Sat Nov 21 1998 Pablo Saratxaga <srtxg@chanae.alphanet.ch>
- added /usr/share/aclocal/* to %files devel
- added spanish and french translations for rpm

* Thu Oct 1 1998 Ricdude <ericmit@ix.netcom.com>
- make autoconf do the version updating for us.

* Wed May 13 1998 Michael Fulbright <msf@redhat.com>
- First try at an RPM
