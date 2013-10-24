set(SRCS
	ExperimentData.hpp
	SocketComm.hpp
	SocketComm.cc
)

## Setup desired protobuf descriptions HERE ##
set(PROTOS
	FailControlMessage.proto
	DatabaseCampaignMessage.proto
	TracePlugin.proto
)

#### PROTOBUFS ####
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${PROTOS})

add_library(fail-comm ${SRCS} ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(fail-comm ${PROTOBUF_LIBRARY})
