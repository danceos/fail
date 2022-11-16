#pragma once

#include <stdint.h>

namespace fail {
typedef uint64_t guest_address_t;  //!< the guest memory address type
typedef uint64_t host_address_t;   //!< the host memory address type
typedef unsigned int timer_t;  //!< type of timer IDs

/**
 * For RISC-V, all GP register may hold up to XLEN bits, e.g. 64 or 32
 * bit for RV32 and RV64 respectively. Larger XLEN are currently not
 * supported by the sail model.
 */
#if defined(BUILD_64BIT)
typedef uint64_t register_data_t;
#define SAIL_XLEN 64
#else
typedef uint32_t register_data_t;
#define SAIL_XLEN 32
#endif

#define SAIL_RESET_VECTOR 0x8000000

}
