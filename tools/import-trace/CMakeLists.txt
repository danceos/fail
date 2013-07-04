set(SRCS
  Importer.cc
  MemoryImporter.cc
)

if (BUILD_LLVM_DISASSEMBLER)
  set(SRCS ${SRCS}
    InstructionImporter.cc
    RegisterImporter.cc
  )

  include(FindLLVM)

  # llvm-config does add -fno-exception to the command line. But this
  # breaks some boost libraries.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXX_FLAGS} -fexceptions")
endif(BUILD_LLVM_DISASSEMBLER)



add_executable(import-trace main.cc ${SRCS})

target_link_libraries(import-trace 
  ${PROTOBUF_LIBRARY} 
  -lmysqlclient 
  fail-util 
  fail-comm
  fail-sal)

if (BUILD_LLVM_DISASSEMBLER)
  target_link_libraries(import-trace fail-llvmdisassembler fail-sal ${LLVM_LIBS} ${LLVM_LDFLAGS})
endif (BUILD_LLVM_DISASSEMBLER)

install(TARGETS import-trace RUNTIME DESTINATION bin)
