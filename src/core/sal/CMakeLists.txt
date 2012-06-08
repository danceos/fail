if(BUILD_BOCHS)
	set(SRCS
		BufferCache.cc
		Event.cc
		EventList.cc
		Memory.cc
		Register.cc
		SimulatorController.cc
		bochs/BochsController.cc
	)
else()
	set(SRCS
		BufferCache.cc
		Event.cc
		EventList.cc
		Memory.cc
		Register.cc
		SimulatorController.cc
		${VARIANT}/OVPController.cc
	)
endif(BUILD_BOCHS)

add_library(sal ${SRCS})

add_dependencies(sal efw)
