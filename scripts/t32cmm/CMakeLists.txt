## Setup T32 target architecture for startup scripts

if(EXISTS $ENV{T32SYS})
  SET(T32_SYS $ENV{T32SYS})
  message(STATUS "[FAIL*] T32 base directory: T32SYS=${T32_SYS}")
else()
  message(FATAL_ERROR "Please set env variable T32SYS to a valid T32 installation base directory.")
endif()

if(EXISTS $ENV{FAIL_ELF_PATH})
  SET(T32_ELF_PATH $ENV{FAIL_ELF_PATH})
  message(STATUS "[FAIL*] T32 ELF under test: ${T32_ELF_PATH}")
else()
  message(FATAL_ERROR "Please set the FAIL_ELF_PATH enviroment variable to the binary under test.")
endif()

OPTION( T32_SIMULATOR "Start LAuterbach as instruction set simulator. No hardware needed." OFF)

if(T32_SIMULATOR)
 set(T32_USB_OR_SIM "SIM")
else()
 set(T32_USB_OR_SIM "USB")
endif()


file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/cmm)
configure_file(config.t32.in ${PROJECT_BINARY_DIR}/cmm/config.t32)
configure_file(t32.cmm.in ${PROJECT_BINARY_DIR}/cmm/t32.cmm)

set(T32_ARCHITECTURE armm3 CACHE PATH "Setup target architecture for default cmm scripts (currently only armm3)")
set(T32_EXE "${T32_SYS}/bin/pc_linux64/" CACHE INTERNAL "") # TODO: set pc_linux64 for 64 bit systems
add_subdirectory(${T32_ARCHITECTURE})


message(STATUS "[FAIL*] T32 Architecture: ${T32_ARCHITECTURE}")
message(STATUS "[FAIL*] T32 CPU name: ${T32_CPUNAME}")
message(STATUS "[FAIL*] T32 Executable: ${T32_EXE}")

add_custom_target(runt32
    COMMAND T32CONFIG=${PROJECT_BINARY_DIR}/cmm/config.t32 ${T32_EXE} &
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/cmm"
    COMMENT "Starting Lauterbach."
    )
