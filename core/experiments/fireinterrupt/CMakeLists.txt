set(EXPERIMENT_NAME fireinterrupt)
set(EXPERIMENT_TYPE fireinterruptExperiment)
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
add_library(${EXPERIMENT_NAME} ${MY_EXPERIMENT_SRCS})
