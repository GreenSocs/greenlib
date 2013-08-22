SET(EXECUTABLE "embedded-adder")

ADD_EXECUTABLE(${EXECUTABLE}
	system.cpp
)
set_target_properties(${EXECUTABLE} PROPERTIES COMPILE_FLAGS "-g -Xlinker -export-dynamic")

target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}
                                    ${PYTHON_LIBRARIES}
                                    greenscript
                                    ${LUA_LIBRARIES})

add_test(NAME COMPILATION:_${EXECUTABLE}
         COMMAND sh -c "export PYTHONPATH=$ENV{PYTHONPATH} && ./${EXECUTABLE} > executable_output.txt")
add_test(NAME DIFF:_${EXECUTABLE}
         COMMAND ${CMAKE_COMMAND} -E compare_files
                                      executable_output.txt expected_output.txt)

add_dependencies(compile_test ${EXECUTABLE})
