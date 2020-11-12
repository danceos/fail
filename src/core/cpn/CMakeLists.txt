set(SRCS
	CampaignManager.cc
	JobServer.cc
	DatabaseCampaign.cc
)

if(BUILD_SAIL)
    ## when building for SAIL, we need c++17
    ## unfortunately this breaks boost::icl with gcc
    ## passing -fno-new-ttp-matching fixes this
    set_source_files_properties(DatabaseCampaign.cc PROPERTIES COMPILE_FLAGS -fno-new-ttp-matching)
endif(BUILD_SAIL)

find_package(MySQL REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MYSQL_CFLAGS}")

# only compile concrete implementation of InjectionPoint
if(CONFIG_INJECTIONPOINT_HOPS)
  set (SRCS ${SRCS} InjectionPointHops.cc)
else(CONFIG_INJECTIONPOINT_HOPS)
  set (SRCS ${SRCS} InjectionPointSteps.cc)
endif(CONFIG_INJECTIONPOINT_HOPS)

add_library(fail-cpn ${SRCS})
if(BUILD_SAIL)
    ## require c++ 17 for this library
    target_compile_features(fail-cpn PRIVATE cxx_std_17)
    # this option needs to be set so that boost-icl still compiles in c++17
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-new-ttp-matching")
endif(BUILD_SAIL)

target_link_libraries(fail-cpn fail-comm fail-util ${MYSQL_LIBRARIES} ${Boost_COROUTINE_LIBRARY} ${Boost_CONTEXT_LIBRARY})

# if hop-chains need to be calculated by the server, we
# the smarthopping module
if(CONFIG_INJECTIONPOINT_HOPS)
  add_dependencies(fail-cpn fail-smarthops)
endif(CONFIG_INJECTIONPOINT_HOPS)

# make sure protobufs are generated before we include them
add_dependencies(fail-cpn fail-comm)
