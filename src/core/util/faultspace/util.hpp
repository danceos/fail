#pragma once

#include <stdint.h>
#include <functional>

namespace fail {
    namespace util {
        namespace fsp {
            typedef uint64_t address_t;
            // each fault space element is at most
            // 8 bit in length, thus its value and/or
            // injected may also only be at most 8 bit
            // long.
            typedef uint8_t injector_value;
            struct injector_result {
                uint8_t original;
                uint8_t injected;
            };
            typedef std::function<injector_value(injector_value)> injector_fn;
        }
    }
}
