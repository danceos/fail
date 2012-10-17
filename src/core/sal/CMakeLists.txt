# common:
set(SRCS
	Listener.cc
	ListenerManager.cc
	SALConfig.cc
	Register.cc
	SimulatorController.cc
	perf/BreakpointBuffer.cc
)
if(CONFIG_FAST_BREAKPOINTS)
	add_aspect_headers(
		${CMAKE_CURRENT_SOURCE_DIR}/perf/BreakpointControllerSlice.ah
		${CMAKE_CURRENT_SOURCE_DIR}/perf/BreakpointManagerSlice.ah
		${CMAKE_CURRENT_SOURCE_DIR}/perf/FastBreakpoints.ah)
endif()

if(BUILD_BOCHS)
	list(APPEND SRCS
		bochs/BochsController.cc
		bochs/BochsListener.cc
	)
	add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/bochs/BochsListener.ah)
	if(CONFIG_BOCHS_NO_ABORT)
		add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/bochs/BochsNoAbort.ah)
	endif()
	if(CONFIG_BOCHS_NON_VERBOSE)
		add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/bochs/BochsNonVerbose.ah)
	endif()
	if(CONFIG_EVENT_BREAKPOINTS)
		add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/bochs/Breakpoints.ah)
	endif()
	add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/bochs/Credits.ah)
	if(CONFIG_DISABLE_KEYB_INTERRUPTS)
		add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/bochs/DisableKeyboardInterrupt.ah)
	endif()
	add_aspect_headers(
		${CMAKE_CURRENT_SOURCE_DIR}/bochs/DisableLogFunctions.ah
		${CMAKE_CURRENT_SOURCE_DIR}/bochs/FailBochsInit.ah)
	if(CONFIG_FIRE_INTERRUPTS)
		add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/bochs/FireInterrupt.ah)
	endif()
	if(CONFIG_EVENT_GUESTSYS)
		add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/bochs/GuestSysCom.ah)
	endif()
	if(CONFIG_EVENT_INTERRUPT)
		add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/bochs/Interrupt.ah)
	endif()
	if(CONFIG_SUPPRESS_INTERRUPTS)
		add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/bochs/InterruptSuppression.ah)
	endif()
	if(CONFIG_EVENT_IOPORT)
		add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/bochs/IOPortCom.ah)
	endif()
	if(CONFIG_EVENT_JUMP)
		add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/bochs/Jump.ah)
	endif()
	if(CONFIG_EVENT_MEMREAD OR CONFIG_EVENT_MEMWRITE)
		add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/bochs/MemAccess.ah)
	endif()
	if(CONFIG_SR_REBOOT)
		add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/bochs/Reboot.ah)
	endif()
	if(CONFIG_SR_RESTORE)
		add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/bochs/RestoreState.ah)
	endif()
	if(CONFIG_SR_SAVE)
		add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/bochs/SaveState.ah)
	endif()
	if(CONFIG_EVENT_TRAP)
		add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/bochs/Trap.ah)
	endif()
elseif(BUILD_GEM5)
	list(APPEND SRCS
		gem5/Gem5Controller.cc
		gem5/Gem5PCEvents.cc
	)
	# note: contrary to the original preprocessor conditional we do not
	# depend on CONFIG_EVENT_BREAKPOINTS here (not necessary)
	add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/gem5/Gem5Listener.ah)
elseif(BUILD_OVP)
	list(APPEND SRCS
		${VARIANT}/OVPController.cc
	)
	add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/ovp/FailOVPInit.ah)
elseif(BUILD_QEMU)
	list(APPEND SRCS
		qemu/QEMUController.cc
		qemu/wrappers.cc
	)
	# note: contrary to the original preprocessor conditional we do not
	# depend on CONFIG_EVENT_BREAKPOINTS here (not necessary)
	add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/qemu/QEMUListener.ah)
elseif(BUILD_T32)
	list(APPEND SRCS
		t32/T32Controller.cc
		t32/wrappers.cc
	)
	# note: contrary to the original preprocessor conditional we do not
	# depend on CONFIG_EVENT_BREAKPOINTS here (not necessary)
	add_aspect_headers(${CMAKE_CURRENT_SOURCE_DIR}/t32/T32Listener.ah)
endif()

add_library(fail-sal ${SRCS})

add_dependencies(fail-sal fail-efw)
