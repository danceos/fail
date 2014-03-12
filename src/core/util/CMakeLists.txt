set(SRCS
 CommandLine.cc
 CommandLine.hpp
 ElfReader.cc
 ElfReader.hpp
 Database.hpp
 Database.cc
 DatabaseProtobufAdapter.hpp
 DatabaseProtobufAdapter.cc
 Demangler.hpp
 Demangler.cc
 Disassembler.hpp
 Disassembler.cc
 DwarfReader.cc
 DwarfReader.hpp
 elfinfo/elfinfo.cc
 elfinfo/elfinfo.h
 gzstream/gzstream.C
 gzstream/gzstream.h
 Logger.cc
 Logger.hpp
 MemoryMap.cc
 MemoryMap.hpp
 ProtoStream.cc
 ProtoStream.hpp
 SynchronizedCounter.cc
 SynchronizedCounter.hpp
 SynchronizedMap.hpp
 SynchronizedQueue.hpp
 WallclockTimer.cc
 WallclockTimer.hpp

)

# required by ProtoStream.cc:
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# required by Synchronized*.cc, MemoryMap.* (needs icl, which came with 1.46):
find_package(Boost 1.46 COMPONENTS thread regex system REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# libiberty required by Demangler.cc:
find_library(LIB_IBERTY iberty)
mark_as_advanced(LIB_IBERTY)

if(${LIB_IBERTY} STREQUAL LIB_IBERTY-NOTFOUND)
  message(FATAL_ERROR "libiberty not found. Try installing binutils-dev: [ sudo aptitude install binutils-dev ]")
endif()

# libz required by gzstream
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

# libelf and libdwarf required by ElfReader/DwarfReader
find_package(LibElf REQUIRED)
find_package(LibDwarf REQUIRED)
include_directories(${LIBELF_INCLUDE_DIRS})
include_directories(${LIBDWARF_INCLUDE_DIRS})

# objdump required by Diassembler.cc

set(THE_OBJDUMP "${ARCH_TOOL_PREFIX}objdump")
find_program(FAIL_OBJDUMP "${THE_OBJDUMP}" DOC "binutils object dump tool")
if(${FAIL_OBJDUMP} STREQUAL FAIL_OBJDUMP-NOTFOUND)
  message(FATAL_ERROR "Cannot find objdump exeuctable (tried: ${THE_OBJDUMP}")
else()
  message(STATUS "[Fail*] objdump binary -> ${FAIL_OBJDUMP}")
endif()
mark_as_advanced(FAIL_OBJDUMP)

add_library(fail-util ${SRCS})
add_dependencies(fail-util fail-comm)
target_link_libraries(fail-util ${PROTOBUF_LIBRARY} ${Boost_LIBRARIES} ${LIB_IBERTY} ${ZLIB_LIBRARIES} ${LIBDWARF_LIBRARIES} ${LIBELF_LIBRARIES})

option(BUILD_LLVM_DISASSEMBLER "Build the LLVM-based disassembler (LLVM 3.3 preferred, for 3.1 and 3.2 read doc/how-to-build.txt)" OFF)
if (BUILD_LLVM_DISASSEMBLER)
  add_subdirectory(llvmdisassembler)
endif (BUILD_LLVM_DISASSEMBLER)

### Tests
add_executable(memorymap-test testing/memorymap-test.cc)
target_link_libraries(memorymap-test fail-util)
add_test(NAME memorymap-test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/testing COMMAND memorymap-test)
