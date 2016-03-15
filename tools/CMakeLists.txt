option(BUILD_DUMP_TRACE   "Build the trace dump tool?" OFF)
option(BUILD_IMPORT_TRACE "Build the trace import tool?" OFF)
option(BUILD_PRUNE_TRACE  "Build the trace prune tool?" OFF)
option(BUILD_CONVERT_TRACE "Build the trace converter tool?" OFF)

option(BUILD_COMPUTE_HOPS  "Build the compute hops tool?" OFF)
option(BUILD_DUMP_HOPS  "Build the hops dump tool?" OFF)

option(BUILD_FAULTSPACEPLOT "Build the faultspace plotting tool?" OFF)
option(BUILD_DATA_AGGREGATOR "Build the data aggregation tools?" OFF)

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

if(BUILD_CONVERT_TRACE)
	add_subdirectory(convert-trace)
endif(BUILD_CONVERT_TRACE)

if(BUILD_COMPUTE_HOPS)
	add_subdirectory(compute-hops)
endif(BUILD_COMPUTE_HOPS)

if(BUILD_DUMP_HOPS)
	add_subdirectory(dump-hops)
endif(BUILD_DUMP_HOPS)

if(BUILD_FAULTSPACEPLOT)
	add_subdirectory(analysis/faultspaceplot)
endif(BUILD_FAULTSPACEPLOT)

if(BUILD_DATA_AGGREGATOR)
	add_subdirectory(analysis/data-aggregator)
endif(BUILD_DATA_AGGREGATOR)
