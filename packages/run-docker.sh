PARENT_DIR=$(dirname $(pwd))

echo "$PARENT_DIR"

docker run --rm -v "$PARENT_DIR:"/libmorph -v $(pwd):/root/rpmbuild/RPMS -w /libmorph/packages builder-18.04 \
    bash -c "pwd && ls && make $1"
