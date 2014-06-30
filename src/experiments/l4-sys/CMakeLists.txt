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
	aluinstr.cc
	experiment.hpp
	experiment.cc
	experimentFI.cc
	experimentParameter.cc
	experimentPreparation.cc
	campaign.hpp
	campaign.cc
	UDIS86.hpp
	UDIS86.cc
	InstructionFilter.hpp
	InstructionFilter.cc
)

#### PROTOBUFS ####
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(PROTOBUF_IMPORT_DIRS ${PROTOBUF_IMPORT_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/../../core/comm)
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${MY_PROTOS})

## Build library
add_library(fail-${EXPERIMENT_NAME} ${PROTO_SRCS} ${PROTO_HDRS} ${MY_CAMPAIGN_SRCS})

find_package(LibUdis86 REQUIRED)
include_directories(${LIBUDIS86_INCLUDE_DIRS})
link_directories(${LIBUDIS86_LINK_DIRS})
add_dependencies(fail-${EXPERIMENT_NAME} fail-comm)
target_link_libraries(fail-${EXPERIMENT_NAME} fail-comm fail-efw fail-util fail-sal)
target_link_libraries(fail-${EXPERIMENT_NAME} ${LIBUDIS86_LIBRARIES} ${PROTOBUF_LIBRARY})

## This is the example's campaign server distributing experiment parameters
add_executable(${EXPERIMENT_NAME}-server main.cc)
target_link_libraries(${EXPERIMENT_NAME}-server fail-${EXPERIMENT_NAME} -Wl,--start-group fail-sal fail-util fail-cpn fail-comm ${PROTOBUF_LIBRARY} ${Boost_THREAD_LIBRARY} -lmysqlclient_r -Wl,--end-group)
install(TARGETS ${EXPERIMENT_NAME}-server RUNTIME DESTINATION bin)
