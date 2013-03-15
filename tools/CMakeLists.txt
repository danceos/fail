option(BUILD_IMPORT_TRACE "Build the trace import tool?" OFF)

### Setup search paths for headers ##
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../src/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/core)

if(BUILD_IMPORT_TRACE)
	add_subdirectory(import-trace)
endif(BUILD_IMPORT_TRACE)
