SET(EXECUTABLE "simple_quantum_keeper")

ADD_EXECUTABLE(${EXECUTABLE}
	b_master_qk.cpp	
	top.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND ${EXECUTABLE})

add_dependencies(compile_test ${EXECUTABLE})
