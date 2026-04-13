FROM quay.io/pypa/manylinux2014_x86_64:latest

RUN yum install -y cmake3 && ln -s /usr/bin/cmake3 /usr/bin/cmake

RUN curl -L https://github.com/Kitware/CMake/releases/download/v3.28.1/cmake-3.28.1-linux-x86_64.sh -o cmake-install.sh && \
    chmod +x cmake-install.sh && \
    ./cmake-install.sh --skip-license --prefix=/usr/local && \
    rm cmake-install.sh

RUN git clone https://github.com/pybind/pybind11.git && \
    cd pybind11 && mkdir build && cd build && \
    cmake .. -DPYBIND11_TEST=OFF -DCMAKE_INSTALL_PREFIX=/usr/local && \
    make install && \
    cd ../.. && rm -rf pybind11*

RUN git clone https://github.com/big-keva/moonycode && \
    cd moonycode && mkdir build && cd build && \
    cmake -B build -S ../ && cmake --build build -j$(nproc) && \
    cmake --install build --component Dev --prefix=/usr/local && \
    cd ../../ && rm -rf moonycode

RUN git clone https://github.com/big-keva/libmorph && \
    cd libmorph && git submodule init && git submodule update && mkdir build && cd build && \
    cmake -B build -S ../ && cmake --build build -j$(nproc) && \
    cmake --install build --component ApiDev --prefix=/usr/local && \
    cmake --install build --component RusDev --prefix=/usr/local && \
    cd ../../ && rm -rf libmorph

# Устанавливаем cibuildwheel для всех Python версий
RUN for pyver in /opt/python/cp*/bin/python; do \
        $pyver -m pip install --upgrade pip cibuildwheel build; \
    done

WORKDIR /project
