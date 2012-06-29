set(SRCS
	CoroutineManager.hpp
	CoroutineManager.cc
	ExperimentFlow.hpp
	JobClient.hpp
	JobClient.cc
)

add_library(efw ${SRCS})
add_dependencies(efw comm)

find_package(LibPCL REQUIRED)
include_directories(${LIBPCL_INCLUDE_DIRS})
link_directories(${LIBPCL_LINK_DIRS})
target_link_libraries(efw ${LIBPCL_LIBRARIES})
