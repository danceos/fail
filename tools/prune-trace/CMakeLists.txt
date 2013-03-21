set(SRCS
  Pruner.cc
  BasicPruner.cc
)

## This is the example's campaign server distributing experiment parameters
add_executable(prune-trace main.cc ${SRCS})
target_link_libraries(prune-trace -lmysqlclient fail-util)
install(TARGETS prune-trace RUNTIME DESTINATION bin)
