SET(EXECUTABLE "greensignalbus_example")

add_executable(${EXECUTABLE} example.cpp)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES} pthread)
add_dependencies(compile_test ${EXECUTABLE})
add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND ${EXECUTABLE})

