docker run --rm \
    -v "$(pwd)/..":/builder \
    -w /builder/packages \
    builder-18.04 \
    bash -c "
        dpkg -i ./moonycode*.deb || apt-get install -f -y && \
        chmod +x build-packages.sh && ./build-packages.sh && \
        chown -R $(id -u):$(id -g) ."
