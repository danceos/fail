set(SRCS
	ElfReader.cc
	ElfReader.hpp
  Demangler.hpp
  Demangler.cc
	elfinfo/elfinfo.cc
	elfinfo/elfinfo.h
	gzstream/gzstream.C
	gzstream/gzstream.h
	Logger.cc
	Logger.hpp
	MemoryMap.hpp
	ProtoStream.cc
	ProtoStream.hpp
	SynchronizedCounter.cc
	SynchronizedCounter.hpp
	SynchronizedMap.hpp
	SynchronizedQueue.hpp
	WallclockTimer.cc
	WallclockTimer.hpp
)

# required by ProtoStream.cc:
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# required by Synchronized*.cc:
find_package(Boost 1.42 COMPONENTS thread REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# libiberty required by Demangler.cc:
find_library(LIB_IBERTY iberty)

if(${LIB_IBERTY} STREQUAL LIB_IBERTY-NOTFOUND)
  message(FATAL_ERROR "libiberty not found. Try installing binutils-dev: [ sudo aptitude install binutils-dev ]")
endif()

add_library(fail-util ${SRCS})
target_link_libraries(fail-util ${PROTOBUF_LIBRARY} ${Boost_THREAD_LIBRARY} ${LIB_IBERTY})
