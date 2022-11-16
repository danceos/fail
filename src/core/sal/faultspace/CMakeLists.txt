add_library(fail-fsp
	BaseFaultSpace.cc
	MemoryArea.cc
	RegisterArea.cc
	../x86/X86FaultSpace.cc
	../arm/ArmFaultSpace.cc
)

if (BUILD_SAIL)
	target_link_libraries(fail-fsp fail-sail)
endif(BUILD_SAIL)
