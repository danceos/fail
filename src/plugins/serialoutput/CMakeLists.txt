set(PLUGIN_NAME serialoutput)

set(MY_PLUGIN_SRCS
	SerialOutputLogger.cc
	SerialOutputLogger.hpp
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## Build library
add_library(fail-${PLUGIN_NAME} ${MY_PLUGIN_SRCS})
add_dependencies(fail-${PLUGIN_NAME} fail-efw)
target_link_libraries(fail-${PLUGIN_NAME} fail-efw)
