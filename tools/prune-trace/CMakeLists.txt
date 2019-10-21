set(SRCS
  Pruner.cc
  BasicPruner.cc
  FESamplingPruner.cc
  SamplingPruner.cc
)

find_package(MySQL REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MYSQL_CFLAGS}")

## This is the example's campaign server distributing experiment parameters
add_executable(prune-trace main.cc ${SRCS})
target_link_libraries(prune-trace ${MYSQL_LIBRARIES} fail-util)
install(TARGETS prune-trace RUNTIME DESTINATION bin)
