if(BUILD_BOCHS)
	set(SRCS
		CPU.cc
		CPUState.cc
		Listener.cc
		ListenerManager.cc
		SALConfig.cc
		Register.cc
		SimulatorController.cc
		bochs/BochsController.cc
		bochs/BochsListener.cc
		bochs/BochsCPU.cc
	)
elseif(BUILD_GEM5)
	set(SRCS
		CPU.cc
		CPUState.cc
		Listener.cc
		ListenerManager.cc
		SALConfig.cc
		Register.cc
		SimulatorController.cc
		gem5/Gem5Controller.cc
	)
	if(BUILD_ARM)
		set(SRCS ${SRCS}
			gem5/Gem5ArmCPU.cc
		)
	endif(BUILD_ARM)
elseif(BUILD_QEMU)
	set(SRCS
		CPU.cc
		CPUState.cc
		Listener.cc
		ListenerManager.cc
		SALConfig.cc
		Register.cc
		SimulatorController.cc
		qemu/QEMUController.cc
		qemu/wrappers.cc
	)
elseif(BUILD_T32)
	set(SRCS
		CPU.cc
		CPUState.cc
		Listener.cc
		ListenerManager.cc
		SALConfig.cc
		Register.cc
		SimulatorController.cc
		t32/T32Controller.cc
	)
	if(BUILD_ARM)
		set(SRCS ${SRCS}
			t32/T32ArmCPU.cc
			arm/ArmMemoryInstruction.cc
			arm/ArmDisassembler.cc
		)
	endif(BUILD_ARM)
elseif(BUILD_PANDA)
	include_directories(${PROJECT_SOURCE_DIR}/debuggers/openocd)
	set(SRCS
		CPU.cc
		CPUState.cc
		Listener.cc
		ListenerManager.cc
		SALConfig.cc
		Register.cc
		SimulatorController.cc
		panda/PandaController.cc
		panda/PandaArmCPU.cc
		panda/PandaListener.cc
	)
	if(BUILD_ARM)
		set(SRCS ${SRCS}
			panda/PandaArmCPU.cc
		)
	endif(BUILD_ARM)
endif(BUILD_BOCHS)

if(BUILD_X86)
	set(SRCS ${SRCS}
		x86/X86Architecture.cc
	)
	set(ARCH_TOOL_PREFIX "" CACHE PATH "Setup prefix for binutils, e.g., arm-none-eabi- or tricore-, ..")
elseif(BUILD_ARM)
	set(SRCS ${SRCS}
		arm/ArmArchitecture.cc
	)
	set(ARCH_TOOL_PREFIX "arm-none-eabi-" CACHE PATH "Setup prefix for binutils, e.g., arm-none-eabi- or tricore-, ..")
endif(BUILD_X86)

if(BUILD_GEM5)
	message(STATUS "[${PROJECT_NAME}] Generating SConscript in ${CMAKE_CURRENT_BINARY_DIR}/gem5")
	set(additional_libs "")

	foreach(exp ${EXPERIMENTS_ACTIVATED})
		set(additional_libs "${additional_libs} '-lfail-${exp}',")
	endforeach(exp)

	foreach(plug ${PLUGINS_ACTIVATED})
		set(additional_libs "${additional_libs} '-lfail-${plug}',")
	endforeach(plug)

	set(GEM5_SAL_SRCS "'${CMAKE_CURRENT_SOURCE_DIR}/gem5/Gem5Wrapper.cc'")
	configure_file(gem5/SConscript.in ${CMAKE_CURRENT_BINARY_DIR}/gem5/SConscript)
endif(BUILD_GEM5)
# Don't include these sources if perf-stuff is disabled
# (reduces compiler overhead):
if(CONFIG_FAST_WATCHPOINTS)
	set(SRCS ${SRCS}
		perf/WatchpointBuffer.cc
	)
endif(CONFIG_FAST_WATCHPOINTS)

if(CONFIG_FAST_BREAKPOINTS)
	set(SRCS ${SRCS}
		perf/BreakpointBuffer.cc
	)
endif(CONFIG_FAST_BREAKPOINTS)


add_library(fail-sal ${SRCS})
target_link_libraries(fail-sal fail-efw fail-util)

foreach(exp ${EXPERIMENTS_ACTIVATED})
	target_link_libraries(fail-sal fail-${exp})
endforeach()
