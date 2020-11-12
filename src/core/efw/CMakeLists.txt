set(SRCS
	CoroutineManager.hpp
	CoroutineManager.cc
	ExperimentFlow.hpp
	JobClient.hpp
	JobClient.cc
	DatabaseExperiment.hpp
	DatabaseExperiment.cc
)

add_library(fail-efw ${SRCS})
if(BUILD_SAIL)
    ## require c++ 17 for this library
    target_compile_features(fail-efw PUBLIC cxx_std_17)
endif(BUILD_SAIL)
add_dependencies(fail-efw fail-protoc)
target_link_libraries(fail-efw fail-comm)
target_link_libraries(fail-efw fail-util) # WallclockTimer
if (BUILD_LLVM_DISASSEMBLER)
  target_link_libraries(fail-efw fail-llvmdisassembler)
endif()

find_package(LibPCL REQUIRED)
include_directories(${LIBPCL_INCLUDE_DIRS})
link_directories(${LIBPCL_LINK_DIRS})
target_link_libraries(fail-efw ${LIBPCL_LIBRARIES})
