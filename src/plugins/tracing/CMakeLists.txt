set(PLUGIN_NAME tracing)

set(MY_PLUGIN_SRCS
	TracingPlugin.cc
	TracingPlugin.hpp
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

## Build library
add_library(fail-${PLUGIN_NAME} ${MY_PLUGIN_SRCS})
add_dependencies(fail-${PLUGIN_NAME} fail-comm)
target_link_libraries(fail-${PLUGIN_NAME} fail-sal fail-cpn fail-efw fail-comm)
