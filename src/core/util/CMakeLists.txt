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
 AliasedRegistry.hpp
 AliasedRegistry.cc
 AliasedRegisterable.hpp

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
find_package(LibIberty REQUIRED)
include_directories(${LibIberty_INCLUDE_DIRS})

# libz required by gzstream
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})

# libelf and libdwarf required by ElfReader/DwarfReader
find_package(LibElf REQUIRED)
find_package(LibDwarf REQUIRED)
include_directories(${LIBELF_INCLUDE_DIRS})
include_directories(${LIBDWARF_INCLUDE_DIRS})

# Database.hpp
find_package(MySQL REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MYSQL_CFLAGS}")

# objdump required by Diassembler.cc

set(THE_OBJDUMP "${ARCH_TOOL_PREFIX}objdump")
find_program(FAIL_OBJDUMP "${THE_OBJDUMP}" DOC "binutils object dump tool")
if(${FAIL_OBJDUMP} STREQUAL FAIL_OBJDUMP-NOTFOUND)
  message(FATAL_ERROR "Cannot find objdump executable (tried: ${THE_OBJDUMP}")
else()
  message(STATUS "[FAIL*] objdump binary -> ${FAIL_OBJDUMP}")
endif()
mark_as_advanced(FAIL_OBJDUMP)

# compile smarthops calculator if needed

if(CONFIG_INJECTIONPOINT_HOPS)
  add_subdirectory(smarthops)
  set(ADDITIONAL_LIBS fail-smarthops)
endif(CONFIG_INJECTIONPOINT_HOPS)

add_library(fail-util ${SRCS})
add_dependencies(fail-util fail-comm)
target_link_libraries(fail-util fail-comm fail-sal ${ADDITIONAL_LIBS} ${PROTOBUF_LIBRARY} ${Boost_LIBRARIES} ${LibIberty_LIBRARIES} ${ZLIB_LIBRARIES} ${LIBDWARF_LIBRARIES} ${LIBELF_LIBRARIES})

option(BUILD_CAPSTONE_DISASSEMBLER "Build the Capstone-based disassembler (Capstone 4.0.1 required, higher versions may not work)" OFF)
if (BUILD_CAPSTONE_DISASSEMBLER)
  add_subdirectory(capstonedisassembler)
endif (BUILD_CAPSTONE_DISASSEMBLER)

option(BUILD_LLVM_DISASSEMBLER "Build the LLVM-based disassembler (LLVM 3.9, 4.0, 5.0 or 6.0 preferred, other versions may not work)" OFF)
if (BUILD_LLVM_DISASSEMBLER)
  add_subdirectory(llvmdisassembler)
endif (BUILD_LLVM_DISASSEMBLER)

if (BUILD_CAPSTONE_DISASSEMBLER AND BUILD_LLVM_DISASSEMBLER)
  message(FATAL_ERROR "BUILD_CAPSTONE_DISASSEMBLER and BUILD_LLVM_DISASSEMBLER can't be on at the same time!")
endif (BUILD_CAPSTONE_DISASSEMBLER AND BUILD_LLVM_DISASSEMBLER)

### Tests
add_executable(memorymap-test testing/memorymap-test.cc)
target_link_libraries(memorymap-test fail-util)
add_test(NAME memorymap-test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/testing COMMAND memorymap-test)

add_executable(sumtree-test testing/SumTreeTest.cc)
target_link_libraries(sumtree-test fail-util)
add_test(NAME sumtree-test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/testing COMMAND sumtree-test)
