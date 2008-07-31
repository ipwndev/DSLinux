Summary: Monitor the progress of data through a pipe
Name: pv
Version: 1.1.4
Release: 1%{?dist}
License: Artistic 2.0
Group: Development/Tools
Source: http://www.ivarch.com/programs/sources/pv-1.1.4.tar.gz
Url: http://www.ivarch.com/programs/pv.shtml
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: gettext

%description
PV ("Pipe Viewer") is a tool for monitoring the progress of data through a
pipeline.  It can be inserted into any normal pipeline between two processes
to give a visual indication of how quickly data is passing through, how long
it has taken, how near to completion it is, and an estimate of how long it
will be until completion.

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}

%install
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf "$RPM_BUILD_ROOT"
mkdir -p "$RPM_BUILD_ROOT"%{_bindir}
mkdir -p "$RPM_BUILD_ROOT"%{_mandir}/man1
mkdir -p "$RPM_BUILD_ROOT"/usr/share/locale

make DESTDIR="$RPM_BUILD_ROOT" install
%find_lang %{name}

%check
make test

%clean
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf "$RPM_BUILD_ROOT"

%files -f %{name}.lang
%defattr(-, root, root)
%{_bindir}/%{name}
%{_mandir}/man1/%{name}.1.gz

%doc README doc/NEWS doc/TODO doc/COPYING

%changelog
* Thu Mar  6 2008 Andrew Wood <andrew.wood@ivarch.com> 1.1.4-1
- Trap SIGINT/SIGHUP/SIGTERM so we clean up IPCs on exit (Laszlo Ersek).
- Abort if numeric option, eg -L, has non-numeric value (Boris Lohner).
- Compilation fixes for Darwin 9 and OS X.

* Thu Aug 30 2007 Andrew Wood <andrew.wood@ivarch.com> 1.1.0-1
- New option "-R" to remotely control another pv process.
- New option "-l" to count lines instead of bytes.
- Performance improvement for "-L" (rate) option.
- Some Mac OS X fixes, and packaging cleanups.

* Sat Aug  4 2007 Andrew Wood <andrew.wood@ivarch.com> 1.0.1-1
- Changed license from Artistic to Artistic 2.0.
- Removed "--license" option.

* Thu Aug  2 2007 Andrew Wood <andrew.wood@ivarch.com> 1.0.0-1
- We now act more like "cat" - just skip unreadable files, don't abort.
- Various code cleanups were done.

* Mon Feb  5 2007 Andrew Wood <andrew.wood@ivarch.com> 0.9.9-1
- New option "-B" to set the buffer size, and a workaround for problems
- piping to dd(1).

* Mon Feb 27 2006 Andrew Wood <andrew.wood@ivarch.com>
- Minor bugfixes, and on the final update, blank out the now-zero ETA.

* Thu Sep  1 2005 Andrew Wood <andrew.wood@ivarch.com>
- Terminal locking now uses lockfiles if the terminal itself cannot be locked.

* Thu Jun 16 2005 Andrew Wood <andrew.wood@ivarch.com>
- A minor problem with the spec file was fixed.

* Mon Nov 15 2004 Andrew Wood <andrew.wood@ivarch.com>
- A minor bug in the NLS code was fixed.

* Sat Nov  6 2004 Andrew Wood <andrew.wood@ivarch.com>
- Code cleanups and minor usability fixes.

* Tue Jun 29 2004 Andrew Wood <andrew.wood@ivarch.com>
- A port of the terminal locking code to FreeBSD.

* Sun May  2 2004 Andrew Wood <andrew.wood@ivarch.com>
- Major reliability improvements to the cursor positioning.

* Sat Apr 24 2004 Andrew Wood <andrew.wood@ivarch.com>
- Rate and size parameters can now take suffixes such as "k", "m" etc.

* Mon Apr 19 2004 Andrew Wood <andrew.wood@ivarch.com>
- A bug in the cursor positioning was fixed.

* Thu Feb 12 2004 Andrew Wood <andrew.wood@ivarch.com>
- Code cleanups and portability fixes.

* Sun Feb  8 2004 Andrew Wood <andrew.wood@ivarch.com>
- The display buffer is now dynamically allocated, fixing an overflow bug.

* Wed Jan 14 2004 Andrew Wood <andrew.wood@ivarch.com>
- A minor bug triggered when installing the RPM was fixed.

* Mon Dec 22 2003 Andrew Wood <andrew.wood@ivarch.com>
- Fixed a minor bug that occasionally reported "resource unavailable".

* Wed Aug  6 2003 Andrew Wood <andrew.wood@ivarch.com>
- Block devices now have their size read correctly, so pv /dev/hda1 works
- Minor code cleanups (mainly removal of CVS "Id" tags)

* Sun Aug  3 2003 Andrew Wood <andrew.wood@ivarch.com>
- Doing ^Z then "bg" then "fg" now continues displaying

* Tue Jul 16 2002 Andrew Wood <andrew.wood@ivarch.com>
- First draft of spec file created.
