set(SRCS
  DumpTrace.cc
)

add_executable(dump-trace ${SRCS})
target_link_libraries(dump-trace ${PROTOBUF_LIBRARY} fail-util fail-comm)
install(TARGETS dump-trace RUNTIME DESTINATION bin)
