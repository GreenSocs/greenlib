TARGET_ARCH = linux

CC     = g++
OPT    = -O3
DEBUG  = -g -Wall
OTHER  = -Wno-deprecated
CFLAGS = $(OPT) $(OTHER)
# CFLAGS = $(DEBUG) $(OTHER)


MODULE = pcie_speedtest
SRCS = example_generic_pcie.cpp PCIeRootComplex2.cpp Mem.cpp ddr_ram.cpp PCIeSendDevice3.cpp
OBJS = $(SRCS:.cpp=.o)

include ../../Makefile.gspm
include ../../Makefile.defs

all: $(EXE)

