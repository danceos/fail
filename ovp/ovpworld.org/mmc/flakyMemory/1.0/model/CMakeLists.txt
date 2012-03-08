set(SRCS
	flipBits.cpp
	model.cpp
)
add_definitions("-m32")
message(STATUS "flaky ld flags: ${HOST_LDFLAGS}")
add_library(flaky SHARED ${SRCS})

add_dependencies(flaky protomessages SAL)
set_target_properties(flaky PROPERTIES LINK_FLAGS "${HOST_LDFLAGS} -m32")

target_link_libraries(flaky ${IMPERAS_VMISTUBS} ${LIBRARY_OUTPUT_PATH}/libSAL.a)

