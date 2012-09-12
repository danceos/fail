add_definitions(" -m32 ${IMPERAS_VMIINC} ")

set(SRCS
	$ENV{IMPERAS_HOME}/ImpPublic/source/host/icm/icmCpuManager.cpp
	platform/flakyMemory.cpp
	platform/beforeInstruction.cpp
	platform/platform.cpp
)
add_executable(ovp ${SRCS})
add_dependencies(ovp fail-comm)
target_link_libraries(ovp ${SIM_LDFLAGS} )
## OVP links all needed shared libraries via a runtimeloader
set_target_properties(ovp PROPERTIES LINK_FLAGS " -m32 ")
