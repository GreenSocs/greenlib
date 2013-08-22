SET(EXECUTABLE "generic")

ADD_EXECUTABLE(${EXECUTABLE}
	ddr_ram.cpp
	example_generic_pcie.cpp
	Mem.cpp
	PCIeRootComplex2.cpp
	PCIeSendDevice3.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND ./${EXECUTABLE})

add_dependencies(compile_test ${EXECUTABLE})
