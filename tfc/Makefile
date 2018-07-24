OUTPUT=../bin/tfc
OBJDIR=obj
SOURCE=tfc.cpp ftable.cpp grammap.cpp

OUTDIR=$(dir $(OUTPUT))
OBJSET=$(addprefix $(OBJDIR)/, $(addsuffix .o, $(basename $(SOURCE))))
DEPEND=$(OBJDIR)/.depend
CFLAGS=-std=c++11 -I ../ -I ../contrib

LCODES=

CXX=g++

all: $(OUTPUT)

$(OUTPUT): $(OBJSET) $(LCODES)
	@echo Linking $@...
	@$(CXX) -o $@ $(OBJSET) $(LCODES)

$(OBJDIR)/%.depend: %.cpp
	@echo Updating $@...
	@$(CXX) $(CFLAGS) -M -MT $(OBJDIR)/$(basename $<).o $< > $@
	@echo -e \\t'$$(CXX) $$(CFLAGS) -o $$@ -c $$<' >> $@

$(DEPEND): $(OBJDIR)
	@echo Updating dependencies...
	@echo include $(addprefix $(OBJDIR)/, $(addsuffix .depend, $(basename $(SOURCE)))) > $@

$(OBJDIR):
	@echo Creating directory $@...
	@mkdir -p $@

-include $(DEPEND)
