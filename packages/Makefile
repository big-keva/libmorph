PACKS_DIR = $(abspath $(CURDIR))
MORPH_DIR = $(abspath $(CURDIR)/..)

API_VERSION := $(shell cat $(MORPH_DIR)/API_VERSION)
MORPHRUS_DICT_VER := $(shell cat $(MORPH_DIR)/lang-rus/dict/VERSION)
MORPHRUS_CODE_VER := $(shell cat $(MORPH_DIR)/lang-rus/libdict/VERSION)

CMAKE = cmake

.PHONY: all morphapi morphrus morphrus-dev clean

all: morphapi morphrus morphrus-dev

morph-api:
	@mkdir -p build/$@ && mkdir -p $@/DEBIAN
	@cd build/$@ && \
		$(CMAKE) $(MORPH_DIR) -DBUILD_MORPHAPI:BOOL=ON -DCMAKE_INSTALL_PREFIX=../../$@/usr && \
		$(CMAKE) --build . --target install
	@cat DEBIAN/$@.control | sed "s/@VERSION@/${API_VERSION}/g" > $@/DEBIAN/control
	@echo "Building DEB..."
	@dpkg-deb --build $@ lib$@_$(API_VERSION)_all.deb
	@echo Building RPM...
	@rpmbuild -bb RPM/$@.spec --define "_version $(API_VERSION)" --define "source_dir $(PACKS_DIR)/$@"

morph-rus:
	@mkdir -p build/$@ && mkdir -p $@/DEBIAN
	@cd build/$@ && \
		$(CMAKE) $(MORPH_DIR) -DBUILD_MORPHRUS:BOOL=ON -DCMAKE_INSTALL_PREFIX=../../$@/usr && \
		$(CMAKE) --build . --target install
	@cat DEBIAN/$@.control | sed "s/@VERSION@/$(API_VERSION).$(MORPHRUS_DICT_VER).$(MORPHRUS_CODE_VER)/g" > $@/DEBIAN/control
	@echo "Building DEB..."
	@dpkg-deb --build $@ lib$@_$(API_VERSION).$(MORPHRUS_DICT_VER).$(MORPHRUS_CODE_VER)_amd64.deb
	@echo Building RPM...
	@mv $@/usr/lib $@/usr/lib64
	@rpmbuild -bb RPM/$@.spec --define "_version $(API_VERSION).$(MORPHRUS_DICT_VER).$(MORPHRUS_CODE_VER)" --define "source_dir $(PACKS_DIR)/$@"

morph-rus-dev:
	@mkdir -p build/$@ && mkdir -p $@/DEBIAN
	@cd build/$@ && \
		$(CMAKE) $(MORPH_DIR) -DBUILD_MORPHRUS:BOOL=ON -DDEV:BOOL=ON -DCMAKE_INSTALL_PREFIX=../../$@/usr && \
		$(CMAKE) --build . --target install
	@cat DEBIAN/$@.control | sed "s/@VERSION@/$(API_VERSION).$(MORPHRUS_DICT_VER).$(MORPHRUS_CODE_VER)/g" > $@/DEBIAN/control
	@dpkg-deb --build $@ lib$@_$(API_VERSION).$(MORPHRUS_DICT_VER).$(MORPHRUS_CODE_VER)_amd64.deb
	@mv $@/usr/lib $@/usr/lib64
	@rpmbuild -bb RPM/$@.spec --define "_version $(API_VERSION).$(MORPHRUS_DICT_VER).$(MORPHRUS_CODE_VER)" --define "source_dir $(PACKS_DIR)/$@"
