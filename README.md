

# GreenLib

[![Join the chat at https://gitter.im/GreenSocs/greenlib](https://badges.gitter.im/GreenSocs/greenlib.svg)](https://gitter.im/GreenSocs/greenlib?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

## Overview

**Productivity library for SystemC modelling**

* **GreenReg** : Easy accessible register modeling
* **GreenSocket** : More advanced and customized socket
* **GreenSignalSocket** : Model all the ports corresponding to different signals in an IP
* **GreenConfig** : Flexible configuration and inspection
* **GreenControl** : Router that connects user modules with service providers (plugins)
* **GreenMessage** : A framework to pass messages from one SystemC module to another
* **GreenSerialSocket** : TLM2 based Asynchronous Serial communication protocol which can be used to model industry standard serial interfaces like UART Model
* **GreenScript** : SystemC module using python
* **GreenRouter** : Connect the IPs using the GreenSocket infrastructure
* **GreenMemory** : Implementation of a sparse memory array to simplify
the implementation of more complex memories

## Code status

[![build status](http://ci.greensocs.com/projects/12/status.png?ref=master)](http://ci.greensocs.com/projects/12?ref=master)

## Requirements

GreenLib requires :

* GIT > 1.8.2
* SystemC 2.3.x
* CMake > 2.8
* Boost > 1.34
* Python
* Glib2
* Pixman
* Lua > 5.1
* SWIG

OS support :

* Linux (Debian/Ubuntu, CentOS)
* Windows[^windows-support] with MinGW
* OS X : soon

[^windows-support]: Currently, we don't support GreenScript on Windows.

## Install requirements

### Linux

**Ubuntu / Debian :**

```sh
$ sudo apt-get install cmake libboost-dev python-dev liblua5.2-dev swig
```

**CentOS / Fedora :**

```sh
$ sudo yum install cmake boost-devel python-devel lua-devel swig
```

### Windows

Please try to install packages in paths without spaces.

* [MinGW](http://sourceforge.net/projects/mingw/) with MSYS
* [CMake](http://www.cmake.org/download/)
* [Boost](http://sourceforge.net/projects/boost/files/boost-binaries/)
* [SWIG](http://www.swig.org/download.html)
* [LUA](http://www.lua.org/download.html)
* [SystemC](http://www.accellera.org/downloads/standards/systemc/)

With MinGW Installer, install these following packages :

**MinGW**

* gcc
* gcc-g++

## Build

```sh
$ cmake [options]
```

This will result in generating the `Makefile` to build **GreenLib**.

Available options are:

* `CMAKE_INSTALL_PREFIX`[^CMAKE_INSTALL_PREFIX-default] : Specify install directory used by install.
* `SYSTEMC_PREFIX` : Specify SystemC prefix (Otherwise, CMake will scan `SYSTEMC_HOME` environment variable)
* `LUA_LIBRARIES` : Specify LUA libraries directory (where is located lua.a, lua.dll, ...).
* `LUA_INCLUDE_DIR` : Specify LUA header directory (where is located lua.h).
* `BOOST_ROOT` : Specify Boost directory.
* `SWIG_DIR` : Specify SWIG directory (where is located swig binary).
* `SWIG_EXECUTABLE` : Specify SWIG binary path.
* `-G` : Specify build tools. Currently, we only support "Unix Makefiles".

[^CMAKE_INSTALL_PREFIX-default]: Default value is `/usr/local`

### Example

#### Linux

```sh
$ cmake -DCMAKE_INSTALL_PREFIX=build -DSYSTEMC_PREFIX=/usr/local/lib/systemc-2.3.0/
```
(The default value of ``` CMAKE_INSTALL_PREFIX``` is ```/usr/local```).

#### Windows

```sh
cmake -DCMAKE_INSTALL_PREFIX=build
-DSYSTEMC_PREFIX=/c/Projects/systemc-2.3.1 \
-DLUA_LIBRARIES=/c/Projects/lua-5.2.4/src/liblua.a
-DLUA_INCLUDE_DIR=/c/Projects/lua-5.2.4/src/ \
-DBOOST_ROOT=/c/Projects/boost-1-57-0/ \
-DSWIG_DIR=/c/Projects/swigwin-3.0.5
-DSWIG_EXECUTABLE=/c/Projects/swigwin-3.0.5/swig.exe \
-G"Unix Makefiles"
```

Then compile:

```sh
make -j[Specifies the number of jobs (commands) to run simultaneously]
```

This action will result in the compilation of **GreenLib**.

## Check

```sh
make check
```

or with parallel jobs:

```sh
make check -j[Specifies the number of jobs (commands) to run simultaneously]
```

This action will compile examples and check output with expected output.

If everything is OK, you should get *"100% tests passed"* message.

## Installation

```sh
make install
```

This action will result in a package of **GreenLib** where you have defined `CMAKE_INSTALL_PREFIX`.

## Usage

* Copy the `misc/FindGreenLib.cmake` file in your project repository.
* Add the following lines into your `CMakeLists.txt`:

```cmake
find_package(GreenLib)
if(GreenLib_FOUND)
	include_directories(${GREENLIB_INCLUDE_DIRS})
   link_directories(${GREENLIB_LIBRARY_DIRS})
else()
	message(FATAL_ERROR "GreenLib not found.")
endif()
```

## Contributing

**GreenLib** is an open source, community-driven project. If you'd like to contribute, please feel free to fork project and open a merge request or to [send us] a patch.

[send us]:http://www.greensocs.com/contact

![GreenLib-logo](http://static.greensocs.com/logo.png)

