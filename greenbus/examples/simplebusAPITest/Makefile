TARGET_ARCH = macosx

CC     = g++
OPT    = -O3
DEBUG  = -g -Wall
OTHER  = -Wno-deprecated
CFLAGS = $(OPT) $(OTHER)
# CFLAGS = $(DEBUG) $(OTHER)


MODULE = example_simplebusAPI
SRCS = example_simplebusAPI.cpp ddr_ram.cpp simplebusAPImaster.cpp
OBJS = $(SRCS:.cpp=.o)

include ../Makefile.gspm
include ../Makefile.defs

all: $(EXE)

