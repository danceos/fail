set(SRCS
  Pruner.cc
  BasicPruner.cc
)

find_package(MySQL REQUIRED)
include_directories(${MYSQL_INCLUDE_DIR})

## This is the example's campaign server distributing experiment parameters
add_executable(prune-trace main.cc ${SRCS})
target_link_libraries(prune-trace ${MYSQL_LIBRARIES} fail-util)
install(TARGETS prune-trace RUNTIME DESTINATION bin)
