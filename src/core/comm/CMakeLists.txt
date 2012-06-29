set(SRCS
	ExperimentData.hpp
	SocketComm.hpp
	SocketComm.cc
)

## Setup desired protobuf descriptions HERE ##
set(MY_PROTOS
	FailControlMessage.proto
)

#### PROTOBUFS ####
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${MY_PROTOS})

add_library(comm ${SRCS} ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(comm ${PROTOBUF_LIBRARY})
