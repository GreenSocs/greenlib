SET(EXECUTABLE "simplebus")

ADD_EXECUTABLE(${EXECUTABLE}
	example_simplebus.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND ./${EXECUTABLE})

add_dependencies(compile_test ${EXECUTABLE})
