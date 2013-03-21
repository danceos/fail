set(SRCS
		main.cc
    T32Connector.cc
    T32TraceFormat.cc
    T32Tracer.cc
)

add_executable(fail-client ${SRCS})
target_link_libraries(fail-client -Wl,-whole-archive t32api -Wl,-no-whole-archive fail )

install(TARGETS fail-client RUNTIME DESTINATION bin)

add_executable(t32cli t32cli.cc)
target_link_libraries(t32cli t32api)

