set(SRCS
  main.cc
  Gem5Converter.cc
  DumpConverter.cc
)

add_executable(convert-trace ${SRCS})
target_link_libraries(convert-trace fail-util fail-comm)

install(TARGETS convert-trace RUNTIME DESTINATION bin)
