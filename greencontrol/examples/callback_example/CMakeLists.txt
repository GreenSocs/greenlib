SET(EXECUTABLE "callback_example")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_STATIC_CASTS")

ADD_EXECUTABLE(${EXECUTABLE}
	ObserverModule.cpp
	ParameterOwnerModule.cpp
	ParamManipulateModule.cpp
	testbench_callback.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND  sh -c "./${EXECUTABLE} > executable_output.txt")
add_test(NAME DIFF:_${EXECUTABLE} COMMAND ${CMAKE_COMMAND} -E compare_files executable_output.txt expected_output.txt)

add_dependencies(compile_test ${EXECUTABLE})
