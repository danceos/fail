add_subdirectory(faultspace)

# Some files are common over all architectures
set(SRCS
	CPU.cc
	CPUState.cc
	Listener.cc
	ListenerManager.cc
	SALConfig.cc
	Register.cc
	SimulatorController.cc
	)

if(BUILD_BOCHS)
	set(SRCS ${SRCS}
		bochs/BochsController.cc
		bochs/BochsListener.cc
		bochs/BochsCPU.cc
	)
elseif(BUILD_GEM5)
	set(SRCS ${SRCS}
		gem5/Gem5Controller.cc
	)
	if(BUILD_ARM)
		set(SRCS ${SRCS}
			gem5/Gem5ArmCPU.cc
		)
	endif(BUILD_ARM)
elseif(BUILD_QEMU)
	set(SRCS ${SRCS}
		qemu/QEMUController.cc
		qemu/wrappers.cc
	)
elseif(BUILD_T32)
	set(SRCS ${SRCS}
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
	set(SRCS ${SRCS}
		panda/PandaController.cc
		panda/PandaArmCPU.cc
		panda/PandaListener.cc
	)
	if(BUILD_ARM)
		set(SRCS ${SRCS}
			panda/PandaArmCPU.cc
		)
	endif(BUILD_ARM)
elseif(BUILD_SAIL)
	add_subdirectory(sail)
	#set(SRCS ${SRCS}
	#	sail/memory.cc
	#	sail/tag_memory.cc
	#	sail/wrapper.cc
	#   sail/simulator.cc
	#)
	#if(BUILD_RISCV)
 	#set(SRCS ${SRCS}
	#		sail/riscv/arch.cpp)
	#elseif(BUILD_RISCV_CHERI)
#		set(SRCS ${SRCS}
#			sail/cheri-riscv/arch.cpp)
#	else()
#		message(ERROR " No SAIL_ARCH set. Aborting!")
#	endif(BUILD_RISCV)
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

# FIXME: Find dynamic
if(BUILD_SAIL)
	target_link_libraries(fail-sal fail-sail)
	if (BUILD_64BIT)
		target_link_libraries(fail-sal -lgmpxx -lgmp)
	endif()
endif()

target_link_libraries(fail-sal fail-efw fail-fsp fail-util)



foreach(exp ${EXPERIMENTS_ACTIVATED})
	target_link_libraries(fail-sal fail-${exp})
endforeach()
