SET(EXECUTABLE "greenmemory_simple")

add_executable(${EXECUTABLE} test.cpp)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES} pthread)
add_dependencies(compile_test ${EXECUTABLE})
add_test(greensignalsocket_simple ${EXECUTABLE})

