TARGET_ARCH = linux64

CC     = g++
OPT    = -O3 
#OPT  = -g -Wall
#OTHER  = -Wno-deprecated
CFLAGS = $(OPT) $(OTHER)
# CFLAGS = $(DEBUG) $(OTHER)
#	EXTRA_LIBS = -lscv -ldust

MODULE = ddr_speedtest
SRCS = speedtest_top.cpp ddr_ram.cpp pseudo_master_simplebus.cpp ddr_speedtest.cpp
OBJS = $(SRCS:.cpp=.o)

include ../Makefile.gspm
include ../Makefile.defs

all: $(EXE)

