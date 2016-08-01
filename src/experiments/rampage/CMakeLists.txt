set(EXPERIMENT_NAME rampage)
set(EXPERIMENT_TYPE RAMpageExperiment)
configure_file(../instantiate-experiment-indirect.ah.in
               ${CMAKE_CURRENT_BINARY_DIR}/instantiate-${EXPERIMENT_NAME}.ah @ONLY
)

set(MY_PROTOS 
	rampage.proto
)

set(MY_CAMPAIGN_SRCS
	instantiateExperiment.cc
	experimentInfo.hpp
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
add_library(fail-${EXPERIMENT_NAME} ${PROTO_SRCS} ${PROTO_HDRS} ${MY_CAMPAIGN_SRCS})
add_dependencies(fail-${EXPERIMENT_NAME} fail-comm)
target_link_libraries(fail-${EXPERIMENT_NAME} fail-comm fail-util fail-efw fail-sal)
target_link_libraries(fail-${EXPERIMENT_NAME} ${PROTOBUF_LIBRARY})

## This is the example's campaign server distributing experiment parameters
add_executable(${EXPERIMENT_NAME}-server main.cc)
target_link_libraries(${EXPERIMENT_NAME}-server fail-${EXPERIMENT_NAME} fail)
install(TARGETS ${EXPERIMENT_NAME}-server RUNTIME DESTINATION bin)
