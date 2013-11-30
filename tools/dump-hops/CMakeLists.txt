set(SRCS
  DumpHops.cc
)

add_executable(dump-hops ${SRCS})
target_link_libraries(dump-hops ${PROTOBUF_LIBRARY} fail-util fail-comm)
install(TARGETS dump-hops RUNTIME DESTINATION bin)
