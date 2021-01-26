set(SRCS
  LLVMDisassembler.cpp
  LLVMDisassembler.hpp
  LLVMtoFailTranslator.cpp
  LLVMtoFailTranslator.hpp
  )

set(FIND_PACKAGE_VERSION 11.0.0)
set(FIND_PACKAGE_ARGS)

if(BUILD_BOCHS)
	set(SRCS ${SRCS}
		LLVMtoFailBochs.cpp
		LLVMtoFailBochs.hpp
		)
elseif (BUILD_GEM5)
	set(SRCS ${SRCS}
		LLVMtoFailGem5.cpp
		LLVMtoFailGem5.hpp
		)
elseif (BUILD_SAIL)
	set(SRCS ${SRCS}
		LLVMtoFailSailRiscv.cpp
		LLVMtoFailSailRiscv.hpp
		)
else()
	message( FATAL_ERROR "No supported architecture selected" )
endif(BUILD_BOCHS)

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs object support core mc AllTargetsDisassemblers AllTargetsDescs)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(fail-llvmdisassembler ${SRCS})
target_link_libraries(fail-llvmdisassembler fail-sal)
target_link_libraries(fail-llvmdisassembler ${llvm_libs})

### Tests
add_executable(llvmDisTest testing/llvmDisTest.cc)
target_link_libraries(llvmDisTest fail-llvmdisassembler)

add_test(NAME llvmDisx86Test COMMAND llvmDisTest ${CMAKE_CURRENT_SOURCE_DIR}/testing/x86 )
add_test(NAME llvmDisx86_64Test COMMAND llvmDisTest ${CMAKE_CURRENT_SOURCE_DIR}/testing/x86_64 )
add_test(NAME llvmDisARMM3Test COMMAND llvmDisTest ${CMAKE_CURRENT_SOURCE_DIR}/testing/armm3 )
add_test(NAME llvmDisARM9Test COMMAND llvmDisTest ${CMAKE_CURRENT_SOURCE_DIR}/testing/arm9 )
