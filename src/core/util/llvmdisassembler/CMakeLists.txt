set(SRCS
  LLVMDisassembler.cpp
  LLVMDisassembler.hpp
  LLVMtoFailBochs.cpp
  LLVMtoFailBochs.hpp
  LLVMtoFailGem5.hpp
  LLVMtoFailGem5.cpp
  LLVMtoFailSailRiscv.hpp
  LLVMtoFailSailRiscv.cpp
  LLVMtoFailTranslator.cpp
  LLVMtoFailTranslator.hpp
)

    find_package(LLVM 11.0.0
    REQUIRED
    CONFIG
    PATHS /proj/opt/llvm-cheri-05.2020
    NO_DEFAULT_PATH
    )

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs object support core mc AllTargetsDisassemblers AllTargetsDescs)

add_library(fail-llvmdisassembler ${SRCS})
target_link_libraries(fail-llvmdisassembler PUBLIC fail-sal)
target_link_libraries(fail-llvmdisassembler PUBLIC ${llvm_libs})
if(BUILD_SAIL)
    ## require c++ 17 for this library
    target_compile_features(fail-llvmdisassembler PUBLIC cxx_std_17)
    # this option needs to be set so that boost-icl still compiles in c++17
endif(BUILD_SAIL)

### Tests
add_executable(llvmDisTest testing/llvmDisTest.cc)
target_link_libraries(llvmDisTest fail-llvmdisassembler)

add_test(NAME llvmDisx86Test COMMAND llvmDisTest ${CMAKE_CURRENT_SOURCE_DIR}/testing/x86 )
add_test(NAME llvmDisx86_64Test COMMAND llvmDisTest ${CMAKE_CURRENT_SOURCE_DIR}/testing/x86_64 )
add_test(NAME llvmDisARMM3Test COMMAND llvmDisTest ${CMAKE_CURRENT_SOURCE_DIR}/testing/armm3 )
add_test(NAME llvmDisARM9Test COMMAND llvmDisTest ${CMAKE_CURRENT_SOURCE_DIR}/testing/arm9 )
add_test(NAME llvmDisRISCV32Test COMMAND llvmDisTest ${CMAKE_CURRENT_SOURCE_DIR}/testing/riscv32 )
