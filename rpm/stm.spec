Name:		stm
Version:	1.0
Release: 	3%{?dist}
Summary: 	Simple Trash Manager

License:	GPL
URL:		https://github.com/Spybull/stm/
Source0:	%{name}-%{version}.tar.gz

BuildRequires: gcc, make, cmake
BuildRequires: pkgconfig(sqlcipher), pkgconfig(openssl), pkgconfig(jansson), pkgconfig(libssh)

Requires: sqlcipher, openssl, jansson, libssh

%description
Simple Trash Manager

%prep
%autosetup

%build
%cmake . -DCMAKE_BUILD_TYPE=Release
%cmake_build

%install
%cmake_install

%files
%defattr(-,root,root)
/usr/bin/stm

%changelog
* Wed Jul 9 2025 VB <spybull@proton.me> - 1.0-3
- Remove system directories for credential daemon

* Tue Jul 1 2025 VB <spybull@proton.me> - 1.0-2
- Add jansson library

* Sun Jun 29 2025 VB <spybull@proton.me> - 1.0-1
- Initial package