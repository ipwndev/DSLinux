Name: hnb
Version: 1.9.17
Release: 1
Summary: HNB - Hierarchical Notebook
License: GPL
Group: Applications/Productivity
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Packager: Asgeir Nilsen <rpm@asgeirnilsen.com>

%description
hnb is a curses program to structure many kinds of data in one place,
for example addresses, to-do lists, ideas, book reviews or to store
snippets of brainstorming. Writing structured documents and speech
outlines.

The default format is XML but hnb can also export to ASCII and
HTML. External programs may be used for more advanced conversions of
the XML data.

%prep
%setup -q

%build
%make

%install
rm -fr $RPM_BUILD_ROOT
install -D src/hnb $RPM_BUILD_ROOT%{_bindir}/hnb
install -D -m444 doc/hnb.1 $RPM_BUILD_ROOT%{_mandir}/man1/hnb.1

%clean
rm -fr $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc README COPYING
%{_bindir}/hnb
%{_mandir}/man1/hnb.1*

%changelog
