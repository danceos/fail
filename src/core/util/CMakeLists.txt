set(SRCS
	SynchronizedCounter.cc
	Logger.hpp
	Logger.cc
	ProtoStream.cc
)

add_library(util ${SRCS})

# FIXME: Add protobuf-dependency (required by ProtoStream.cc@line56)
