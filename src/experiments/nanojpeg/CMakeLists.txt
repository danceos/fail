set(EXPERIMENT_NAME nanojpeg)
set(EXPERIMENT_TYPE NanoJPEGExperiment)
configure_file(../instantiate-experiment-indirect.ah.in
               ${CMAKE_CURRENT_BINARY_DIR}/instantiate-${EXPERIMENT_NAME}.ah @ONLY
)
#add_aspect_headers(${CMAKE_CURRENT_BINARY_DIR}/instantiate-${EXPERIMENT_NAME}.ah)

## Setup desired protobuf descriptions HERE ##
set(MY_PROTOS 
	nanojpeg.proto
)

set(MY_CAMPAIGN_SRCS
	experimentInfo.hpp
	instantiateExperiment.cc
	experiment.hpp
	experiment.cc
	campaign.hpp
	campaign.cc
	UDIS86.hpp
	UDIS86.cc
	udis86_helper.hpp
	udis86_helper.cc
	psnr.cc
	psnr.hpp
)

#### PROTOBUFS ####
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${MY_PROTOS})

# dependency: libudis86
find_package(LibUdis86 REQUIRED)
include_directories(${LIBUDIS86_INCLUDE_DIRS})
link_directories(${LIBUDIS86_LINK_DIRS})

## Build library
add_library(fail-${EXPERIMENT_NAME} ${PROTO_SRCS} ${PROTO_HDRS} ${MY_CAMPAIGN_SRCS})
add_dependencies(fail-${EXPERIMENT_NAME} fail-comm)
target_link_libraries(fail-${EXPERIMENT_NAME} fail-tracing fail-comm)
target_link_libraries(fail-${EXPERIMENT_NAME} ${LIBUDIS86_LIBRARIES} ${PROTOBUF_LIBRARY})

## This is the example's campaign server distributing experiment parameters
add_executable(${EXPERIMENT_NAME}-server main.cc)
target_link_libraries(${EXPERIMENT_NAME}-server fail-${EXPERIMENT_NAME} fail ${Boost_THREAD_LIBRARY})   
install(TARGETS ${EXPERIMENT_NAME}-server RUNTIME DESTINATION bin)
