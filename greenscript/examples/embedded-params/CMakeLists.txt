SET(EXECUTABLE "embedded-params")

ADD_EXECUTABLE(${EXECUTABLE}
	system.cpp
)
target_link_libraries(${EXECUTABLE} ${SystemC_LIBRARIES}  pthread ${PYTHON_LIBRARIES} greenscript ${LUA_LIBRARIES})

add_test(NAME COMPILATION:_${EXECUTABLE} COMMAND  sh -c "./${EXECUTABLE}")

add_dependencies(compile_test ${EXECUTABLE})
