MAKEFILE_DEFS = $(firstword $(wildcard Makefile.defs ../Makefile.defs ../../Makefile.defs))
include $(MAKEFILE_DEFS)

GREENSCRIPT = ../..
LIBS += -L $(GREENSCRIPT)/lib -lgreenscript

system: system.cpp bidir.h
	g++ -g -Xlinker -export-dynamic system.cpp -o $@ $(DEFINES) $(INCLUDES) $(LIBS) -I$(GREENSCRIPT)

clean:
	rm -f system *~

test: clean system runtest

runtest:
	./system

