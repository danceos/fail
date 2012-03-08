set(SRCS
	Memory.cc
	Register.cc
	SimulatorController.cc
	${VARIANT}/Controller.cc
)


add_library(SAL ${SRCS})

