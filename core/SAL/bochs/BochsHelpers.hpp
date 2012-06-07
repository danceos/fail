#ifndef __BOCHS_HELPERS_HPP__
  #define __BOCHS_HELPERS_HPP__

#include "../../../bochs/cpu/cpu.h"

static inline BX_CPU_C *getCPU(BX_CPU_C *that)
{
#if BX_USE_CPU_SMF == 0
	return that;
#else
	return BX_CPU_THIS;
#endif
}

#endif // __BOCHS_HELPERS_HPP__
