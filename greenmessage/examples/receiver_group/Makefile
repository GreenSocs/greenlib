SYSTEMC ?= /home/bartho/greensocs/gspm/OSCI/systemc/2.2/gcc4.0
GREENMESSAGE ?= ../..

EXE = system
SRCS = system.cpp sender.cpp receiver.cpp
OBJS = system.o sender.o receiver.o

$(EXE): $(OBJS)
	g++ -g -o $@ $(OBJS)  -L$(wildcard $(SYSTEMC)/lib-*) -lsystemc

system.o:  system.cpp
	g++ -c -o system.o system.cpp -I$(GREENMESSAGE) -I$(SYSTEMC)/include -I.

sender.o:  sender.cpp
	g++ -c -o sender.o sender.cpp -I$(GREENMESSAGE) -I$(SYSTEMC)/include -I.

receiver.o:  receiver.cpp
	g++ -c -o receiver.o receiver.cpp -I$(GREENMESSAGE) -I$(SYSTEMC)/include -I.

clean::
	rm -f $(EXE) *.o
distclean:: clean
	rm -f *~
