SYSTEMC ?= /home/bartho/greensocs/gspm/OSCI/systemc/2.2/gcc4.0
GREENMESSAGE ?= ../..

EXE = system
SRCS = system.cpp
OBJS = $(patsubst .cpp,.o,$(SRCS))

$(EXE): $(OBJS)
	g++ -g -o $@ $(OBJS) -I$(GREENMESSAGE) -I$(SYSTEMC)/include -L$(wildcard $(SYSTEMC)/lib-*) -lsystemc

clean::
	rm -f $(EXE)
distclean:: clean
	rm -f *~
