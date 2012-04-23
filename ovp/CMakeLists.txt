

 # PreDefined environment variables (setupImperas)
 # IMPERAS_HOME
 # IMPERAS_ARCA
 
 # And the variables set in Imperas/bin/Makefile.include
 # IMPERAS_LIB
 # IMPERAS_BIN
 # IMPERAS_VMISTUBS
 # IMPERAS_VMIINC
 # SHRSUF
 # 
#TODO test ist IMPERAS_HOME is set.


message(STATUS "Imperas Home: $ENV{IMPERAS_HOME}")
message(STATUS "Imperas License: $ENV{IMPERASD_LICENSE_FILE}")


message(STATUS "Arch: $ENV{IMPERAS_ARCH}")
#message(STATUS "VLNV: $ENV{IMPERAS_VLNV}")
#set(IMPERAS_LIB $ENV{IMPERAS_HOME}/lib/$ENV{IMPERAS_ARCH})

set(IMPERAS_BIN $ENV{IMPERAS_HOME}/bin/$ENV{IMPERAS_ARCH})
set(IMPERAS_VMISTUBS ${IMPERAS_BIN}/vmiStubs.a)
set(IMPERAS_VMIINC
	 -I$ENV{IMPERAS_HOME}/ImpPublic/include/host 
)

## This is needed when building a shared library
set(HOST_LDFLAGS "-Wl,--version-script=$ENV{IMPERAS_HOME}/ImperasLib/source/buildutils/version.script")

## This is needed by a platform executable
set(SIM_LDFLAGS $ENV{IMPERAS_HOME}/bin/$ENV{IMPERAS_ARCH}/libRuntimeLoader.so)


include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/core)
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_BINARY_DIR}/core)
include_directories($ENV{IMPERAS_HOME}/ImpPublic/include/host)

#add_subdirectory(ovpworld.org/mmc/flakyMemory/1.0/model)
add_subdirectory(cortexM3)

include(ExternalProject)
#### Put resulting (model) library file in <your_build_dir>/lib ####
ExternalProject_Add(
	OVParmmModel
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/armmModel
	INSTALL_COMMAND ""
	CMAKE_ARGS -DFAIL_BASE=${CMAKE_SOURCE_DIR} -DLIBRARY_OUTPUT_PATH=${LIBRARY_OUTPUT_PATH}
)

