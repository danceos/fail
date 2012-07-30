if(BUILD_BOCHS)
	set(SRCS
		BufferCache.cc
		Listener.cc
		ListenerManager.cc
		SALConfig.cc
		Register.cc
		SimulatorController.cc
		bochs/BochsController.cc
		bochs/BochsListener.cc
	)
elseif(BUILD_OVP)
	set(SRCS
		BufferCache.cc
		Listener.cc
		ListenerManager.cc
		SALConfig.cc
		Register.cc
		SimulatorController.cc
		${VARIANT}/OVPController.cc
	)
elseif(BUILD_GEM5)
	set(SRCS
		BufferCache.cc
		Listener.cc
		ListenerManager.cc
		SALConfig.cc
		Register.cc
		SimulatorController.cc
		gem5/Gem5Controller.cc
	)
endif(BUILD_BOCHS)

add_library(sal ${SRCS})

add_dependencies(sal efw)
