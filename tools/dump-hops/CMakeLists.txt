set(SRCS
  DumpHops.cc
)

if(NOT CONFIG_INJECTIONPOINT_HOPS)
  message (FATAL_ERROR "For building the dump-hops tool you need to enable CONFIG_INJECTIONPOINT_HOPS")
endif()

add_executable(dump-hops ${SRCS})
target_link_libraries(dump-hops ${PROTOBUF_LIBRARY} fail-util fail-comm)
install(TARGETS dump-hops RUNTIME DESTINATION bin)
