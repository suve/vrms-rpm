Name:          vrms-rpm
Version:       1.0
Release:       4%{dist}  
Summary:       Report non-free software
BuildArch:     noarch
Requires:      bash, grep
License:       GPLv3

%global githubowner suve
%global gittag0 release-%{version}
URL:           https://github.com/%{githubowner}/%{name}/
Source0:       https://github.com/%{githubowner}/%{name}/archive/%{gittag0}.tar.gz


%description
vrms-rpm ("virtual Richard M. Stallman") reports non-free packages 
installed on the system.

%prep
%autosetup -n %{name}-%{gittag0} 

%build

%install
install -m 755 -d %{buildroot}/%{_bindir}/
install -m 755 -d %{buildroot}/%{_mandir}/man1/

install -m 755 ./vrms-rpm.sh %{buildroot}%{_bindir}/vrms-rpm
install -m 644 ./vrms-rpm.man %{buildroot}%{_mandir}/man1/vrms-rpm.1

%files
%{_bindir}/vrms-rpm
%{_mandir}/man1/vrms-rpm.1*
%license LICENCE.txt

%changelog
* Thu Mar 23 2017 suve <veg@svgames.pl> 1.0-4
- Add grep as a dependency

* Wed Mar 22 2017 suve <veg@svgames.pl> 1.0-3
- Use the GitHub archive link for Source0
- Do not use the _builddir variable during install section
- Use wildcard for the manpage in files section
- Include licence in files section
- Add bash as a dependency

