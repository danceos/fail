#pragma once

#include <stdint.h>
#include "enums.hpp"

namespace fail {
    /**
     * For CHERI enabled RISC-V, all GP register may hold
     * up to length(capability) bits, which is 2*XLEN for
     * CHERI concentrate representation.
     * NOTE: This uses the GCC builtin 128 bit unsigned integer type.
     *       And thus is not compatible with LLVM.
     */
#if defined(BUILD_64BIT)
    typedef unsigned __int128 register_data_t;
#else
    typedef uint64_t register_data_t;
#endif
}
