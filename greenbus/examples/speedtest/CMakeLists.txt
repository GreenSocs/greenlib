SET(EXECUTABLE "speedtest")

ADD_EXECUTABLE(${EXECUTABLE}
	ddr_ram.cpp
	ddr_speedtest.cpp
	pseudo_master_simplebus.cpp
	speedtest_top.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND  sh -c "./${EXECUTABLE} > executable_output.txt")
#add_test(NAME DIFF:_${EXECUTABLE} COMMAND ${CMAKE_COMMAND} -E compare_files executable_output.txt expected_output.txt)

add_dependencies(compile_test ${EXECUTABLE})
