set(SRCS
	CampaignManager.cc
	JobServer.cc
)

add_library(cpn ${SRCS})

add_dependencies(cpn comm)
