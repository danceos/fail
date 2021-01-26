#pragma once

#include "config/FailConfig.hpp"
#include "config/VariantConfig.hpp"


#if defined(BUILD_X86)
#include "sal/x86/X86FaultSpace.hpp"
namespace fail {
typedef X86FaultSpace FaultSpace;
}
#elif defined(BUILD_ARM)
#include "sal/arm/ArmFaultSpace.hpp"
namespace fail {
typedef ArmFaultSpace FaultSpace;
}
#else
#error Architecture does not yet support the Virtual Fault Space abstraction
#endif

