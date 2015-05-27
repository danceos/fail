set(EXPERIMENT_NAME erika-tracing)
set(EXPERIMENT_TYPE ErikaTracing)
configure_file(../instantiate-experiment.ah.in
               ${CMAKE_CURRENT_BINARY_DIR}/instantiate-${EXPERIMENT_NAME}.ah @ONLY
)

set(MY_CAMPAIGN_SRCS
	experiment.hpp
	experiment.cc
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

## Build library
add_library(fail-${EXPERIMENT_NAME} ${PROTO_SRCS} ${PROTO_HDRS} ${MY_CAMPAIGN_SRCS})

add_dependencies(fail-${EXPERIMENT_NAME} fail-comm)
target_link_libraries(fail-${EXPERIMENT_NAME} fail-tracing fail-randomgenerator fail-checkpoint fail-comm)
