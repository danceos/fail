set(SRCS
  Importer.cc
  BasicImporter.cc
)

add_executable(import-trace main.cc ${SRCS})
target_link_libraries(import-trace ${PROTOBUF_LIBRARY} -lmysqlclient fail-util fail-sal fail-comm)
install(TARGETS import-trace RUNTIME DESTINATION bin)
