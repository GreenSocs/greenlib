GREENMESSAGE ?= ../../include
SYSTEMC ?= $(SYSTEMC_HOME)

EXE = system
SRCS = system.cpp
OBJS = $(patsubst .cpp,.o,$(SRCS))

$(EXE): $(OBJS)
	g++ -g -o $@ $(OBJS) -I$(GREENMESSAGE) -I$(SYSTEMC)/include -L$(wildcard $(SYSTEMC)/lib-*) -lsystemc

clean::
	rm -f $(EXE)
distclean:: clean
	rm -f *~
