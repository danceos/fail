set(SRCS
	BufferCache.cc
	CampaignManager.cc
	CoroutineManager.cc
	Event.cc
	EventList.cc
	ExperimentDataQueue.cc
	Signal.cc
	SynchronizedExperimentDataQueue.cc
)

add_library(controller ${SRCS})
