ADD_LIBRARY( greenscript STATIC
	../src/greenscript.cpp
	../src/gsp_sc.cpp
    )

#SWIG related
SET_SOURCE_FILES_PROPERTIES(../src/gsp_sc.i PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(../src/gsp_sc.i PROPERTIES SWIG_FLAGS "-DUSE_GREENCONTROL")
SWIG_ADD_MODULE(gsp_sc python ../src/gsp_sc.i)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_GREENCONTROL")
ADD_LIBRARY( _gsp_sc.so SHARED
	../src/gsp_sc.cpp
    ../src/gsp_scPYTHON_wrap.cxx
	 )
SET_SOURCE_FILES_PROPERTIES(../src/gsp_scPYTHON_wrap.cxx PROPERTIES GENERATED 1)
SET_TARGET_PROPERTIES(_gsp_sc.so PROPERTIES PREFIX "")
SET_TARGET_PROPERTIES(_gsp_sc.so PROPERTIES SUFFIX "")

install(TARGETS _gsp_sc DESTINATION lib_shared)
install(TARGETS greenscript DESTINATION lib)
