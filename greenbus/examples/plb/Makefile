TARGET_ARCH = linux

CC     = g++
OPT    = -O3 -funroll-loops
DEBUG  = -g -Wall
OTHER  = -Wno-deprecated
CFLAGS = $(OPT) $(OTHER)
# CFLAGS = $(DEBUG) $(OTHER)


MODULE = example_plb
SRCS = example_plb.cpp
OBJS = $(SRCS:.cpp=.o)

include ../Makefile.gspm
include ../Makefile.defs

all: $(EXE)

