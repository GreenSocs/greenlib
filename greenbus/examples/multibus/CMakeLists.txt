SET(EXECUTABLE "multibus")

ADD_EXECUTABLE(${EXECUTABLE}
	example_two_busses.cpp
	example_bridge.cpp
	example_hierarchy_busses.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND ${EXECUTABLE})

add_dependencies(compile_test ${EXECUTABLE})
