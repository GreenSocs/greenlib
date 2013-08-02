TARGET_ARCH = linux

CC     = g++
OPT    = -O3
DEBUG  = -g -Wall
OTHER  = -Wno-deprecated
CFLAGS = $(OPT) $(OTHER)
# CFLAGS = $(DEBUG) $(OTHER)


MODULE = pcie_speedtest
SRCS = example_mixed.cpp PCIeMaDevice.cpp PCIeRootComplex2.cpp Mem.cpp GenericMaDevice.cpp ddr_ram.cpp PCIeRecvDevice2.cpp
OBJS = $(SRCS:.cpp=.o)

include ../../Makefile.gspm
include ../../Makefile.defs

all: $(EXE)

