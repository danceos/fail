set(SRCS
	CampaignManager.cc
	JobServer.cc
	DatabaseCampaign.cc
)

find_package(MySQL REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MYSQL_CFLAGS}")

# only compile concrete implementation of InjectionPoint
if(CONFIG_INJECTIONPOINT_HOPS)
  set (SRCS ${SRCS} InjectionPointHops.cc)
else(CONFIG_INJECTIONPOINT_HOPS)
  set (SRCS ${SRCS} InjectionPointSteps.cc)
endif(CONFIG_INJECTIONPOINT_HOPS)

add_library(fail-cpn ${SRCS})
target_link_libraries(fail-cpn fail-comm fail-util ${MYSQL_LIBRARIES} ${Boost_COROUTINE_LIBRARY} ${Boost_CONTEXT_LIBRARY})

# if hop-chains need to be calculated by the server, we
# the smarthopping module
if(CONFIG_INJECTIONPOINT_HOPS)
  add_dependencies(fail-cpn fail-smarthops)
endif(CONFIG_INJECTIONPOINT_HOPS)

# make sure protobufs are generated before we include them
add_dependencies(fail-cpn fail-comm)
