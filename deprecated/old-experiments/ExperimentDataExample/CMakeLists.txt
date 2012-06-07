## Setup desired protobuf descriptions HERE ##
set(MY_PROTOS 
	FaultCoverageExperiment.proto
)

set(SRCS
	example.cc
)

#### PROTOBUFS ####
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRNET_BINARY_DIR})

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS  ${MY_PROTOS}   )

## Build library
add_library(fcexperimentmessage  ${PROTO_SRCS} ${SRCS} )

