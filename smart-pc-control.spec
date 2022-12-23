%global srcname smart-pc-control

Summary:            Smart PC Control
Name:               smart-pc-control
Version:            0.1.3
Release:            1%{?dist}
License:            Apache v2
# Sources can be obtained by
# git clone https://github.com/orpiske/smart-pc-control
# cd gru
# tito build --tgz
Source0:            %{name}-%{version}.tar.gz
URL:                https://github.com/orpiske/smart-pc-control.git
BuildRequires:      cmake
BuildRequires:      gcc
BuildRequires:      gcc-c++
BuildRequires:      openssl-devel
BuildRequires:      rust-packaging


%description
A tool for controlling your PC via homekit2mqtt


%prep
%autosetup -n %{name}-%{version}


%build
%cargo_build -a


%install
install -D -m755 target/release/smart-pc-control %{buildroot}/%{_exec_prefix}/bin/smart-pc-control
install -D -m644 src/power_control/config/smart-pc-control-power.sh %{buildroot}/%{_sysconfdir}/sysconfig/smart-pc-control-power.sh
install -D -m644 src/power_control/config/smart-pc-control-power.service %{buildroot}/%{_prefix}/lib/systemd/system/smart-pc-control-power.service

%files
%doc README.md
%license LICENSE
%{_exec_prefix}/bin/smart-pc-control
%{_sysconfdir}/sysconfig/smart-pc-control-power.sh
%{_prefix}/lib/systemd/smart-pc-control-power.service


%if %{with check}
%check
%cargo_test -a
%endif

%changelog
* Fri Dec 23 2022 Otavio R. Piske <angusyoung@gmail.com> 0.1.3-1
- Added missing package for building (angusyoung@gmail.com)
- Remove the outdated debug setting (angusyoung@gmail.com)
- Formatting fixes in the packing spec (angusyoung@gmail.com)
- Fixed incorrect srcname in packaging (angusyoung@gmail.com)
- Updated cargo lock (angusyoung@gmail.com)
- Updated cargo lock (angusyoung@gmail.com)

* Thu Dec 22 2022 Otavio R. Piske <angusyoung@gmail.com> 0.1.2-1
- Do not turn off if running stateless (angusyoung@gmail.com)

* Thu Dec 22 2022 Otavio R. Piske <angusyoung@gmail.com> 0.1.1-1
- Fixed mismatch in the lock file (angusyoung@gmail.com)
- Use specific paths to prevent conflicts (angusyoung@gmail.com)

* Thu Dec 22 2022 Otavio R. Piske <angusyoung@gmail.com> 0.1.0-1
- SystemD unit must reload after install (angusyoung@gmail.com)
- SystemD unit must use exec for this binary (angusyoung@gmail.com)
- Updated documentation (angusyoung@gmail.com)
- Packaging fixes (angusyoung@gmail.com)
- Removed the old scripts used by the C version (angusyoung@gmail.com)
- Converted the project to Rust (angusyoung@gmail.com)
- Removed CMake build files and C source code (angusyoung@gmail.com)
- CMake 3 or greater is required for the build (angusyoung@gmail.com)
- Updated readme (angusyoung@gmail.com)

* Thu Dec 22 2022 Otavio R. Piske <angusyoung@gmail.com> 0.0.4-1
- Converted the project to Rust

* Sat Mar 05 2022 Otavio R. Piske <angusyoung@gmail.com> 0.0.3-1
- new package built with tito

* Sun Sep 26 2021 Otavio R. Piske <angusyoung@gmail.com> - 0.0.3-3
- Fixed overly verbose log message

* Fri Sep 24 2021 Otavio R. Piske <angusyoung@gmail.com> - 0.0.3-2
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