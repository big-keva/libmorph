Name:           libmorph-rus
Version:        %{_version}
Release:        1%{?dist}
Summary:        libmorph libraries common headers
License:        MIT
Group:          Development/Libraries
URL:            https://github.com/big-keva/libmorph

%description
This package contains libmorphrus shared runtime library.

%install
# Очищаем директорию сборки
rm -rf %{buildroot}
# Создаем структуру каталогов внутри buildroot
mkdir -p %{buildroot}/usr
cp -a %{source_dir}/usr/* %{buildroot}/usr/ 2>/dev/null || :

%files
%defattr(-,root,root,-)
%{_libdir}/*

%changelog
* Mon Mar 10 2025 Andrey Kovalenko <keva@rambler.ru>
- Initial build from pre-installed image
