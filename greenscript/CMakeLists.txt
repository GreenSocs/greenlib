#export PYTHONPATH for example execution.
set(ENV{PYTHONPATH} "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(ENV{PYTHONPATH} "$ENV{PYTHONPATH}:${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(lib)
add_subdirectory(examples EXCLUDE_FROM_ALL)
