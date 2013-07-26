ADD_LIBRARY( greenscript STATIC
	../src/greenscript.cpp
	../src/gsp_sc.cpp
    )

install(TARGETS greenscript DESTINATION lib)
