SET(EXECUTABLE "greenmessage-configset")

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND  sh -c "export PYTHONPATH=$ENV{PYTHONPATH} && python system.py > executable_output.txt")
add_test(NAME DIFF:_${EXECUTABLE} COMMAND ${CMAKE_COMMAND} -E compare_files executable_output.txt expected_output.txt)

add_dependencies(compile_test ${EXECUTABLE})
