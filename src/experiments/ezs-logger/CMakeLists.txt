set(EXPERIMENT_NAME ezs-logger)
set(EXPERIMENT_TYPE EZSLogger)
configure_file(../instantiate-experiment.ah.in
               ${CMAKE_CURRENT_BINARY_DIR}/instantiate-${EXPERIMENT_NAME}.ah @ONLY
)

## Setup desired protobuf descriptions HERE ##
set(MY_PROTOS
)

set(MY_CAMPAIGN_SRCS
	experiment.hpp
	experiment.cc
)

## Build library
add_library(fail-${EXPERIMENT_NAME}  ${MY_CAMPAIGN_SRCS})
add_dependencies(fail-${EXPERIMENT_NAME} fail-comm)
target_link_libraries(fail-${EXPERIMENT_NAME} fail-comm fail-util)

