SET(EXECUTABLE "simple_b_transport_multi")

ADD_EXECUTABLE(${EXECUTABLE}
	../common/simple_b_gs_secure_adapter.cpp
        ../common/simple_b_gs_secure_slave.cpp
        ../common/simple_b_gs_slave.cpp
	simple_b_gs_master_multi.cpp
	simple_b_transport_top.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND ${EXECUTABLE})

add_dependencies(compile_test ${EXECUTABLE})
