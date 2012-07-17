# cmake 2.6 might suffice, but we don't test it (even Debian stable has 2.8.2)
cmake_minimum_required(VERSION 2.8.2)
if("${CMAKE_VERSION}" VERSION_GREATER 2.8.3)
  # system cmake modules take precedence over those in CMAKE_MODULE_PATH
  # (makes cmake 2.8.4 and newer)
  cmake_policy(SET CMP0017 NEW)
endif("${CMAKE_VERSION}" VERSION_GREATER 2.8.3)

PROJECT(Fail*)

set(PROJECT_VERSION "0.0.1" CACHE STRING "Fail* version number")

#### Put all resulting library files in <your_build_dir>/lib ####
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

#### Put all resulting executables in <your_build_dir>/bin ####
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
# At the moment this is the campaign controller executable.
## (The autoconf'd Bochs instance is placed in the auto-configured path,
## as we still just call Bochs' Makefile's make install)


#### Setup search path for custom cmake scipts  ####
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

#### Compiler configuration, see cmake/compilerconfig.cmake
include(compilerconfig)

#### #OPTION to configure Bochs/OVP ####
OPTION( BUILD_OVP    "Build OVP Variant?" OFF) # Defaults to BOCHS  ON
OPTION( BUILD_GEM5   "Build gem5 Variant?" OFF)
OPTION( BUILD_BOCHS  "Build Bochs Variant?" ON)

if(BUILD_BOCHS)
  ## add necessary additional header search paths.
  #add_definitions(-I${CMAKE_SOURCE_DIR}/simulators/bochs/instrument/stubs/ -I${CMAKE_SOURCE_DIR}/simulators/bochs)
  include_directories(simulators/bochs/instrument/stubs simulators/bochs)
elseif(BUILD_GEM5)
  include_directories(simulators/gem5/src simulators/gem5/build/ARM)
  add_definitions(-std=c++0x)
elseif(BUILD_OVP)
  add_subdirectory(simulators/ovp)
endif(BUILD_BOCHS)

## Additional Compiler flags ##
set(CMAKE_C_FLAGS "-g -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}") 
## Tell the linker where to find the libfail.a
link_directories("${LIBRARY_OUTPUT_PATH}")

# Add "simulators"-directory to the include path. This allows
# us to use simulator-specific headers in a comfortable way.
include_directories(${CMAKE_BINARY_DIR}/src/core)
# FIXME: this should be in src/core/CMakeLists.txt but actually doesn't work

## Add CMakeLists from subdirectories:
add_subdirectory(src)

#### Backend-related build system stuff
include(bochs)
include(ovp)
include(gem5)

## Just for testing:
## Invoking bochs build via external project
# Setup configure call for bochs (-> make ebochs)
# Prefix dir for make install etc
#set(BOCHS_PREFIX_DIR "${CMAKE_BINARY_DIR}/bochs")
#configure_file(${CMAKE_SOURCE_DIR}/cmake/config_failbochs.sh.in ${CMAKE_BINARY_DIR}/config_bochs.sh)
#
#include(ExternalProject)
#externalproject_add(
#	ebochs
#	SOURCE_DIR 		${CMAKE_SOURCE_DIR}/bochs
#	CONFIGURE_COMMAND 	${CMAKE_BINARY_DIR}/config_bochs.sh
#	BUILD_IN_SOURCE 	1
#	PREFIX			${BOCHS_PREFIX_DIR}
#)
#
#message(STATUS "Include directories are:")
#get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#foreach(dir ${dirs})
#  message(STATUS "    ${dir}")
#endforeach(dir)
