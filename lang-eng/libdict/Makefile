MRROOT=../../

STATIC_SRC=$(wildcard *.cpp) $(wildcard *.h)
SHARED_SRC=$(STATIC_SRC)

ifdef _DEMO
	DMNAME=_demo
endif

ifdef _PROFILE
	PRSUFF=_p
endif

STATIC_DBG=$(MRROOT)lib/libmorpheng$(DMNAME)$(PRSUFF)_d.a
STATIC_REL=$(MRROOT)lib/libmorpheng$(DMNAME)$(PRSUFF).a
SHARED_DBG=$(MRROOT)lib/libmorpheng$(DMNAME)$(PRSUFF)_d.so
SHARED_REL=$(MRROOT)lib/libmorpheng$(DMNAME)$(PRSUFF).so

all: $(STATIC_DBG) $(STATIC_REL) $(SHARED_DBG) $(SHARED_REL)

clean_all: clean
	@rm -f $(STATIC_DBG) $(STATIC_REL) $(SHARED_DBG) $(SHARED_REL)

clean:
	@$(MAKE) -f libmorpheng_static.mak _DEBUG=1
	@$(MAKE) -f libmorpheng_static.mak
	@$(MAKE) -f libmorpheng_shared.mak _DEBUG=1
	@$(MAKE) -f libmorpheng_shared.mak

$(STATIC_DBG): $(STATIC_SRC)
	@echo Building static debug library '$(STATIC_DBG)'
	@$(MAKE) -f libmorpheng_static.mak _DEBUG=1

$(STATIC_REL): $(STATIC_SRC)
	@echo Building static library '$(STATIC_REL)'
	@$(MAKE) -f libmorpheng_static.mak

$(SHARED_DBG): $(SHARED_SRC)
	@echo Building shared debug library '$(SHARED_DBG)'
	@$(MAKE) -f libmorpheng_shared.mak _DEBUG=1

$(SHARED_REL): $(SHARED_SRC)
	@echo Building shared library '$(SHARED_REL)'
	@$(MAKE) -f libmorpheng_shared.mak

