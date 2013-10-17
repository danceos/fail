set(PLUGIN_NAME realtimelogger)

set(MY_PLUGIN_SRCS
    RealtimeLogger.cc
    RealtimeLogger.hpp
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## Build library
add_library(fail-${PLUGIN_NAME} ${MY_PLUGIN_SRCS})
