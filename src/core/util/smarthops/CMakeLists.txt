set(SRCS
  SmartHops.cc
  SmartHops.hpp
  TraceReader.cc
  TraceReader.hpp
)

add_library(fail-smarthops ${SRCS})
# ToDo: Do we really need this dependency?
add_dependencies(fail-smarthops fail-comm)