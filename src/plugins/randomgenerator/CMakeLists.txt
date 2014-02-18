set(PLUGIN_NAME randomgenerator)

set(MY_PLUGIN_SRCS
  RandomGenerator.cc
  RandomGenerator.hpp
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## Build library
add_library(fail-${PLUGIN_NAME} ${MY_PLUGIN_SRCS})
