include Makefile.defs

MODULE = gsp_sc
SWIG_OPTIONS = -c++ -python #-naturalvar

####################
# No need to change after here
####################

all: _$(MODULE).so lib/libgreenscript.a


lib/libgreenscript.a: greenscript.o
	if test ! -d lib; then mkdir lib; fi
	ar -r $@ $<

_%.so: %.o %_wrap.o
	g++ -g -fPIC -shared $^ $(LIBS) -o $@

%.o: %.cpp %.h
	g++ -g -fPIC -c $< -o $@ $(DEFINES) $(INCLUDES)
%.o: %.cxx
	g++ -g -fPIC -c $< -o $@ $(DEFINES) $(INCLUDES)

%_wrap.cxx: %.i %.cpp %.h test_swig_version_1.3.31
	swig $(SWIG_OPTIONS) $(DEFINES) $(INCLUDES) $<
	#fix swig generated file so it supports redefinition of __setattr__ from C++
	sed -i -s "s/\.this/.__dict__[\"this\"]/" $*.py

clean:
	rm -f _$(MODULE).so $(MODULE).py* GreenScript.pyc
	rm -f $(MODULE)_wrap.o $(MODULE)_wrap.cxx $(MODULE).o
	rm -f greenscript.o
	rm -rf lib

distclean:
	rm -f *~ examples/*~ examples/test_gav_output.txt
	find . -name "*.pyc" | xargs rm -rf

test_swig_version_%:
	#test if swig version is greater then $*
	@SWIG_VERSION="$$(swig -version | sed -n -e 's/swig *version *:* *//ip')"; \
         if test "$$SWIG_VERSION" \< "$*"; then \
            echo "ERROR: Please update your SWIG to version greater then $*"; \
            false; \
         else \
            echo "SWIG version $$SWIG_VERSION"; \
         fi

#SECONDARY: $(MODULE)_wrap.cxx


##############################
# test the standalone examples
##############################

ALL_EXAMPLES = $(notdir $(wildcard examples/*.py))
GREENCONTROL_EXAMPLES = test_gs_param.py test_gav_output.py

ifeq ($(strip $(USE_GREENCONTROL)),yes)
EXAMPLES = $(ALL_EXAMPLES)
else
EXAMPLES = $(filter-out $(GREENCONTROL_EXAMPLES),$(ALL_EXAMPLES))
endif

test_standalone: all $(EXAMPLES)

%.py : FORCE_RUN
	@echo ""
	@echo "####################################"
	@echo "### $@"
	@export PYTHONPATH=$(CURDIR); cd examples; python$(PYTHON_VERSION) $@


############################
# test the embedded examples
############################

ALL_EMB_EXAMPLES = $(wildcard examples/*/Makefile)

test_embedded: all $(ALL_EMB_EXAMPLES)

examples/%/Makefile : FORCE_RUN
	@echo ""
	@echo "####################################"
	@echo "### $(@D)"
	@export PYTHONPATH=$(CURDIR); $(MAKE) test -C $(@D)


test: test_standalone test_embedded

FORCE_RUN:;


