SET(EXECUTABLE "embedded-adder-fifo")

ADD_EXECUTABLE(${EXECUTABLE}
	system.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread ${PYTHON_LIBRARIES} greenscript ${LUA_LIBRARIES})

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND ${EXECUTABLE})

add_dependencies(compile_test ${EXECUTABLE})
