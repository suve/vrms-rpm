Name:          vrms-rpm
Version:       1.0
Release:       1  
Summary:       Report nonfree software
BuildArch:     noarch
License:       GPLv3
URL:           https://github.com/suve/vrms-rpm/

# Archive generated from git repository
# No modifications made
Source0:       %{name}-%{version}.zip

%description
vrms-rpm reports nonfree packages installed on the system.

%prep
%setup
%build

%install
install -m 755 -d %{buildroot}/%{_bindir}/
install -m 755 -d %{buildroot}/%{_mandir}/man1/

install -m 755 %{_builddir}/%{name}-%{version}/vrms-rpm.sh %{buildroot}%{_bindir}/vrms-rpm
install -m 644 %{_builddir}/%{name}-%{version}/vrms-rpm.man %{buildroot}%{_mandir}/man1/vrms-rpm.1

%files
%{_bindir}/vrms-rpm
%{_mandir}/man1/vrms-rpm.1.gz
