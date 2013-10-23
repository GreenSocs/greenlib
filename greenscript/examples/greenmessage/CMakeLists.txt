SET(EXECUTABLE "greenmessage")

add_test(NAME COMPILATION:_${EXECUTABLE}
         COMMAND sh -c "export PYTHONPATH=$ENV{PYTHONPATH} && python system.py")

add_dependencies(compile_test ${EXECUTABLE})
