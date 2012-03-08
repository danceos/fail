## Setup desired protobuf descriptions HERE ##
set(MY_PROTOS 
	FailControlMessage.proto
)

#### PROTOBUFS ####
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS  ${MY_PROTOS}   )

## Build library
add_library(protomessages  ${PROTO_SRCS} ${PROTO_HDRS} )

