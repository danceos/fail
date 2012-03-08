Project(OVPCM3_Model)
cmake_minimum_required(VERSION 2.8)


message(STATUS "Imperas Home: $ENV{IMPERAS_HOME}")
message(STATUS "Imperas License: $ENV{IMPERASD_LICENSE_FILE}")


message(STATUS "Arch: $ENV{IMPERAS_ARCH}")
set(IMPERAS_BIN $ENV{IMPERAS_HOME}/bin/$ENV{IMPERAS_ARCH})
set(IMPERAS_VMISTUBS ${IMPERAS_BIN}/vmiStubs.a)
set(IMPERAS_VMIINC
	 -I$ENV{IMPERAS_HOME}/ImpPublic/include/host 
)

## This is needed when building a shared library
set(HOST_LDFLAGS "-Wl,--version-script=$ENV{IMPERAS_HOME}/ImperasLib/source/buildutils/version.script")

## This is needed by a platform executable
set(SIM_LDFLAGS $ENV{IMPERAS_HOME}/bin/$ENV{IMPERAS_ARCH}/libRuntimeLoader.so)



message(STATUS "Fail* base dir included: ${FAIL_BASE}")
include_directories(${FAIL_BASE})
include_directories($ENV{IMPERAS_HOME}/ImpPublic/include/host)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

#### Put resulting (model) library file in <your_build_dir>/lib ####

set(SRCS
	failSALlink.cc
	armAttributeEntriesThumb16.h
	armAttributeEntriesThumb32.h
	armBitMacros.h
	armBus.c
	armConfig.h
	armDebug.h
	armDecodeEntriesThumb16.h
	armDecodeEntriesThumb32.h
	armDecode.h
	armDecodeThumb.h
	armDecodeTypes.h
	armDisassembleFormats.h
	armDisassemble.h
	armEmit.h
	armExceptions.h
	armExceptionTypes.h
	armFPConstants.h
	armFunctions.h
	armInfo.c
	armmAttrs.c
	armmConfigList.c
	armmDebug.c
	armmDecode.c
	armmDecodeThumb.c
	armmDisassemble.c
	armmDoc.c
	armmDoc.h
	armmEmit.c
	armMessage.h
	armmExceptions.c
	armmMain.c
	armmMorphFunctions.c
	armmMorphTable.c
	armMode.h
	armMorphEntries.h
	armMorphFunctions.h
	armMorph.h
	armmParameters.c
	armmParameters.h
	armmSemiHost.c
	armmSys.c
	armmUtils.c
	armmVFP.c
	armmVM.c
	armRegisters.h
	armStructure.h
	armSys.h
	armSysRegisters.h
	armTypeRefs.h
	armUtils.h
	armVariant.h
	armVFP.h
	armVM.h
)
add_definitions("-m32")
message(STATUS "armmModel ld flags: ${HOST_LDFLAGS}")
add_library(armmModel SHARED ${SRCS})

#add_dependencies(armmModel SAL)
set_target_properties(armmModel PROPERTIES LINK_FLAGS "${HOST_LDFLAGS} -m32")

#target_link_libraries(armmModel ${IMPERAS_VMISTUBS}  ${LIBRARY_OUTPUT_PATH}/libSAL.a)
target_link_libraries(armmModel ${IMPERAS_VMISTUBS})

