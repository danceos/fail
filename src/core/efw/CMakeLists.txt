set(SRCS
	CoroutineManager.hpp
	CoroutineManager.cc
	ExperimentFlow.hpp
	JobClient.hpp
	JobClient.cc
	DatabaseExperiment.hpp
	DatabaseExperiment.cc
)

set_source_files_properties(JobClient.cc PROPERTIES COMPILE_FLAGS -std=c++11)

add_library(fail-efw ${SRCS})
add_dependencies(fail-efw fail-protoc)
target_link_libraries(fail-efw fail-comm)
target_link_libraries(fail-efw fail-util) # WallclockTimer
if (BUILD_LLVM_DISASSEMBLER)
  target_link_libraries(fail-efw fail-llvmdisassembler)
endif()
if (BUILD_CAPSTONE_DISASSEMBLER)
	target_link_libraries(fail-efw fail-capstonedisassembler)
endif()

find_package(LibPCL REQUIRED)
include_directories(${LIBPCL_INCLUDE_DIRS})
link_directories(${LIBPCL_LINK_DIRS})
target_link_libraries(fail-efw ${LIBPCL_LIBRARIES})
