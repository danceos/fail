set(SRCS
	CampaignManager.cc
	JobServer.cc
)

add_library(fail-cpn ${SRCS})

add_dependencies(fail-cpn fail-comm)
