Name:          vrms-rpm
Version:       1.1
Release:       3%{?dist}
Summary:       Report non-free software
BuildArch:     noarch
Requires:      bash, grep
License:       GPLv3

%global githubowner suve
%global gittag0 release-%{version}
URL:           https://github.com/%{githubowner}/%{name}/
Source0:       %{url}archive/%{gittag0}.tar.gz#/%{name}-%{gittag0}.tar.gz


%description
vrms-rpm ("virtual Richard M. Stallman") reports non-free packages 
installed on the system.

%prep
%autosetup -n %{name}-%{gittag0} 

%build
# Nothing to do, this is a bash script.
# Omitting the build section causes rpmlint to complain.

%install
install -m 755 -d %{buildroot}/%{_bindir}/
install -m 755 -d %{buildroot}/%{_mandir}/man1/

install -m 755 -p ./vrms-rpm.sh %{buildroot}%{_bindir}/vrms-rpm
install -m 644 -p ./vrms-rpm.man %{buildroot}%{_mandir}/man1/vrms-rpm.1

%files
%{_bindir}/vrms-rpm
%{_mandir}/man1/vrms-rpm.1*
%license LICENCE.txt

%changelog
* Sat May 20 2017 suve <veg@svgames.pl> 1.1-3
- Use "{?dist}" instead of "{dist}" in release number

* Sat Apr 15 2017 suve <veg@svgames.pl> 1.1-2
- Use the -p option (preserve timestamps) with install

* Fri Apr 07 2017 suve <veg@svgames.pl> 1.1-1
- Change version number to match upstream

* Fri Apr 07 2017 suve <veg@svgames.pl> 1.0-5
- Use URL variable when defining Source0
- Use #/ in Source0 to request a better archive name from GitHub

* Thu Mar 23 2017 suve <veg@svgames.pl> 1.0-4
- Add grep as a dependency

* Wed Mar 22 2017 suve <veg@svgames.pl> 1.0-3
- Use the GitHub archive link for Source0
- Do not use the _builddir variable during install section
- Use wildcard for the manpage in files section
- Include licence in files section
- Add bash as a dependency

* Sun Mar 19 2017 suve <veg@svgames.pl> 1.0-2
- Initial packaging

