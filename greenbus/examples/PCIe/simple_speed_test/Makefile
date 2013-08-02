TARGET_ARCH = linux

CC     = g++
OPT    = -O3
DEBUG  = -g -Wall
OTHER  = -Wno-deprecated
CFLAGS = $(OPT) $(OTHER)
# CFLAGS = $(DEBUG) $(OTHER)


MODULE = pcie_speedtest
SRCS = example_pcie.cpp MySimplePCIeRootComplex.cpp PCIeRecvDevice.cpp PCIeSendDevice.cpp
OBJS = $(SRCS:.cpp=.o)

include ../../Makefile.gspm
include ../../Makefile.defs

all: $(EXE)

