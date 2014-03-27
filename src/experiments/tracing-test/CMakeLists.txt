set(EXPERIMENT_NAME tracing-test)
set(EXPERIMENT_TYPE TracingTest)
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

target_link_libraries(fail-${EXPERIMENT_NAME} fail-tracing)
