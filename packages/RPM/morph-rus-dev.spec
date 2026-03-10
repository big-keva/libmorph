Name:           libmorph-rus-dev
Version:        %{_version}
Release:        1%{?dist}
Summary:        libmorphrus static library and header
License:        MIT
Group:          Development/Libraries
URL:            https://github.com/big-keva/libmorph

# В RPM зависимости разделяются пробелами, запятые не нужны
Requires:       glibc >= 2.17 libmorph-api

%description
This package contains libmorphrus static library and header.

%install
# Очищаем директорию сборки
rm -rf %{buildroot}
# Создаем структуру каталогов внутри buildroot
mkdir -p %{buildroot}/usr
# Копируем ваш готовый "образ установки" (из каталога, где был make install)
# Предположим, вы передадите путь к нему через переменную
cp -a %{source_dir}/usr/include %{buildroot}/usr/ 2>/dev/null || :
cp -a %{source_dir}/usr/* %{buildroot}/usr/ 2>/dev/null || :

%files
%defattr(-,root,root,-)
%{_includedir}/libmorph/*.h
%{_libdir}/*

%changelog
* Mon Mar 10 2025 Andrey Kovalenko <keva@rambler.ru>
- Initial build from pre-installed image
