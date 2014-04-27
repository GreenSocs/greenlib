SET(EXECUTABLE "greencontrol_scml_example")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_DEPRECATED_WARNINGS")

add_executable(${EXECUTABLE} 
  testbench.cpp
  scml_test_ip.cpp
  scml_orig_ip.cpp
  )

target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES} pthread)

add_dependencies(compile_test ${EXECUTABLE})

add_test(NAME COMPILATION:_${EXECUTABLE}
         COMMAND  sh -c "./${EXECUTABLE} > executable_output.txt")
add_test(NAME DIFF:_${EXECUTABLE}
         COMMAND ${CMAKE_COMMAND} -E compare_files executable_output.txt
                                ${CMAKE_CURRENT_SOURCE_DIR}/expected_output.txt)
