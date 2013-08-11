SET(EXECUTABLE "messages_streamer_lua_file_example")

ADD_EXECUTABLE(${EXECUTABLE}
	testb_rep_msg_lua.cpp
)

target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread ${LUA_LIBRARIES})

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND  sh -c "./${EXECUTABLE} > executable_output.txt")
add_test(NAME DIFF:_${EXECUTABLE} COMMAND ${CMAKE_COMMAND} -E compare_files executable_output.txt expected_output.txt)

add_dependencies(compile_test ${EXECUTABLE})
