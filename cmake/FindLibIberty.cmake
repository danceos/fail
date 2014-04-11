# - FindLibIberty.cmake
# This module can find libiberty on both Debian 7 and Ubuntu 14.04
# (The libiberty headers moved from /usr/include to /usr/include/libiberty
# between Ubuntu 13.10 and 14.04, which made this search module necessary.)
#
# The following variables will be defined for your use:
#
#   LibIberty_FOUND - TRUE if both library and headers were found
#   LibIberty_INCLUDE_DIRS - Include directories
#   LibIberty_LIBRARIES - Library path

# set(LibIberty_PREFER_DYNAMIC True) if you want to prefer the dynamic library

if(LibIberty_PREFER_DYNAMIC)
	set(LibIberty_SEARCHORDER libiberty.so libiberty.a)
else()
	set(LibIberty_SEARCHORDER libiberty.a libiberty.so)
endif()

find_library(LibIberty_LIBRARIES NAMES ${LibIberty_SEARCHORDER})

find_path(LibIberty_INCLUDE_DIRS libiberty.h
	PATHS
		/usr/include /usr/include/libiberty
		/usr/local/include /usr/local/include/libiberty
	DOC "libiberty include directory containing libiberty.h")

if(LibIberty_INCLUDE_DIRS AND LibIberty_LIBRARIES)
	set(LibIberty_FOUND TRUE)
endif()

if(LibIberty_FOUND)
	if(NOT LibIberty_FIND_QUIETLY)
		MESSAGE(STATUS "Found libiberty: ${LibIberty_LIBRARIES}")
	endif()
else()
	if(LibIberty_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find libiberty: install binutils-dev or libiberty-dev")
	else()
		MESSAGE(STATUS "libiberty not found")
	endif()
endif()
