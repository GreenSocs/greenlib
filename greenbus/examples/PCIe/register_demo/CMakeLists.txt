SET(EXECUTABLE "register_demo")

ADD_EXECUTABLE(${EXECUTABLE}
	example_register_demo.cpp
	MyPCIeRootComplex.cpp
	PCIeRecvInfoDevice.cpp
	PCIeSendDevice.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND ./${EXECUTABLE})

add_dependencies(compile_test ${EXECUTABLE})
