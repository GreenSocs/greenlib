SET(EXECUTABLE "greencontrol_regression_mixed_tests")

add_executable(${EXECUTABLE} 
  testbench.cpp
  testip.cpp
  testip2.cpp
  testip3.cpp
  testtool.cpp
  testipVec.cpp
  ParamListIP.cpp
  AttributeTestsIP.cpp
  scattribparam.cpp
  sceventparam.cpp
  )

target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES} pthread)

add_dependencies(compile_test ${EXECUTABLE})

add_test(NAME COMPILATION:_${EXECUTABLE}
         COMMAND  sh -c "./${EXECUTABLE} | grep -v 'In file:' > executable_output.txt")

add_test(NAME DIFF:_${EXECUTABLE}
         COMMAND ${CMAKE_COMMAND} -E compare_files executable_output.txt
                                ${CMAKE_CURRENT_SOURCE_DIR}/expected_output.txt)
