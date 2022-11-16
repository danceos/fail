#pragma once

#include <stdint.h>

namespace fail {
typedef uint64_t guest_address_t;  //!< the guest memory address type
typedef uint64_t host_address_t;   //!< the host memory address type
typedef unsigned int timer_t;  //!< type of timer IDs

/**
 * For CHERI enabled RISC-V, all GP register may hold
 * up to length(capability) bits, which is 2*XLEN for
 * CHERI concentrate representation.
 * NOTE: This uses the GCC builtin 128 bit unsigned integer type.
 *       And thus is not compatible with LLVM.
 */
#if defined(BUILD_64BIT)
typedef unsigned __int128 register_data_t;
#define SAIL_XLEN 64
#else
typedef uint64_t register_data_t;
#define SAIL_XLEN 32
#endif

#define SAIL_RESET_VECTOR 0x8000000

}
