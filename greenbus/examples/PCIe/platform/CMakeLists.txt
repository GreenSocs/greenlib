SET(EXECUTABLE "platform")

ADD_EXECUTABLE(${EXECUTABLE}
	example_pcie_platform.cpp
	MyPCIeRootComplex.cpp
	PCIeRecvInfoDevice.cpp
	PCIeRoutingTestSendDevice.cpp
	PCIeSendDevice.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND ./${EXECUTABLE})

add_dependencies(compile_test ${EXECUTABLE})
