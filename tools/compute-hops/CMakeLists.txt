set(SRCS
  ResultCollector.cc
  SimpleAlgorithm.cc
  SmartAlgorithm.cc
)

if(NOT CONFIG_INJECTIONPOINT_HOPS)
  message (FATAL_ERROR "For building the compute-hops tool you need to enable CONFIG_INJECTIONPOINT_HOPS")
endif()

add_executable(compute-hops main.cc ${SRCS})
# add procps for built-in memory-usage measurements:
target_link_libraries(compute-hops ${PROTOBUF_LIBRARY} fail-util fail-comm)
install(TARGETS compute-hops RUNTIME DESTINATION bin)
