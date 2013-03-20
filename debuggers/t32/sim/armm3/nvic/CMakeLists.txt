# NVIC simulation model for the T32
set(SRCS
  nvic.c
  ../../include/simul.c # TODO we need a simul.c anywhere...
  nvic_id_space.c
  nvic_interrupt_type.c
  nvic_nvic.c
  nvic_software_trigger.c
  nvic_system_control_block.c
  nvic_system_control_block_cpuid.c
  nvic_system_timer.c
)

add_library(nvic SHARED ${SRCS})
GET_TARGET_PROPERTY(__T32_NVIC_LIB nvic LOCATION)
SET(T32_NVIC_LIB "${__T32_NVIC_LIB}" CACHE INTERNAL "")

