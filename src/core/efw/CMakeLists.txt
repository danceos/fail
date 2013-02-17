set(SRCS
	CoroutineManager.hpp
	CoroutineManager.cc
	ExperimentFlow.hpp
	JobClient.hpp
	JobClient.cc
)

add_library(fail-efw ${SRCS})
add_dependencies(fail-efw fail-comm)

find_package(LibPCL REQUIRED)
include_directories(${LIBPCL_INCLUDE_DIRS})
link_directories(${LIBPCL_LINK_DIRS})
target_link_libraries(fail-efw ${LIBPCL_LIBRARIES})
