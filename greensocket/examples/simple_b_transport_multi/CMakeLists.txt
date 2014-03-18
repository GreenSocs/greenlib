SET(EXECUTABLE "greensocket_simple_b_transport_multi")

add_executable(${EXECUTABLE} simple_b_transport_top.cpp
  simple_b_gs_master_multi.cpp
  ../common/simple_b_gs_secure_slave.cpp
  ../common/simple_b_gs_slave.cpp
  ../common/simple_b_gs_secure_adapter.cpp
)

target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES} pthread)

add_dependencies(compile_test ${EXECUTABLE})

add_test(NAME COMPILATION:_${EXECUTABLE}
         COMMAND  sh -c "./${EXECUTABLE} > executable_output.txt")
add_test(NAME DIFF:_${EXECUTABLE}
         COMMAND ${CMAKE_COMMAND} -E compare_files executable_output.txt
                                ${CMAKE_CURRENT_SOURCE_DIR}/expected_output.txt)
