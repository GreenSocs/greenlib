SET(EXECUTABLE "mixed_tests")

ADD_EXECUTABLE(${EXECUTABLE}
	AttributeTestsIP.cpp
	ParamListIP.cpp
	scattribparam.cpp
	sceventparam.cpp
	testbench.cpp
	testip2.cpp
	testip3.cpp
	testip.cpp
	testipVec.cpp
	testtool.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND  sh -c "./${EXECUTABLE} > executable_output.txt")
add_test(NAME DIFF:_${EXECUTABLE} COMMAND ${CMAKE_COMMAND} -E compare_files executable_output.txt expected_output.txt)

add_dependencies(compile_test ${EXECUTABLE})
