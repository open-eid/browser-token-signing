Name: esteidfirefoxplugin		
Version: 3.3
Release: 1%{?dist}
Summary: EstEID Firefox plugin	
Group: Applications/Internet		
License: LGPLv2+
URL: http://www.ria.ee		
Source0: esteidfirefoxplugin.tar.gz	
BuildRoot: %{_tmppath}/-%{version}-%{release}-root-%(%{__id_u} -n)

%if 0%{?fedora}
BuildRequires: make, gtk2-devel, openssl-devel, gcc, openssl-devel, gcc-c++
Requires: opensc, pcsc-lite
%else
BuildRequires: make, gtk2-devel, openssl-devel, gcc, libopenssl-devel, gcc-c++
Requires: opensc, pcsc-lite
%endif

%if %{defined suse_version}
%debug_package
%endif

%description
EstEID Firefox plugin	

%prep
%setup -q -n %{name}

%build
make plugin

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}
install -d %{buildroot}/%{_libdir}/mozilla/plugins
mv %{_builddir}/%{name}/npesteid-firefox-plugin.so %{buildroot}/%{_libdir}/mozilla/plugins

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%doc
%{_libdir}/mozilla/plugins/npesteid-firefox-plugin.so

%changelog
* Fri Aug 13 2010 RIA <info@ria.ee> 1.0-1
- first build no changes
