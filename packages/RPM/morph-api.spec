Name:           libmorph-api
Version:        %{_version}
Release:        1%{?dist}
Summary:        libmorph libraries common headers
License:        MIT
Group:          Development/Libraries
URL:            https://github.com/big-keva/libmorph

%description
This package contains common libmorph library headers.

%install
# Очищаем директорию сборки
rm -rf %{buildroot}
# Создаем структуру каталогов внутри buildroot
mkdir -p %{buildroot}/usr
# Копируем ваш готовый "образ установки" (из каталога, где был make install)
# Предположим, вы передадите путь к нему через переменную
cp -a %{source_dir}/usr/include %{buildroot}/usr/ 2>/dev/null || :

%files
%defattr(-,root,root,-)
%{_includedir}/libmorph/*.h

%changelog
* Mon Mar 10 2025 Andrey Kovalenko <keva@rambler.ru>
- Initial build from pre-installed image
