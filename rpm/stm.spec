Name:		stm
Version:	1.0
Release: 	1%{?dist}
Summary: 	Simple Trash Manager

License:	GPL
URL:		https://github.com/Spybull/stm/
Source0:	%{name}-%{version}.tar.gz

BuildRequires: gcc, make, cmake
BuildRequires: pkgconfig(sqlcipher), pkgconfig(openssl)

Requires: sqlcipher, openssl

%description
Simple Trash Manager

%prep
%autosetup

%build
%cmake . -DCMAKE_BUILD_TYPE=Release
%cmake_build

%install
install -d -m 0700 %{buildroot}/var/lib/stm
%cmake_install

%pre
getent group stm >/dev/null || groupadd stm
getent passwd stm >/dev/null || useradd -g stm -d /var/lib/stm -M -s /sbin/nologin stm

%preun
if [ "$1" = 0 ]; then
	userdel stm  > /dev/null 2>&1 || :
	groupdel stm > /dev/null 2>&1 || :
fi

%files
%defattr(-,root,root)
%dir %attr(0700,stm,stm) /var/lib/stm
/usr/bin/stm

%changelog
* Sun Jun 29 2025 VB <spybull@proton.me> - 1.0-1
- Initial package