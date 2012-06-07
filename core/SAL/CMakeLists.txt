if(BUILD_BOCHS)
	set(SRCS
		Memory.cc
		Register.cc
		SimulatorController.cc
		bochs/BochsController.cc
	)
else()
	set(SRCS
		Memory.cc
		Register.cc
		SimulatorController.cc
		${VARIANT}/OVPController.cc
	)
endif(BUILD_BOCHS)


add_library(SAL ${SRCS})
