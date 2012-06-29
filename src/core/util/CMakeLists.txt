set(SRCS
	Logger.cc
	Logger.hpp
	MemoryMap.hpp
	ProtoStream.cc
	ProtoStream.hpp
	SynchronizedCounter.cc
	SynchronizedCounter.hpp
	SynchronizedMap.hpp
	SynchronizedQueue.hpp
)

# required by ProtoStream.cc:
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# required by Synchronized*.cc:
find_package(Boost 1.42 COMPONENTS thread REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

add_library(util ${SRCS})
target_link_libraries(util ${PROTOBUF_LIBRARY} ${Boost_THREAD_LIBRARY})
