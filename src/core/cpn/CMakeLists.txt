set(SRCS
	CampaignManager.cc
	JobServer.cc
	DatabaseCampaign.cc
)

# only compile concrete implementation of InjectionPoint
if(CONFIG_INJECTIONPOINT_HOPS)
    set (SRCS ${SRCS} InjectionPointHops.cc) 
else(CONFIG_INJECTIONPOINT_HOPS)
    set (SRCS ${SRCS} InjectionPointSteps.cc)
endif(CONFIG_INJECTIONPOINT_HOPS)

add_library(fail-cpn ${SRCS})

# if hop-chains need to be calculated by the server, we 
# the smarthopping module
if(CONFIG_INJECTIONPOINT_HOPS)
    add_dependencies(fail-cpn fail-smarthops)
endif(CONFIG_INJECTIONPOINT_HOPS)

add_dependencies(fail-cpn fail-comm)
