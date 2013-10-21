SET(EXECUTABLE "configurable")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_STATIC_CASTS")

ADD_EXECUTABLE(${EXECUTABLE}
	example_primchannel.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND  sh -c "./${EXECUTABLE} > executable_output.txt")

add_dependencies(compile_test ${EXECUTABLE})
