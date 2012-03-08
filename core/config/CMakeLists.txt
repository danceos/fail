#### Configuration file emitting BUILD_OVP/BOCHS defines ####
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/variant_config.hpp.in
               ${CMAKE_CURRENT_BINARY_DIR}/variant_config.hpp)

OPTION(CONFIG_EVENT_CPULOOP             "Event source: Breakpoints" OFF)
OPTION(CONFIG_EVENT_MEMREAD             "Event source: Memory reads" OFF)
OPTION(CONFIG_EVENT_MEMWRITE            "Event source: Memory writes" OFF)
OPTION(CONFIG_EVENT_GUESTSYS            "Event source: Outbound guest-system communication" OFF)
OPTION(CONFIG_EVENT_INTERRUPT           "Event source: Interrupts" OFF)
OPTION(CONFIG_EVENT_TRAP                "Event source: Traps" OFF)
OPTION(CONFIG_EVENT_JUMP                "Event source: Branch instructions" OFF)
OPTION(CONFIG_SR_RESTORE                "Target backend: State restore" OFF)
OPTION(CONFIG_SR_SAVE                   "Target backend: State saving" OFF)
OPTION(CONFIG_SR_REBOOT                 "Target backend: Reboot" OFF)
OPTION(CONFIG_STFU                      "Misc: Reduced verbosity" OFF)
OPTION(CONFIG_SUPPRESS_INTERRUPTS       "Target backend: Suppress interrupts" OFF)
OPTION(CONFIG_DISABLE_KEYB_INTERRUPTS   "Target backend: Suppress keyboard interrupts" OFF)
OPTION(CONFIG_FI_MEM_ACCESS_BITFLIP     "deprecated something" OFF)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/AspectConfig.hpp.in
               ${CMAKE_CURRENT_BINARY_DIR}/AspectConfig.hpp)
