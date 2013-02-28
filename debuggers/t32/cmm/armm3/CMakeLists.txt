### Configure cmm Scripts for ARM Cortex-M3

set(T32_CPUNAME STM32F103RG CACHE PATH "CPU name for SYSTEM.CPU call. (e.g. STM32F103RG)")
configure_file(armm3cfg.cmm.in ${PROJECT_BINARY_DIR}/cmm/armm3cfg.cmm)
configure_file(init.cmm ${PROJECT_BINARY_DIR}/cmm/init.cmm COPYONLY)
configure_file(loadelf.cmm ${PROJECT_BINARY_DIR}/cmm/loadelf.cmm COPYONLY)
configure_file(t32term.cmm ${PROJECT_BINARY_DIR}/cmm/t32term.cmm COPYONLY)


