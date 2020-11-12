#ifndef SAILARCHITECTURECONFIG_HPP
#define SAILARCHITECTURECONFIG_HPP

#include "../SALConfig.hpp"

#if defined(BUILD_RISCV)

// an arch_cpu.hpp must define a ConcreteCPU through the SAIL_DECLARE_CPU macro.
#include "riscv/arch_cpu.hpp"

#elif defined(BUILD_RISCV_CHERI)

#include "cheri-riscv/arch_cpu.hpp"

#else
    #error SAIL Architecture not defined
#endif

#endif /* SAILARCHITECTURECONFIG_HPP */
