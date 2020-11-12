#pragma once

#include <stdint.h>
#include "enums.hpp"

namespace fail {
    /**
     * For RISC-V, all GP register may hold
     * up to XLEN bits, e.g. 64 or 32 bit for RV32 and RV64 respectively.
     * Larger XLEN are currently not supported by the sail model.
     */
#if defined(BUILD_64BIT)
    typedef uint64_t register_data_t;
#else
    typedef uint32_t register_data_t;
#endif
}
