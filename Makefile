.PHONY: all

SRCDIR=src
OUTDIR=out
DEPDIR=dep

CXX=g++
CXXFLAGS= -g -I $(SRCDIR) -I $(OUTDIR) --std=c++14
CXXWFLAGS= -Wall -Wextra $(CXXFLAGS)
SRC=$(wildcard $(SRCDIR)/*.cpp)
OBJ=$(patsubst $(SRCDIR)/%.cpp,$(OUTDIR)/%.o,$(SRC))
DEPF = $(wildcard $(DEPDIR)/*.d)

all: SMT

SMT: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@


$(OUTDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OUTDIR)
	@mkdir -p $(DEPDIR)
	@echo Compiling $<
	@$(CXX) $(CXXWFLAGS) -MMD -MT '$@' -MF	$(patsubst $(SRCDIR)/%.cpp, $(DEPDIR)/%.d, $<) -c $< -o $@

clean:
	rm -rf $(OUTDIR)
	rm -rf $(DEPDIR)
	rm -f SMT

include $(DEPF)
