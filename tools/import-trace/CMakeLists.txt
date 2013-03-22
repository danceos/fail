## Setup desired protobuf descriptions HERE ##
set(MY_PROTOS
  ../../src/plugins/tracing/trace.proto
)

set(SRCS
  Importer.cc
  BasicImporter.cc
  DCiAOKernelImporter.cc
)

#### PROTOBUFS ####
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${MY_PROTOS})

## This is the example's campaign server distributing experiment parameters
add_executable(import-trace main.cc ${SRCS} ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(import-trace ${PROTOBUF_LIBRARY} -lmysqlclient fail-util fail-sal)
install(TARGETS import-trace RUNTIME DESTINATION bin)
