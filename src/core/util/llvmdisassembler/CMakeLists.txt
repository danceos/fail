set(SRCS
  LLVMDisassembler.cpp
  LLVMDisassembler.hpp
  LLVMtoFailBochs.cpp
  LLVMtoFailBochs.hpp
  LLVMtoFailGem5.hpp
  LLVMtoFailGem5.cpp
  LLVMtoFailTranslator.cpp
  LLVMtoFailTranslator.hpp
)

include(FindLLVM)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXX_FLAGS}" )

add_library(fail-llvmdisassembler ${SRCS})
target_link_libraries(fail-llvmdisassembler fail-sal)
target_link_libraries(fail-llvmdisassembler ${LLVM_LIBS} ${LLVM_LDFLAGS} )

### Tests
add_executable(llvmDisTest testing/llvmDisTest.cc)
target_link_libraries(llvmDisTest fail-llvmdisassembler fail-sal)

add_test(NAME llvmDisx86Test COMMAND llvmDisTest ${CMAKE_CURRENT_SOURCE_DIR}/testing/x86 )
add_test(NAME llvmDisx86_64Test COMMAND llvmDisTest ${CMAKE_CURRENT_SOURCE_DIR}/testing/x86_64 )
add_test(NAME llvmDisARMM3Test COMMAND llvmDisTest ${CMAKE_CURRENT_SOURCE_DIR}/testing/armm3 )
add_test(NAME llvmDisARM9Test COMMAND llvmDisTest ${CMAKE_CURRENT_SOURCE_DIR}/testing/arm9 )
