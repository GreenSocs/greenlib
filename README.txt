To build GreenLib, please do the following :



First, please set the variable SYSTEMC_PREFIX with the path
for the systemC/TLM directory this way :
 #> export SYSTEMC_PREFIX="/tmp/systemc-2.3.0"

SystemC: http://www.accellera.org/downloads/standards/systemc



Secondly, ensure that you have Boost, (best version for this project is 1.49)
and if cmake doesn't find boost, specify the boost directory this way :
 #> export BOOST_ROOT="/usr/include/boost"

Boost 1.49: http://www.boost.org/users/history/version_1_49_0.html



Thirdly, the Python header files are neccessary so assure you have this. 
If cmake doesn't find Python, maybe specify the python directory this way :
 #> export PYTHON_ROOT="/usr/include/python2.7"

Python: http://python.org/download/

Next step, is for testing the library.

The Lua library is necessary, so assure you have them.
If cmake doesn't find Lua, maybe specify the lua directories this way :
 #> export LUA_LYBRARY_HOME="/usr/lib/x86_64-linux-gnu/liblua5.2.so"
 #> export LUA_INCLUDE_HOME="/usr/include/lua5.2/"

Lua: http://www.lua.org/download.html


Now we can compile :)

1 . Type #> cmake . -DCMAKE_INSTALL_PREFIX=. 
    This will result in generating the Makefile to build GreenLib
    If you wish to install the package in another folder,
    please change CMAKE_INSTALL_PREFIX 
    If you don't precise the CMAKE_INSTALL_PREFIX it will be in /usr/local

2 . Type #> make 
    This action will result in the compilation  

3 . Type #> make install
    This action will result in a package of GreenLib
    where you have defined CMAKE_INSTALL_PREFIX

4 . Enjoy !

If you want to test, you need to do the previous step and after:

1 . Type #> make compile_test
    This action will result in the compilation of the test

2 . Type #> make test
    This action will result in the test with the expected_output
    or to see other problem

3.  You can type #> make check
    This action will do the last two step in the same time

How to include this library in a project:

* Copy the misc/FindGREENLIB.cmake in your project repository.
* Add the following lines into your CMakeLists.txt:
    FIND_PACKAGE(GREENLIB)
    if(GREENLIB_FOUND)
      INCLUDE_DIRECTORIES(${GREENLIB_INCLUDE_DIRS})
      LINK_DIRECTORIES(${GREENLIB_LIBRARY_DIRS})
    else()
      MESSAGE(FATAL_ERROR "GreenLib not found.")
    endif()

You're now able to use PL011 headers and link the toplevel with
GREENLIB_LIBRARIES.

CMake: http://www.cmake.org/cmake/resources/software.html
