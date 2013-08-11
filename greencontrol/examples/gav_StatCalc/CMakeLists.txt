SET(EXECUTABLE "gav_StatCalc")

ADD_EXECUTABLE(${EXECUTABLE}
	AVnewStatCalcFormulaTest.cpp
	AVnewStCalc.cpp
	testbench_analysis_statcalc.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND  sh -c "./${EXECUTABLE} > executable_output.txt")
add_test(NAME DIFF:_${EXECUTABLE} COMMAND ${CMAKE_COMMAND} -E compare_files executable_output.txt expected_output.txt)

add_dependencies(compile_test ${EXECUTABLE})
