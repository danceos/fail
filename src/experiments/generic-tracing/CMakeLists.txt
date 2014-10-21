set(EXPERIMENT_NAME generic-tracing)
set(EXPERIMENT_TYPE GenericTracing)
configure_file(../instantiate-experiment.ah.in
               ${CMAKE_CURRENT_BINARY_DIR}/instantiate-${EXPERIMENT_NAME}.ah @ONLY
)

set(MY_CAMPAIGN_SRCS
	experiment.hpp
	experiment.cc
)

SET(PLUGINS_ACTIVATED "tracing" CACHE STRING "")

SET(bochs_configure_params "--enable-a20-pin;--enable-x86-64;--enable-cpu-level=6;--enable-ne2000;--enable-acpi;--enable-pci;--enable-usb;--enable-trace-cache;--enable-fast-function-calls;--enable-host-specific-asms;--enable-readline;--enable-clgd54xx;--enable-fpu;--enable-vmx=2;--enable-monitor-mwait;--enable-cdrom;--enable-sb16=linux;--enable-gdb-stub;--with-nogui" CACHE STRING "")

# Build the import and prune trace tools always
SET(BUILD_IMPORT_TRACE      ON CACHE BOOL "" FORCE)
SET(BUILD_PRUNE_TRACE       ON CACHE BOOL "" FORCE)
SET(BUILD_CONVERT_TRACE     ON CACHE BOOL "" FORCE)
SET(BUILD_DUMP_TRACE        ON CACHE BOOL "" FORCE)
SET(BUILD_LLVM_DISASSEMBLER ON CACHE BOOL "" FORCE)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

## Build library
add_library(fail-${EXPERIMENT_NAME} ${PROTO_SRCS} ${PROTO_HDRS} ${MY_CAMPAIGN_SRCS})

add_dependencies(fail-${EXPERIMENT_NAME} fail-comm)
target_link_libraries(fail-${EXPERIMENT_NAME} fail-tracing fail-comm)
