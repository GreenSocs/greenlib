To build GreenLib, please do the following :



First, please set the variable SYSTEMC_PREFIX with the path
for the systemC/TLM directory this way :
 #> export SYSTEMC_PREFIX="/tmp/systemc-2.3.0"

SystemC: http://www.accellera.org/downloads/standards/systemc



Secondly, please ensure that you have Boost, (best version for this project is 1.49)
and if cmake doesn't find boost, maybe specify the boost directory this way :
 #> export BOOST_ROOT="/usr/include/boost"

Boost 1.49: http://www.boost.org/users/history/version_1_49_0.html



Thirdly, the Python header files are neccessary so please assure you have this. 
If cmake doesn't find Python, maybe specify the python directory this way :
 #> export PYTHON_ROOT="/usr/include/python2.7"

Python: http://python.org/download/



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

CMake: http://www.cmake.org/cmake/resources/software.html
