#export PYTHONPATH for example execution.
set(ENV{PYTHONPATH} "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(ENV{PYTHONPATH} "$ENV{PYTHONPATH}:${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(lib)
if(COMPILE_EXAMPLE)
add_subdirectory(examples)
endif(COMPILE_EXAMPLE)
