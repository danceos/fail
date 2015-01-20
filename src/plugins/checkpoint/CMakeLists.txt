set(PLUGIN_NAME checkpoint)

set(MY_PLUGIN_SRCS
    Checkpoint.cc
    Checkpoint.hpp
    sha1.c
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## Build library
add_library(fail-${PLUGIN_NAME} ${MY_PLUGIN_SRCS})
