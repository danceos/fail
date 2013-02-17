## Setup desired protobuf descriptions HERE ##
#set(MY_PROTOS 
#	TestData.proto
#)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

#find_package (LibElf REQUIRED)
#find_package (LibDwarf REQUIRED)

#### PROTOBUFS ####
#find_package(Protobuf REQUIRED)
#include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_CURERNT_BINARY_DIR})

#PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS  ${MY_PROTOS}   )

## Build library
#add_library(testmessages  ${PROTO_SRCS})

## Add some tests 
#add_executable(testclient client.cc )
#add_executable(testclient testjc.cc )
#add_executable(testserver server.cc)
#target_link_libraries(testclient fail ${PROTOBUF_LIBRARY} ${CMAKE_THREAD_LIBS_INIT}  anexperimentmessage protomessages)
#target_link_libraries(testserver fail ${PROTOBUF_LIBRARY} ${CMAKE_THREAD_LIBS_INIT}  anexperimentmessage protomessages)


#add_executable(dwarf dwarf.cc)
#target_link_libraries(dwarf  ${LIBDWARF_LIBRARIES} ${LIBELF_LIBRARIES}  )
#include_directories(${CMAKE_BINARY_DIR}/core/experiments/MHTestCampaign)
#add_executable(mhcampaign mhcampaign.cc)
#target_link_libraries(mhcampaign mhtestcampaign fail ${PROTOBUF_LIBRARY}  ${Boost_THREAD_LIBRARY})
