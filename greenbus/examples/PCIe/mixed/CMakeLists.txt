SET(EXECUTABLE "mixed")

ADD_EXECUTABLE(${EXECUTABLE}
	ddr_ram.cpp
	example_mixed.cpp
	GenericMaDevice.cpp
	PCIeMaDevice.cpp
	PCIeRecvDevice2.cpp
	Mem.cpp
	PCIeRootComplex2.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND ./${EXECUTABLE})

add_dependencies(compile_test ${EXECUTABLE})
