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

    find_package(LLVM 11.0.0
        REQUIRED
        CONFIG
        PATHS /proj/opt/llvm-cheri-05.2020
        NO_DEFAULT_PATH
        )

    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

    ## Set your project compile flags.
    ## E.g. if using the C++ header files
    ## you will need to enable C++11 support
    ## for your compiler.

    include_directories(${LLVM_INCLUDE_DIRS})
    add_definitions(${LLVM_DEFINITIONS})

    ## Find the libraries that correspond to the LLVM components
    ## that we wish to use
    llvm_map_components_to_libnames(llvm_libs object support core mc AllTargetsDisassemblers AllTargetsInfos AllTargetsDescs)

    find_package(Boost 1.42 COMPONENTS regex REQUIRED)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})

    # libelf and libdwarf required by ElfImporter
    find_package(LibElf REQUIRED)
    find_package(LibDwarf REQUIRED)
    include_directories(${LIBELF_INCLUDE_DIRS})
    include_directories(${LIBDWARF_INCLUDE_DIRS})

endif(BUILD_LLVM_DISASSEMBLER)

find_package(MySQL REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MYSQL_CFLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-new-ttp-matching")

add_executable(import-trace main.cc ${SRCS})

target_link_libraries(import-trace 
  ${PROTOBUF_LIBRARY}
  ${MYSQL_LIBRARIES}
  fail-util
  fail-comm
  fail-sal
)

if (BUILD_LLVM_DISASSEMBLER)
  target_link_libraries(import-trace
      fail-llvmdisassembler
      fail-sal
      ${llvm_libs} ${Boost_LIBRARIES})
endif (BUILD_LLVM_DISASSEMBLER)

install(TARGETS import-trace RUNTIME DESTINATION bin)
install(PROGRAMS import-symbols.sh DESTINATION bin)
