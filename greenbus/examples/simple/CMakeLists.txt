SET(EXECUTABLE "greenbus_simple")

ADD_EXECUTABLE(${EXECUTABLE}
	example_simple.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND ./${EXECUTABLE})

add_dependencies(compile_test ${EXECUTABLE})
