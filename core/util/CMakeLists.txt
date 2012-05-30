set(SRCS
	SynchronizedCounter.cc
	Logger.hpp
	Logger.cc
	ProtoStream.cc
)

add_library(util ${SRCS})
