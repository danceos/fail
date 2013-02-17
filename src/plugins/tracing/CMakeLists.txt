set(PLUGIN_NAME tracing)

#### PROTOBUFS ####
set(MY_PROTOS 
	trace.proto
)
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${MY_PROTOS})

set(MY_PLUGIN_SRCS
	TracingPlugin.cc
	TracingPlugin.hpp
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## Build library
add_library(fail-${PLUGIN_NAME} ${MY_PLUGIN_SRCS} ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(fail-${PLUGIN_NAME} ${PROTOBUF_LIBRARY})
