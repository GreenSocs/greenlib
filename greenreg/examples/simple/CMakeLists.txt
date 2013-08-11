SET(EXECUTABLE "greenreg_simple")

ADD_EXECUTABLE(${EXECUTABLE}
	sc_main.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES} pthread greenreg)

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND ${EXECUTABLE})

add_dependencies(compile_test ${EXECUTABLE})
