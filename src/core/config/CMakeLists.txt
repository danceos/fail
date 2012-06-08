#### Configuration file emitting BUILD_OVP/BOCHS defines ####
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/VariantConfig.hpp.in
               ${CMAKE_CURRENT_BINARY_DIR}/VariantConfig.hpp)

OPTION(CONFIG_EVENT_BREAKPOINTS         "Event source: Breakpoints" OFF)
OPTION(CONFIG_EVENT_MEMREAD             "Event source: Memory reads" OFF)
OPTION(CONFIG_EVENT_MEMWRITE            "Event source: Memory writes" OFF)
OPTION(CONFIG_EVENT_GUESTSYS            "Event source: Outbound guest-system communication" OFF)
OPTION(CONFIG_EVENT_IOPORT              "Event source: I/O port communication" OFF)
OPTION(CONFIG_EVENT_INTERRUPT           "Event source: Interrupts" OFF)
OPTION(CONFIG_EVENT_TRAP                "Event source: Traps" OFF)
OPTION(CONFIG_EVENT_JUMP                "Event source: Branch instructions" OFF)
OPTION(CONFIG_SR_RESTORE                "Target backend: State restore" OFF)
OPTION(CONFIG_SR_SAVE                   "Target backend: State saving" OFF)
OPTION(CONFIG_SR_REBOOT                 "Target backend: Reboot" OFF)
OPTION(CONFIG_BOCHS_NON_VERBOSE         "Misc: Reduced verbosity" OFF)
OPTION(CONFIG_SUPPRESS_INTERRUPTS       "Target backend: Suppress interrupts" OFF)
OPTION(CONFIG_FIRE_INTERRUPTS           "Target backend: Fire interrupts" OFF)
OPTION(CONFIG_DISABLE_KEYB_INTERRUPTS   "Target backend: Suppress keyboard interrupts" OFF)
OPTION(SERVER_PERFORMANCE_MEASURE       "Performance measurement in job-server" OFF)
SET(SERVER_PERF_LOG_PATH        "perf.log" CACHE STRING "A file name for storing the server's performance log (CSV)")
SET(SERVER_PERF_STEPPING_SEC    "1"        CACHE STRING "Stepping of performance measurements in seconds")
SET(CLIENT_RAND_BACKOFF_TSTART  "3"        CACHE STRING "Lower limit of client's backoff phase in seconds")
SET(CLIENT_RAND_BACKOFF_TEND    "8"        CACHE STRING "Upper limit of client's backoff phase in seconds")
SET(CLIENT_RETRY_COUNT          "3"        CACHE STRING "Client's number of reconnect retries")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/FailConfig.hpp.in
               ${CMAKE_CURRENT_BINARY_DIR}/FailConfig.hpp)
