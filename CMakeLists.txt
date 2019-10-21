# cmake 2.6 might suffice, but we don't test it (even Debian stable has 2.8.2)
cmake_minimum_required(VERSION 2.8.2)
if("${CMAKE_VERSION}" VERSION_GREATER 2.8.3)
  # system cmake modules take precedence over those in CMAKE_MODULE_PATH
  # (makes cmake 2.8.4 and newer)
  cmake_policy(SET CMP0017 NEW)
endif("${CMAKE_VERSION}" VERSION_GREATER 2.8.3)
if(NOT ("${CMAKE_VERSION}" VERSION_LESS 3.0)) # >= 3.0?
  # overwrite PROJECT_VERSION when calling PROJECT()
  cmake_policy(SET CMP0048 NEW)

  PROJECT(FAIL* VERSION 1.0.1)
else()
  PROJECT(FAIL*)
  set(PROJECT_VERSION "1.0.1" CACHE INTERNAL "FAIL* version number" FORCE)
endif()

ENABLE_TESTING()

#### Put all resulting library files in <your_build_dir>/lib ####
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

#### Put all resulting executables in <your_build_dir>/bin ####
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
# At the moment this is the campaign controller executable.
## (The autoconf'd Bochs instance is placed in the auto-configured path,
## as we still just call Bochs' Makefile's make install)

# where will executable tests be written ?
IF (EXECUTABLE_OUTPUT_PATH)
    SET (CXX_TEST_PATH ${EXECUTABLE_OUTPUT_PATH})
ELSE (EXECUTABLE_OUTPUT_PATH)
    SET (CXX_TEST_PATH .)
ENDIF (EXECUTABLE_OUTPUT_PATH)

#### Setup search path for custom cmake scipts  ####
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

#### Compiler configuration, see cmake/compilerconfig.cmake
include(BuildType)
include(compilerconfig)
include(doxygen)

#### Backend selection ####
OPTION( BUILD_BOCHS  "Build Bochs Variant?" ON)
OPTION( BUILD_GEM5   "Build gem5 Variant?" OFF)
OPTION( BUILD_QEMU   "Build QEMU Variant?" OFF)
OPTION( BUILD_T32    "Build Lauterbach Trace32 Variant?" OFF)
OPTION( BUILD_PANDA  "Build Pandaboard ES + Flyswatter2 Variant?" OFF)

OPTION( BUILD_X86  "Build for x86 guests?" ON)
OPTION( BUILD_ARM  "Build for ARM guests?" OFF)

# FIXME: only add simulators/ to include_directories, and include, e.g.,
# bochs/bochs.h in FAIL*. -> avoids naming conflicts (e.g., /usr/include/elf.h
# vs. qemu/elf.h)
if(BUILD_BOCHS)
  ## add necessary additional header search paths.
  #add_definitions(-I${CMAKE_SOURCE_DIR}/simulators/bochs/instrument/stubs/ -I${CMAKE_SOURCE_DIR}/simulators/bochs)
  include_directories(simulators/bochs/instrument/stubs simulators/bochs)
elseif(BUILD_GEM5)
  include_directories(simulators/gem5/src simulators/gem5/build/ARM)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -DTRACING_ON")
elseif(BUILD_QEMU)
  include_directories(simulators)
elseif(BUILD_T32)
  include_directories(debuggers/t32/include src/core)
  add_subdirectory(debuggers/t32)
  add_subdirectory(scripts/t32cmm)
elseif(BUILD_PANDA)
  include_directories(debuggers/openocd/src debuggers/openocd/jimtcl src/core)
endif(BUILD_BOCHS)

## Tell the linker where to find the FAIL* libraries
link_directories("${LIBRARY_OUTPUT_PATH}")

# Add "simulators"-directory to the include path. This allows
# us to use simulator-specific headers in a comfortable way.
include_directories(${CMAKE_BINARY_DIR}/src/core)
# FIXME: this should be in src/core/CMakeLists.txt but actually doesn't work

## Add CMakeLists from subdirectories:
add_subdirectory(tools)
add_subdirectory(src)


#### Backend-related build system stuff
include(bochs)
include(gem5)
include(qemu)
include(t32)
include(panda)

