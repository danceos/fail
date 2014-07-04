set(EXPERIMENT_NAME regression-test)
set(EXPERIMENT_TYPE RegressionTest)
configure_file(../instantiate-experiment.ah.in
               ${CMAKE_CURRENT_BINARY_DIR}/instantiate-${EXPERIMENT_NAME}.ah @ONLY
)

#experiment sources
set(MY_EXPERIMENT_SRCS
	experiment.hpp
	experiment.cc
)

#### include directories ####
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## build library
add_library(fail-${EXPERIMENT_NAME} ${MY_EXPERIMENT_SRCS})
add_dependencies(fail-${EXPERIMENT_NAME} fail-comm)
target_link_libraries(fail-${EXPERIMENT_NAME} fail-tracing fail-serialoutput fail-comm fail-util)
target_link_libraries(fail-${EXPERIMENT_NAME} ${PROTOBUF_LIBRARY})
