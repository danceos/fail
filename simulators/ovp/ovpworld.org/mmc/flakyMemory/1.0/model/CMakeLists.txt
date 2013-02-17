set(SRCS
	flipBits.cpp
	model.cpp
)
add_definitions("-m32")
message(STATUS "flaky ld flags: ${HOST_LDFLAGS}")
add_library(fail-flaky SHARED ${SRCS})

add_dependencies(fail-flaky fail-comm fail-sal)
set_target_properties(fail-flaky PROPERTIES LINK_FLAGS "${HOST_LDFLAGS} -m32")

target_link_libraries(fail-flaky ${IMPERAS_VMISTUBS} ${LIBRARY_OUTPUT_PATH}/libsal.a)
