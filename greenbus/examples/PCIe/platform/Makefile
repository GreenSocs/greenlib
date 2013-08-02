TARGET_ARCH = linux

CC     = g++
OPT    = -O3
DEBUG  = -g -Wall
OTHER  = -Wno-deprecated
CFLAGS = $(OPT) $(OTHER)
# CFLAGS = $(DEBUG) $(OTHER)


MODULE = example_pcie_platform
SRCS = example_pcie_platform.cpp MyPCIeRootComplex.cpp PCIeRecvInfoDevice.cpp PCIeRoutingTestSendDevice.cpp PCIeSendDevice.cpp
OBJS = $(SRCS:.cpp=.o)

include ../../Makefile.gspm
include ../../Makefile.defs

all: $(EXE)

