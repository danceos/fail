set(EXPERIMENT_NAME mh-test-campaign)
set(EXPERIMENT_TYPE MHTestExperiment)
configure_file(../instantiate-experiment.ah.in
               ${CMAKE_CURRENT_BINARY_DIR}/instantiate-${EXPERIMENT_NAME}.ah @ONLY
)

## Setup desired protobuf descriptions HERE ##
set(MY_PROTOS 
	MHTest.proto
)

set(MY_CAMPAIGN_SRCS
	MHTestCampaign.hpp
	MHTestCampaign.cc
	experiment.hpp
	experiment.cc
)

#### PROTOBUFS ####
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${MY_PROTOS})

## Build library
add_library(fail-${EXPERIMENT_NAME} ${PROTO_SRCS} ${PROTO_HDRS} ${MY_CAMPAIGN_SRCS})
add_dependencies(fail-${EXPERIMENT_NAME} fail-comm)
target_link_libraries(fail-${EXPERIMENT_NAME} fail-comm)
target_link_libraries(fail-${EXPERIMENT_NAME} ${PROTOBUF_LIBRARY})

## This is the example's campaign server distributing experiment parameters
add_executable(${EXPERIMENT_NAME}-server mhcampaign.cc)
target_link_libraries(fail-${EXPERIMENT_NAME}-server fail ${PROTOBUF_LIBRARY} ${Boost_THREAD_LIBRARY})
install(TARGETS ${EXPERIMENT_NAME}-server RUNTIME DESTINATION bin)
