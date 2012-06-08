set(SRCS
	CoroutineManager.cc
	JobClient.cc
)

# FIXME: Add dependency check for pcl-library here.

add_library(efw ${SRCS})

add_dependencies(efw comm)
