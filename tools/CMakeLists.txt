option(BUILD_DUMP_TRACE   "Build the trace dump tool?" OFF)
option(BUILD_IMPORT_TRACE "Build the trace import tool?" OFF)
option(BUILD_PRUNE_TRACE  "Build the trace prune tool?" OFF)


### Setup search paths for headers ##
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../src/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src/core)

if(BUILD_IMPORT_TRACE)
	add_subdirectory(import-trace)
endif(BUILD_IMPORT_TRACE)

if(BUILD_PRUNE_TRACE)
	add_subdirectory(prune-trace)
endif(BUILD_PRUNE_TRACE)

if(BUILD_DUMP_TRACE)
	add_subdirectory(dump-trace)
endif(BUILD_DUMP_TRACE)
