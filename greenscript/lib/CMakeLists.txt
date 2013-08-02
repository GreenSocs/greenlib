ADD_LIBRARY(greenscript STATIC
	../src/greenscript.cpp
    )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_GREENCONTROL")

#
# Create _gsp_sc.so and _gsp_sc.py for embedded Python scripts.
#
SET_SOURCE_FILES_PROPERTIES(../src/gsp_scPYTHON_wrap.cxx PROPERTIES GENERATED 1)
SET_SOURCE_FILES_PROPERTIES(../src/gsp_sc.i PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(../src/gsp_sc.i
                                     PROPERTIES SWIG_FLAGS "-DUSE_GREENCONTROL")
SWIG_ADD_MODULE(gsp_sc python ../src/gsp_sc.i)

ADD_LIBRARY(_gsp_sc.so SHARED ../src/gsp_sc.cpp
                              ../src/gsp_scPYTHON_wrap.cxx)
SET_TARGET_PROPERTIES(_gsp_sc.so PROPERTIES PREFIX "")
SET_TARGET_PROPERTIES(_gsp_sc.so PROPERTIES SUFFIX "")

install(TARGETS greenscript DESTINATION lib)
install(FILES _gsp_sc.so
        DESTINATION "lib/python${PYTHONLIBS_VERSION_STRING}/site-packages")
install(FILES gsp_sc.py
        DESTINATION "lib/python${PYTHONLIBS_VERSION_STRING}/site-packages")
install(FILES ../gs.py
        DESTINATION "lib/python${PYTHONLIBS_VERSION_STRING}/site-packages")
