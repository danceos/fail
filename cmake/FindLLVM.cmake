find_program(LLVMCONFIG NAMES llvm-config-8 llvm-config-7 llvm-config-6.0 llvm-config-5.0 llvm-config-4.0 llvm-config-3.9 llvm-config)

if( NOT LLVMCONFIG )
  message(FATAL_ERROR "llvm-config not found, try installing llvm-dev llvm")
else()
  message(STATUS "[FAIL*] LLVM Disassembler: Found llvm-config @ ${LLVMCONFIG}")
endif()

# examine LLVM include directory
execute_process( COMMAND ${LLVMCONFIG} --includedir
       OUTPUT_VARIABLE LLVM_INCLUDE_DIRS
       OUTPUT_STRIP_TRAILING_WHITESPACE )
include_directories( ${LLVM_INCLUDE_DIRS} )

# Library path
execute_process( COMMAND ${LLVMCONFIG} --libdir
       OUTPUT_VARIABLE LLVM_LIBRARY_DIRS
       OUTPUT_STRIP_TRAILING_WHITESPACE )
link_directories( ${LLVM_LIBRARY_DIRS} )

# necessary CPP flags.
execute_process( COMMAND ${LLVMCONFIG} --cxxflags
                 OUTPUT_VARIABLE LLVM_CXX_FLAGS
                 OUTPUT_STRIP_TRAILING_WHITESPACE )

# and additional libs (this is -ldl and -lpthread in llvm 3.1)
execute_process( COMMAND ${LLVMCONFIG} --ldflags
                 OUTPUT_VARIABLE LLVM_LDFLAGS
                 OUTPUT_STRIP_TRAILING_WHITESPACE )

## FIXME? Here we add *all* libraries to the link step (although we need only a handful..)
execute_process( COMMAND ${LLVMCONFIG} --libs all
                 OUTPUT_VARIABLE LLVM_LIBS
                 OUTPUT_STRIP_TRAILING_WHITESPACE )
