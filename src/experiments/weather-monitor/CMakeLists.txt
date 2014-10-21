set(EXPERIMENT_NAME weather-monitor)
set(EXPERIMENT_TYPE WeatherMonitorExperiment)
configure_file(../instantiate-experiment.ah.in
               ${CMAKE_CURRENT_BINARY_DIR}/instantiate-${EXPERIMENT_NAME}.ah @ONLY
)

## Configuration
SET(PLUGINS_ACTIVATED "tracing" CACHE STRING "")

SET(bochs_configure_params "--enable-a20-pin;--enable-x86-64;--enable-cpu-level=6;--enable-ne2000;--enable-acpi;--enable-pci;--enable-usb;--enable-trace-cache;--enable-fast-function-calls;--enable-host-specific-asms;--enable-disasm;--enable-readline;--enable-clgd54xx;--enable-fpu;--enable-vmx=2;--enable-monitor-mwait;--enable-cdrom;--enable-sb16=linux;--enable-gdb-stub;--with-nogui" CACHE STRING "")

## Setup desired protobuf descriptions HERE ##
set(MY_PROTOS 
	weathermonitor.proto
)

set(MY_CAMPAIGN_SRCS
	experimentInfo.hpp
	experiment.hpp
	experiment.cc
	campaign.hpp
	campaign.cc
)

#### PROTOBUFS ####
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(PROTOBUF_IMPORT_DIRS ${PROTOBUF_IMPORT_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/../../core/comm)
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${MY_PROTOS})

## Build library
add_library(fail-${EXPERIMENT_NAME} ${PROTO_SRCS} ${PROTO_HDRS} ${MY_CAMPAIGN_SRCS})
add_dependencies(fail-${EXPERIMENT_NAME} fail-comm fail-util fail-sal fail-tracing)
target_link_libraries(fail-${EXPERIMENT_NAME} fail-comm fail-util fail-sal fail-tracing)
target_link_libraries(fail-${EXPERIMENT_NAME} ${PROTOBUF_LIBRARY})

## This is the example's campaign server distributing experiment parameters
add_executable(${EXPERIMENT_NAME}-server main.cc)
target_link_libraries(${EXPERIMENT_NAME}-server fail-${EXPERIMENT_NAME} fail fail-cpn fail-util -lmysqlclient_r ${PROTOBUF_LIBRARY} ${Boost_THREAD_LIBRARY})   
install(TARGETS ${EXPERIMENT_NAME}-server RUNTIME DESTINATION bin)
