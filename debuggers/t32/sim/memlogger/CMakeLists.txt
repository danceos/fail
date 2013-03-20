# NVIC simulation model for the T32
set(SRCS
  memlogger.c
  ../include/simul.c
)

add_library(memlogger SHARED ${SRCS})
GET_TARGET_PROPERTY(__T32_MEMLOGGER_LIB memlogger LOCATION)
SET(T32_MEMLOGGER_LIB "${__T32_MEMLOGGER_LIB}" CACHE INTERNAL "")

