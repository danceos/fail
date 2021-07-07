set(SRCS)

if(BUILD_RISCV_CHERI)
	set(SRCS ${SRCS}
		${SAIL_ARCH}/SailTagMemoryManager.cc)
endif(BUILD_RISCV_CHERI)

add_library(fail-sail
	# Architecture independent
	SailArchitecture.cc
	SailCPU.cc
	SailSimulator.cc
	SailMemoryManager.cc
	SailFailInterface.cc
	# Architecture dependend:
	${SAIL_ARCH}/SailArchRegisters.cc
	${SAIL_ARCH}/SailArchFaultSpace.cc
	${SAIL_ARCH}/SailArchSimulator.cc
	${SRCS}
)

target_link_libraries(fail-sail fail-fsp)
