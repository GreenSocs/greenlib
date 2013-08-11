SET(EXECUTABLE "extended_parameter_arrays")

ADD_EXECUTABLE(${EXECUTABLE}
	EArrayOwnerModule.cpp
	EArrayUserModule.cpp
	testbench_arrays.cpp
	testbench_extended_arrays.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND  sh -c "./${EXECUTABLE} > executable_output.txt")
add_test(NAME DIFF:_${EXECUTABLE} COMMAND ${CMAKE_COMMAND} -E compare_files executable_output.txt expected_output.txt)

add_dependencies(compile_test ${EXECUTABLE})
