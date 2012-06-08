set(SRCS
	SocketComm.cc
)

add_subdirectory(msg)

add_library(comm ${SRCS})
add_dependencies(comm msg)
