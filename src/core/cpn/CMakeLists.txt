set(SRCS
	CampaignManager.cc
	JobServer.cc
	DatabaseCampaign.cc
)

find_package(MySQL REQUIRED)
include_directories(${MYSQL_INCLUDE_DIR})

add_library(fail-cpn ${SRCS})
target_link_libraries(fail-cpn fail-comm ${MYSQL_LIBRARIES})

# make sure protobufs are generated before we include them
add_dependencies(fail-cpn fail-comm)
