OUTPUT=classmap.cpp flexTree.cpp lidstree.cpp mixTypes.cpp mxTables.cpp stemtree.cpp
DICSRC=$(addprefix ../dict/, rusverbs.dic namecity.dic masculin.dic feminine.dic middle.dic adjects.dic pronadjs.dic pronouns.dic adjnouns.dic !4censor.dic)

ifeq ("$(ProjectRoot)", "")
  ProjectRoot=.
endif

ifeq ("$(TMPDIR)", "")
  TMPDIR=$(ProjectRoot)/tmp
endif

FX_BIN=$(TMPDIR)/flextabs.btf
FX_IDX=$(TMPDIR)/flextabs.sym
FX_SRC=../tables/flextabs.tf $(wildcard ../tables/flex/*.tf ../tables/flex/*/*.tf)
FXTABS=$(FX_BIN) $(FX_IDX)

MX_BIN=$(TMPDIR)/intrtabs.bmx
MX_IDX=$(TMPDIR)/intrtabs.sym
MX_SRC=../tables/mtables.mx $(wildcard ../tables/mix/*.mx)
MXTABS=$(MX_BIN) $(MX_IDX)

MKDICT=$(ProjectRoot)/bin/makerus$(_EXE)
MKFLEX=$(ProjectRoot)/bin/tfc$(_EXE)
MKINTR=$(ProjectRoot)/bin/makeich$(_EXE)

UNKNWN=-unknown=./unknowns.txt
NSPACE=-namespace=__libmorphrus__
USECPS=-codepage=dos

all: $(OUTPUT)

$(OUTPUT): $(DICSRC) $(FXTABS) $(MXTABS) $(MKDICT)
	@echo Building dictionaries $@
	$(MKDICT) -flex-table=$(FX_BIN) -flex-index=$(FX_IDX) -intr-table=$(MX_BIN) -intr-index=$(MX_IDX) -target-dir=./ $(UNKNWN) $(NSPACE) $(USECPS) $(DICSRC)

$(FXTABS): $(FX_SRC) $(MKFLEX)
	@echo Building inflexion tables '$@'
	@$(MKFLEX) -lang:rus $< $(FXTABS)

$(MXTABS): $(MX_SRC) $(MKINTR)
	@echo Building interchanges tables '$@'
	@$(MKINTR) $< $(MXTABS)

$(MKDICT):
	@$(error $@ not found, it would be built before)

$(MKFLEX):
	@$(error $@ not found, it would be built before)

$(MKINTR):
	@$(error $@ not found, it would be built before)

