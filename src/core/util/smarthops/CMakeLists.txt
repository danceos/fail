set(SRCS
  SmartHops.cc
  SmartHops.hpp
  TraceReader.cc
  TraceReader.hpp
)

add_library(fail-smarthops ${SRCS})
# make sure comm/TracePlugin.pb.h has been generated
add_dependencies(fail-smarthops fail-comm)
# fail-util: ProtoIStream
target_link_libraries(fail-smarthops fail-comm fail-util)
