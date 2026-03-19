#
# build libraries
#
cmake -B build -S ../ && cmake --build build -j$(nproc)

#
# install all Components
#
cmake --install build --component RusRtm --prefix ./morphrus
cmake --install build --component ApiDev --prefix ./morphapi-dev
cmake --install build --component RusDev --prefix ./morphrus-dev

#
# read the versions
#
API_VER=$(cat "../API_VER")
RUS_VER=${API_VER}.$(cat "../lang-rus/dict/VERSION")

#
# create morphapi-dev
#
echo "building morphapi-dev..."
mkdir -p morphapi-dev/DEBIAN
cat DEBIAN/morphapi-dev.control | sed "s/@VERSION@/${API_VER}/g" > morphapi-dev/DEBIAN/control
dpkg-deb --build ./morphapi-dev "./morphapi-dev_${API_VER}_all.deb"

#
# create morphrus
#
echo "building morphrus..."
mkdir -p morphrus/DEBIAN
cat DEBIAN/morphrus.control | sed "s/@VERSION@/${RUS_VER}/g" > morphrus/DEBIAN/control
dpkg-deb --build ./morphrus "./morphrus_${RUS_VER}_amd64.deb"

#
# create morphapi-dev
#
echo "building morphrus-dev..."
mkdir -p morphrus-dev/DEBIAN
cat DEBIAN/morphrus-dev.control | sed "s/@VERSION@/${RUS_VER}/g" > morphrus-dev/DEBIAN/control
dpkg-deb --build ./morphrus-dev "./morphrus-dev_${RUS_VER}_amd64.deb"
