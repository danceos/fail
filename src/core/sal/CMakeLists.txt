if(BUILD_BOCHS)
	set(SRCS
		Listener.cc
		ListenerManager.cc
		SALConfig.cc
		Register.cc
		SimulatorController.cc
		perf/BreakpointBuffer.cc
		bochs/BochsController.cc
	)
elseif(BUILD_GEM5)
	set(SRCS
		Listener.cc
		ListenerManager.cc
		SALConfig.cc
		Register.cc
		SimulatorController.cc
		perf/BreakpointBuffer.cc
		gem5/Gem5Controller.cc
		gem5/Gem5PCEvent.cc
	)
elseif(BUILD_OVP)
	set(SRCS
		Listener.cc
		ListenerManager.cc
		SALConfig.cc
		Register.cc
		SimulatorController.cc
		perf/BreakpointBuffer.cc
		${VARIANT}/OVPController.cc
	)
elseif(BUILD_QEMU)
	set(SRCS
		Listener.cc
		ListenerManager.cc
		SALConfig.cc
		Register.cc
		perf/BreakpointBuffer.cc
		SimulatorController.cc
		qemu/QEMUController.cc
		qemu/wrappers.cc
	)
endif(BUILD_BOCHS)

add_library(fail-sal ${SRCS})

add_dependencies(fail-sal fail-efw)
