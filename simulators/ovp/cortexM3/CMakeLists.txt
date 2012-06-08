add_definitions(" -m32 ${IMPERAS_VMIINC} ")

set(SRCS
	$ENV{IMPERAS_HOME}/ImpPublic/source/host/icm/icmCpuManager.cpp
	platform/platform.cc
	../OVPPlatform.cc
)

add_executable(ovp_cortexM3 ${SRCS})
add_dependencies(ovp_cortexM3 ovpstatusmessages )


target_link_libraries(ovp_cortexM3 ${SIM_LDFLAGS} sal cpn comm efw util ovpstatusmessages)
## OVP links all needed shared libraries via a runtimeloader
set_target_properties(ovp_cortexM3 PROPERTIES LINK_FLAGS "-L${CMAKE_BINARY_DIR}/lib -lpcl -lprotobuf -m32 ")
