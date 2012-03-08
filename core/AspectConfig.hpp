#ifndef __ASPECT_CONFIG_HPP__
  #define __ASPECT_CONFIG_HPP__

// The following configuration macros to disable (0) / enable (1) the various
// event sources, fault injection sinks, and miscellaneous other features.

// Event sources
#define CONFIG_EVENT_CPULOOP    0
#define CONFIG_EVENT_MEMREAD    0
#define CONFIG_EVENT_MEMWRITE   0
#define CONFIG_EVENT_GUESTSYS   0
#define CONFIG_EVENT_INTERRUPT  0
#define CONFIG_EVENT_TRAP       0
#define CONFIG_EVENT_JUMP       0

// Save/restore functionality
#define CONFIG_SR_RESTORE       0
#define CONFIG_SR_SAVE          0
#define CONFIG_SR_REBOOT        0

// Miscellaneous
#define CONFIG_STFU                      0
#define CONFIG_SUPPRESS_INTERRUPTS       0
#define CONFIG_DISABLE_KEYB_INTERRUPTS   0

// Fault injection
#define CONFIG_FI_MEM_ACCESS_BITFLIP     0


#endif /* __ASPECT_CONFIG_HPP__ */
