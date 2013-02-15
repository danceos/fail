set(SRCS
		T32Connector.cc
)

add_executable(fail-client ${SRCS})
target_link_libraries(fail-client -Wl,-whole-archive t32api -Wl,-no-whole-archive fail )

install(TARGETS fail-client RUNTIME DESTINATION bin)

add_executable(t32cli t32cli.cc)
target_link_libraries(t32cli -Wl,-whole-archive t32api -Wl,-no-whole-archive)
