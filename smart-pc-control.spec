# Debug info package generation currently breaks the RPM build
%global _enable_debug_package 0
%global debug_package %{nil}

Summary:            Smart PC Control
Name:               smart-pc-control
Version:            0.0.4
Release:            1%{?dist}
License:            Apache v2
# Sources can be obtained by
# git clone https://github.com/orpiske/smart-pc-control
# cd gru
# tito build --tgz
Source0:             smart-pc-control-%{version}.tar.gz
URL:                https://github.com/orpiske/smart-pc-control.git
BuildRequires:      cmake
BuildRequires:      make
BuildRequires:      gcc >= 4.8.0
BuildRequires:      gcc-c++
BuildRequires:      paho-c-devel
BuildRequires:      libuuid-devel
BuildRequires:      zlib-devel
BuildRequires:      uriparser-devel
BuildRequires:      gru-devel
BuildRequires:      json-c-devel
Requires:           jq
Requires:           lm_sensors

%description
A tool for controlling your PC via homekit2mqtt

%prep
%autosetup -n smart-pc-control-%{version}

%build
mkdir build
%cmake -DCMAKE_USER_C_FLAGS="-fPIC" -S . -B build
cd build && %make_build

%install
cd build
%make_install

%files
%doc README.md
%license LICENSE
%{_bindir}/*
%{_sysconfdir}/*
%{_prefix}/lib/systemd/*
%{_libexecdir}/*

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%changelog
* Sat Mar 05 2022 Otavio R. Piske <angusyoung@gmail.com> 0.0.4-1
- 

* Sat Mar 05 2022 Otavio R. Piske <angusyoung@gmail.com> 0.0.3-1
- new package built with tito

* Sun Sep 26 2024 Otavio R. Piske <angusyoung@gmail.com> - 0.0.3-3
- Fixed overly verbose log message

* Fri Sep 24 2024 Otavio R. Piske <angusyoung@gmail.com> - 0.0.3-2
- Adjusted package to build on Fedora 33, 34, 34 and rawhide

* Sun Nov 10 2019 Otavio R. Piske <angusyoung@gmail.com> - 0.0.3-1
- Added support for installing the scripts

* Sun Nov 10 2019 Otavio R. Piske <angusyoung@gmail.com> - 0.0.2-3
- Added missing dependency: lm_sensors
- Fixed memory handling issues
- Fixed return status handling issues

* Sat Nov 09 2019 Otavio R. Piske <angusyoung@gmail.com> - 0.0.2-2
- Added missing dependency: jq

* Sat Nov 09 2019 Otavio R. Piske <angusyoung@gmail.com> - 0.0.2-1
- Added temperature daemon

* Sun Nov 03 2019 Otavio R. Piske <angusyoung@gmail.com> - 0.0.1-2
- Fixes

* Sun Nov 03 2019 Otavio R. Piske <angusyoung@gmail.com> - 0.0.1-1
- Initial release