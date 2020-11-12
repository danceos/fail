#ifndef __SAL_CONFIG_HPP__
#define __SAL_CONFIG_HPP__

#include <stdint.h>
#include <iostream>

#include "config/VariantConfig.hpp"

// this needs to be defined before included arch specific configs as memory_type enums use this as the base type.
typedef uint64_t memory_type_t;
// Type-config depends on the current selected simulator:
#if defined BUILD_BOCHS
	#include "bochs/BochsConfig.hpp"
#elif defined BUILD_GEM5
	#include "gem5/Gem5Config.hpp"
#elif defined BUILD_QEMU
	#include "qemu/QEMUConfig.hpp"
#elif defined BUILD_T32
	#include "t32/T32Config.hpp"
#elif defined BUILD_PANDA
	#include "panda/PandaConfig.hpp"
#elif defined BUILD_SAIL
	#include "sail/config.hpp"
#else
	#error SAL Config Target not defined
#endif

namespace fail {

typedef guest_address_t  address_t;   //!< common address type to be used in experiment flows
typedef uint32_t         regwidth_t;  //!< type of register width [bits]
typedef register_data_t  regdata_t;   //!< type of register data
typedef timer_t          timer_id_t;  //!< type of timer IDs
//! backend-specific notion of time, e.g. CPU cycles or nanoseconds
//! (move this to backend-specific headers when necessary)
typedef uint64_t         simtime_t;
//! backend-specific notion of time difference
typedef int64_t          simtime_diff_t;

// define additional ostream operators for 128 bit integers
#if defined(BUILD_SAIL) && defined(BUILD_64BIT)
std::ostream& operator<<(std::ostream& os, unsigned __int128 v);
std::istream& operator>>(std::istream& is, unsigned __int128& v);
#endif

// Note: The following flags are defined in SALConfig.cc.

//! invalid address flag (e.g. for memory address ptrs)
extern const address_t   ADDR_INV;
//! address wildcard (e.g. for breakpoint listeners)
extern const address_t   ANY_ADDR;
//! memory type wildcard (e.g. for memory access listener)
extern const memory_type_t ANY_MEMORY;
//! instruction wildcard (e.g. for jump listeners)
extern const unsigned    ANY_INSTR;
//! trap wildcard
extern const unsigned    ANY_TRAP;
//! interrupt wildcard
extern const unsigned    ANY_INTERRUPT;
//! invalid timer id (e.g. for timer listeners)
extern const timer_id_t  INVALID_TIMER;

} // end-of-namespace: fail

#endif // __SAL_CONFIG_HPP__
