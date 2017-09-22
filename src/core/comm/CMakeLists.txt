set(SRCS
	ExperimentData.hpp
)

## Setup desired protobuf descriptions HERE ##
set(PROTOS
	FailControlMessage.proto
	${CMAKE_CURRENT_BINARY_DIR}/DatabaseCampaignMessage.proto
	TracePlugin.proto
)

## Set concrete implementation of InjectionPointMessage
# TODO: Define separate symbol, so it can also be used in other build types
if(CONFIG_INJECTIONPOINT_HOPS)
    set(PROTOS ${PROTOS} InjectionPointHopsMessage.proto)
    set(CONCRETE_INJECTION_POINT InjectionPointHopsMessage.proto)
else(CONFIG_INJECTIONPOINT_HOPS)
    set(PROTOS ${PROTOS} InjectionPointStepsMessage.proto)
    set(CONCRETE_INJECTION_POINT InjectionPointStepsMessage.proto)
endif(CONFIG_INJECTIONPOINT_HOPS)

# Configure concrete protobuf message to be included by campaign message
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DatabaseCampaignMessage.proto.in
       ${CMAKE_CURRENT_BINARY_DIR}/DatabaseCampaignMessage.proto)

#### PROTOBUFS ####
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# The PROTOBUF_IMPORT_DIRS is set in src/CMakeLists.txt
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${PROTOS})

add_custom_target(fail-protoc
  DEPENDS ${PROTO_SRCS} ${PROTO_HDRS}
)
add_library(fail-comm ${SRCS} ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(fail-comm ${PROTOBUF_LIBRARY})
