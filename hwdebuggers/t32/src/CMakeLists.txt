set(SRCS
		t32connector.cc
)

add_executable(fail-client ${SRCS})
target_link_libraries(fail-client -Wl,-whole-archive t32api -Wl,-no-whole-archive fail )

install(TARGETS fail-client RUNTIME DESTINATION bin)

