set(SRCS
  Importer.cc
  MemoryImporter.cc
  FullTraceImporter.cc
)

if (BUILD_LLVM_DISASSEMBLER)
  set(SRCS ${SRCS}
    InstructionImporter.cc
    RegisterImporter.cc
    RandomJumpImporter.cc
    AdvancedMemoryImporter.cc
    ElfImporter.cc
  )

  include(FindLLVM)

  # llvm-config does add -fno-exception to the command line. But this
  # breaks some boost libraries.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXX_FLAGS} -fexceptions")

  find_package(Boost 1.42 COMPONENTS regex REQUIRED)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})

  # libelf and libdwarf required by ElfImporter
  find_package(LibElf REQUIRED)
  find_package(LibDwarf REQUIRED)
  include_directories(${LIBELF_INCLUDE_DIRS})
  include_directories(${LIBDWARF_INCLUDE_DIRS})
endif(BUILD_LLVM_DISASSEMBLER)

if (BUILD_CAPSTONE_DISASSEMBLER)
  set(SRCS ${SRCS}
    InstructionImporter.cc
    RegisterImporter.cc
    RandomJumpImporter.cc
    AdvancedMemoryImporter.cc
    ElfImporter.cc
  )

  include(FindCapstone)

  # libelf and libdwarf required by ElfImporter
  find_package(LibElf REQUIRED)
  find_package(LibDwarf REQUIRED)
  include_directories(${LIBELF_INCLUDE_DIRS})
  include_directories(${LIBDWARF_INCLUDE_DIRS})
endif(BUILD_CAPSTONE_DISASSEMBLER)

find_package(MySQL REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MYSQL_CFLAGS}")

add_executable(import-trace main.cc ${SRCS})

target_link_libraries(import-trace 
  ${PROTOBUF_LIBRARY} 
  ${MYSQL_LIBRARIES}
  fail-util 
  fail-comm
  fail-sal
)

if (BUILD_LLVM_DISASSEMBLER)
  target_link_libraries(import-trace fail-llvmdisassembler fail-sal ${LLVM_LIBS} ${LLVM_LDFLAGS} ${Boost_LIBRARIES})
endif (BUILD_LLVM_DISASSEMBLER)

if (BUILD_CAPSTONE_DISASSEMBLER)
  target_link_libraries(import-trace fail-capstonedisassembler fail-sal)
endif (BUILD_CAPSTONE_DISASSEMBLER)

install(TARGETS import-trace RUNTIME DESTINATION bin)
install(PROGRAMS import-symbols.sh DESTINATION bin)
