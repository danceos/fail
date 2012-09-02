set(EXPERIMENT_NAME l4-sys)
set(EXPERIMENT_TYPE L4SysExperiment)
configure_file(../instantiate-experiment.ah.in
               ${CMAKE_CURRENT_BINARY_DIR}/instantiate-${EXPERIMENT_NAME}.ah @ONLY
)

## Setup desired protobuf descriptions HERE ##
set(MY_PROTOS 
	l4sys.proto
)

set(MY_CAMPAIGN_SRCS
	aluinstr.hpp
	experiment.hpp
	experiment.cc
	campaign.hpp
	campaign.cc
	UDIS86.hpp
	UDIS86.cc
)

#### PROTOBUFS ####
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${MY_PROTOS})

## Build library
add_library(${EXPERIMENT_NAME} ${PROTO_SRCS} ${PROTO_HDRS} ${MY_CAMPAIGN_SRCS})

find_package(LibUdis86 REQUIRED)
include_directories(${LIBUDIS86_INCLUDE_DIRS})
link_directories(${LIBUDIS86_LINK_DIRS})
target_link_libraries(${EXPERIMENT_NAME} ${LIBUDIS86_LIBRARIES})

## This is the example's campaign server distributing experiment parameters
add_executable(${EXPERIMENT_NAME}-server main.cc)
target_link_libraries(${EXPERIMENT_NAME}-server ${EXPERIMENT_NAME} fail ${PROTOBUF_LIBRARY} ${Boost_THREAD_LIBRARY})   
install(TARGETS ${EXPERIMENT_NAME}-server RUNTIME DESTINATION bin)
