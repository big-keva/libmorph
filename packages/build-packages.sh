#
# build libraries
#
cmake -B build -S ../ && cmake --build build -j$(nproc)

#
# install all Components
#
cmake --install build --component ApiDev --prefix ./morphapi-dev/usr
cmake --install build --component RusRtm --prefix ./morphrus/usr
cmake --install build --component RusDev --prefix ./morphrus-dev/usr

cmake --install build --component EngRtm --prefix ./morpheng/usr
cmake --install build --component EngDev --prefix ./morpheng-dev/usr

#
# read the versions
#
API_VER=$(cat "../API_VER")
RUS_VER=${API_VER}.$(cat "../lang-rus/dict/VERSION")
ENG_VER=${API_VER}.$(cat "../lang-eng/dict/VERSION")

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
# create morphrus-dev
#
echo "building morphrus-dev..."
mkdir -p morphrus-dev/DEBIAN
cat DEBIAN/morphrus-dev.control | sed "s/@VERSION@/${RUS_VER}/g" > morphrus-dev/DEBIAN/control
dpkg-deb --build ./morphrus-dev "./morphrus-dev_${RUS_VER}_amd64.deb"

#
# create morpheng
#
echo "building morpheng..."
mkdir -p morpheng/DEBIAN
cat DEBIAN/morpheng.control | sed "s/@VERSION@/${ENG_VER}/g" > morpheng/DEBIAN/control
dpkg-deb --build ./morpheng "./morpheng_${ENG_VER}_amd64.deb"

#
# create morphrus-dev
#
echo "building morpheng-dev..."
mkdir -p morpheng-dev/DEBIAN
cat DEBIAN/morpheng-dev.control | sed "s/@VERSION@/${ENG_VER}/g" > morpheng-dev/DEBIAN/control
dpkg-deb --build ./morpheng-dev "./morpheng-dev_${ENG_VER}_amd64.deb"
