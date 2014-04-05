# INCLUDE
include_directories(examples/)

# EXAMPLES
add_subdirectory(examples/bidir_nb_example)
add_subdirectory(examples/bidir_nb_example_hier)
add_subdirectory(examples/f_example)
add_subdirectory(examples/f_example_hier)
add_subdirectory(examples/generic_port_read_write)
add_subdirectory(examples/nb_example)
add_subdirectory(examples/simple)
add_subdirectory(examples/simplebus)
add_subdirectory(examples/test_devices)
# Can't work in systemC2.3
#add_subdirectory(examples/osci_slave_gp_master_example)
