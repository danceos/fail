#FaultCoverage experiment
set(EXPERIMENT_NAME fault-coverage)
set(EXPERIMENT_TYPE FaultCoverageExperiment)
configure_file(../instantiate-experiment.ah.in
               ${CMAKE_CURRENT_BINARY_DIR}/instantiate-${EXPERIMENT_NAME}.ah @ONLY
)

#experiment sources
set(MY_EXPERIMENT_SRCS
	experiment.cc
	experiment.hpp
)

#### include directories ####
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## build library
add_library(fail-${EXPERIMENT_NAME} ${MY_EXPERIMENT_SRCS})
