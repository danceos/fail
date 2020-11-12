#ifndef SAILCONFIG_HPP
#define SAILCONFIG_HPP

#include <stdint.h>

namespace fail {

typedef uint64_t guest_address_t;  //!< the guest memory address type
typedef uint64_t host_address_t;   //!< the host memory address type
typedef unsigned int timer_t;  //!< type of timer IDs
typedef uint8_t byte_t;        //!< type of memory bytes.

/**
 * SAIL models support two memory types, tag memory and 'normal'
 * memory, which can be injected seperately.
 * This a limitation imposed by the sail runtime, though model developers
 * could opt to implement their own physical memory implementation.
 * If such a model is ported in the future, it might be good to
 * move this enumeration into arch specific config.hpp's
 */
namespace memory_type {
    enum types: memory_type_t {
        tag = 1, ram = 0, any = 2
    };
}
}  // namespace fail


#if defined(BUILD_RISCV)
#include "riscv/config.hpp"
#elif defined(BUILD_RISCV_CHERI)
#include "cheri-riscv/config.hpp"
#else
#error SAIL architecture does not have a config.hpp defined.
#endif


#endif /* SAILCONFIG_HPP */
