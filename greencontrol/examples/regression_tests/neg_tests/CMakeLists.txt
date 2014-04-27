SET(EXECUTABLE "greencontrol_regression_neg_tests")

add_executable(${EXECUTABLE} 
  testbench_neg.cpp
  testip_neg.cpp
  testtool_neg_wrong_api.cpp
  )

target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES} pthread)

add_dependencies(compile_test ${EXECUTABLE})

add_test(NAME COMPILATION:_${EXECUTABLE}
  COMMAND  sh -c "./${EXECUTABLE} | grep -v 'In file:' > executable_output.txt")

add_test(NAME DIFF:_${EXECUTABLE}
         COMMAND ${CMAKE_COMMAND} -E compare_files executable_output.txt
                                ${CMAKE_CURRENT_SOURCE_DIR}/expected_output.txt)
