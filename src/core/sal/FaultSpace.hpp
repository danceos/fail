#pragma once

#include "config/FailConfig.hpp"

#ifdef BUILD_SAIL

#include "sail/fault_space.hpp"
namespace fail {
    typedef sail_fault_space FaultSpace;
}
#else

#error Unsupported Architecture

#endif

