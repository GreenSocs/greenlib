cmake_minimum_required(VERSION 2.8)
project (GREENLIB)

include(FeatureSummary)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})


# Compile and run the tests
add_custom_target(check COMMAND make compile_test COMMAND make test)

# Compile the test
add_custom_target(compile_test)
enable_testing()

# Test if the compilation/running/diff is good
add_custom_target(run_test COMMAND make test)

# Require SystemC

find_package( SystemC )
if(SystemC_FOUND)
    include_directories(${SystemC_INCLUDE_DIRS})
    link_directories(${SystemC_LIBRARIES})
else()
    message( FATAL_ERROR "SystemC library not found." )
endif()

set_package_properties(SystemC PROPERTIES
                       URL "http://www.accellera.org/downloads/standards/systemc"
                       DESCRIPTION "The SystemC Hardware Description Language"
                       TYPE REQUIRED
                       PURPOSE "Compile the examples")

# Require TLM
find_package( TLM )
if(TLM_FOUND)
    include_directories(${TLM_INCLUDE_DIRS})
else()
    message( FATAL_ERROR "TLM library not found." )
endif()


# Require Boost

set(Boost_ADDITIONAL_VERSIONS 1.34 1.34.0 1.34.1)

find_package( Boost )
if(Boost_FOUND)
   include_directories(${Boost_INCLUDE_DIRS})
endif()

# Require Python

if(DEFINED ENV{PYTHON_ROOT})
  include_directories($ENV{PYTHON_ROOT})
else()
  find_package(PythonLibs REQUIRED)
  include_directories(${PYTHON_INCLUDE_DIRS})
endif()

# Require Lua

find_package( Lua )
if(LUA_FOUND)
  include_directories(${LUA_INCLUDE_DIR})
  link_directories(${LUA_LIBRARIES})
elseif(DEFINED ENV{LUA_INCLUDE_HOME} && DEFINED ENV{LUA_LIBRARY_HOME})
  include_directories($ENV{LUA_INCLUDE_HOME})
  link_directories($ENV{LUA_LIBRARY_HOME})
else()
  message( STATUS "Lua library not found." )
endif()

# Require SWIG
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

# GreenSocs library

set (GreenSocs_INCLUDE_DIRS "")
set (GreenSocs_LIBRARY_DIRS ../../../lib)
set (GreenSocs_LIBRARIES pthread)


# License

install(FILES greencontrol/LICENSE
        DESTINATION .
)


# GreenControl

configure_file(
    greencontrol/examples/Makefile.defs.in
    Makefile.defs
    @ONLY
)

set (GreenSocs_INCLUDE_DIRS ${GreenSocs_INCLUDE_DIRS} greencontrol/include/)

install(DIRECTORY greencontrol/include/
        DESTINATION include
        PATTERN ".svn" EXCLUDE
)

install(DIRECTORY greencontrol/examples/
        DESTINATION examples/greencontrol
        PATTERN ".svn" EXCLUDE
)

install(DIRECTORY greencontrol/docs/
        DESTINATION docs/greencontrol
        PATTERN ".svn" EXCLUDE
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile.defs
        DESTINATION examples/greencontrol
)


# GreenSocket

set (GreenSocs_INCLUDE_DIRS ${GreenSocs_INCLUDE_DIRS} greensocket/include/)

install(DIRECTORY greensocket/include/
        DESTINATION include
        PATTERN ".svn" EXCLUDE
)

install(DIRECTORY greensocket/examples/
        DESTINATION examples/greensocket
        PATTERN ".svn" EXCLUDE
)

install(DIRECTORY greensocket/doc/     # TODO: doc or docs?
        DESTINATION docs/greensocket
        PATTERN ".svn" EXCLUDE
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile.defs
        DESTINATION examples/greensocket
)


# GsgpSocket

set (GreenSocs_INCLUDE_DIRS ${GreenSocs_INCLUDE_DIRS} gsgpsocket/include/)

install(DIRECTORY gsgpsocket/include/
        DESTINATION include
        PATTERN ".svn" EXCLUDE
)

install(DIRECTORY gsgpsocket/examples/
        DESTINATION examples/gsgpsocket
        PATTERN ".svn" EXCLUDE
)

# install(DIRECTORY gsgpsocket/doc/     # TODO: doc or docs?
#         DESTINATION docs/gsgpsocket
#         PATTERN ".svn" EXCLUDE
# )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile.defs
        DESTINATION examples/gsgpsocket
)


# GreenRouter

set (GreenSocs_INCLUDE_DIRS ${GreenSocs_INCLUDE_DIRS} greenrouter/include/)

install(DIRECTORY greenrouter/include/
        DESTINATION include
        PATTERN ".svn" EXCLUDE
)

install(DIRECTORY greenrouter/examples/
        DESTINATION examples/greenrouter
        PATTERN ".svn" EXCLUDE
)

install(DIRECTORY greenrouter/docs/     # TODO: doc or docs?
        DESTINATION docs/greenrouter
        PATTERN ".svn" EXCLUDE
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile.defs
        DESTINATION examples/greenrouter
)

# GreenSignalSocket

set (GreenSocs_INCLUDE_DIRS ${GreenSocs_INCLUDE_DIRS} greensignalsocket/include/)

install(DIRECTORY greensignalsocket/include/
        DESTINATION include
        PATTERN ".svn" EXCLUDE
)

install(DIRECTORY greensignalsocket/examples/
        DESTINATION examples/greensignalsocket
        PATTERN ".svn" EXCLUDE
)

install(DIRECTORY greensignalsocket/docs/     # TODO: doc or docs?
        DESTINATION docs/greensignalsocket
        PATTERN ".svn" EXCLUDE
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile.defs
        DESTINATION examples/greensignalsocket
)

# Greenreg

set (GreenSocs_INCLUDE_DIRS ${GreenSocs_INCLUDE_DIRS} greenreg/include/)
set (GreenSocs_LIB_DIRS ${GreenSocs_LIB_DIRS} ${PROJECT_SOURCE_DIR}/greenreg/lib/libgreenreg.a)

install(DIRECTORY greenreg/include/
        DESTINATION include
        PATTERN ".svn" EXCLUDE
)

install(DIRECTORY greenreg/examples/
        DESTINATION examples/greenreg
        PATTERN ".svn" EXCLUDE
)

install(DIRECTORY greenreg/doc/     # TODO: doc or docs?
        DESTINATION docs/greenreg
        PATTERN ".svn" EXCLUDE
)

# Greenmessage

set (GreenSocs_INCLUDE_DIRS ${GreenSocs_INCLUDE_DIRS} greenmessage/include/)

install(DIRECTORY greenmessage/include/
	DESTINATION include
)

install(DIRECTORY greenmessage/examples/
	DESTINATION examples/greenmessage
)

install(DIRECTORY greenmessage/docs/
	DESTINATION docs/greenmessage
)


# Greensocs Library include directory (for example).
include_directories(${GreenSocs_INCLUDE_DIRS})
link_directories(${GreenSocs_LIB_DIRS})

# CXX flags for example or library compilation.
set(CMAKE_CXX_FLAGS "-g -Wall -DSC_INCLUDE_DYNAMIC_PROCESSES -D_REENTRANT")

add_subdirectory(greensignalsocket EXCLUDE_FROM_ALL)
add_subdirectory(greenrouter EXCLUDE_FROM_ALL)

# Compile greenreg library.
add_subdirectory(greenreg)

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE  
     "${CMAKE_CURRENT_SOURCE_DIR}/greencontrol/LICENSE")
set (CPACK_PACKAGE_VERSION_MAJOR "1")
set (CPACK_PACKAGE_VERSION_MINOR "0")
set (CPACK_PACKAGE_VERSION_PATCH "0")
include (CPack)
