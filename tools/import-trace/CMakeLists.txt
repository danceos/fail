set(SRCS
  Importer.cc
  BasicImporter.cc
  DCiAOKernelImporter.cc
)

## This is the example's campaign server distributing experiment parameters
add_executable(import-trace main.cc ${SRCS} ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(import-trace ${PROTOBUF_LIBRARY} -lmysqlclient fail-util fail-sal fail-comm)
install(TARGETS import-trace RUNTIME DESTINATION bin)
