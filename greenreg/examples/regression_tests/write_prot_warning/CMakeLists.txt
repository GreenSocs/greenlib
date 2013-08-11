SET(EXECUTABLE "write_prot_warning")

ADD_EXECUTABLE(${EXECUTABLE}
	sc_main.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES} pthread greenreg)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND  sh -c "./${EXECUTABLE} > executable_output.txt")
add_test(NAME DIFF:_${EXECUTABLE} COMMAND ${CMAKE_COMMAND} -E compare_files executable_output.txt expected_output.txt)

add_dependencies(compile_test ${EXECUTABLE})
