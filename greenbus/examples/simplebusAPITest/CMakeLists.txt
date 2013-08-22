SET(EXECUTABLE "simplebusAPITest")

ADD_EXECUTABLE(${EXECUTABLE}
	ddr_ram.cpp
	example_simplebusAPI.cpp
	simplebusAPImaster.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND ./${EXECUTABLE})

add_dependencies(compile_test ${EXECUTABLE})
