SET(EXECUTABLE "gav_simple")

ADD_EXECUTABLE(${EXECUTABLE}
	AVAnalyserTool.cpp
	AVOwner.cpp
	AVscvAnalyserTool.cpp
	testbench_simple_analysis.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND  sh -c "./${EXECUTABLE} > executable_output.txt")
add_test(NAME DIFF:_${EXECUTABLE} COMMAND ${CMAKE_COMMAND} -E compare_files executable_output.txt expected_output.txt)

add_dependencies(compile_test ${EXECUTABLE})
