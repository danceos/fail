// Architecture.hpp: wraps architecture definition headers
#ifndef __ARCHITECTURE_HPP__
#define __ARCHITECTURE_HPP__

#include "config/FailConfig.hpp"

#ifdef BUILD_X86
#include "x86/X86Architecture.hpp"
#endif

#ifdef BUILD_ARM
#include "arm/ArmArchitecture.hpp"
#endif

#endif
