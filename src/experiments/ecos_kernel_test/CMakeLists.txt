set(EXPERIMENT_NAME ecos_kernel_test)
set(EXPERIMENT_TYPE EcosKernelTestExperiment)
configure_file(../instantiate-experiment.ah.in
               ${CMAKE_CURRENT_BINARY_DIR}/instantiate-${EXPERIMENT_NAME}.ah @ONLY
)

## Setup desired protobuf descriptions HERE ##
set(MY_PROTOS 
	ecos_kernel_test.proto
)

set(MY_CAMPAIGN_SRCS
	experiment.hpp
	experiment.cc
	campaign.hpp
	campaign.cc
)

#### PROTOBUFS ####
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${MY_PROTOS})

## Build library
add_library(${EXPERIMENT_NAME} ${PROTO_SRCS} ${PROTO_HDRS} ${MY_CAMPAIGN_SRCS})
add_dependencies(${EXPERIMENT_NAME} tracing)

## This is the example's campaign server distributing experiment parameters
add_executable(${EXPERIMENT_NAME}-server main.cc)
target_link_libraries(${EXPERIMENT_NAME}-server ${EXPERIMENT_NAME} fail ${PROTOBUF_LIBRARY} ${Boost_THREAD_LIBRARY})   
install(TARGETS ${EXPERIMENT_NAME}-server RUNTIME DESTINATION bin)
