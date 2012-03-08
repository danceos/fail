set(SRCS
	JobClient.cc
	JobServer.cc
	SocketComm.cc
)
add_subdirectory(messagedefs)
add_library(jobserver ${SRCS})
add_dependencies(jobserver protomessages)
