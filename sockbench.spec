# https://fedoraproject.org/wiki/How_to_create_an_RPM_package
# Built and maintained by John Boero - boeroboy@gmail.com
# In honor of Seth Vidal https://www.redhat.com/it/blog/thank-you-seth-vidal

Name:           sockbench
Version:        1.0
Release:        1%{?dist}
Summary:        Basic UDS/TCP socket benchmarking tool.
License:        Apache
# Our engineering uses "amd64" instead of "x86_64" so ugly mapping...
Source0:        https://github.com/jboero/sockbench/archive/refs/heads/main.tar.gz

# Some builds fail on systemd, but hey, systemd right? 👍
BuildRequires: systemd unzip make tree
Requires(pre): shadow-utils
Requires(post):	systemd libcap
URL:		https://github.com/jboero/sockperf

%define debug_package %{nil}

%description
Terraform provisions multicloud resources using a common language of HCL.

%prep
%autosetup -c %{name}-%{version}

%build
tree
cd sockbench-main
make

%install
mkdir -p %{buildroot}%{_bindir}/
cp -p %{name} %{buildroot}%{_bindir}/

%clean
rm -rf %{buildroot}
rm -rf %{_builddir}/*

%files
%{_bindir}/%{name}

%pre

%post

%preun

%postun

%changelog
