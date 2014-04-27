SET(EXECUTABLE "greencontrol_messages_streamer_lua_file_example")

add_executable(${EXECUTABLE} 
  testb_rep_msg_lua.cpp
  )

target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES} pthread lua5.1)

add_dependencies(compile_test ${EXECUTABLE})

add_test(NAME COMPILATION:_${EXECUTABLE}
         COMMAND  sh -c "./${EXECUTABLE} > executable_output.txt")
add_test(NAME DIFF:_${EXECUTABLE}
         COMMAND ${CMAKE_COMMAND} -E compare_files executable_output.txt
                                ${CMAKE_CURRENT_SOURCE_DIR}/expected_output.txt)
