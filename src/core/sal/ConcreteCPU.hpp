#ifndef __CONCRETE_CPU_HPP__
#define __CONCRETE_CPU_HPP__

#if defined BUILD_BOCHS
	#include "bochs/BochsCPU.hpp"
#elif defined BUILD_GEM5
	#if defined BUILD_ARM
		#include "gem5/Gem5ArmCPU.hpp"
	#else
		#error Active config currently not supported!
	#endif
#elif defined BUILD_QEMU
	#if defined BUILD_X86
		#include "qemu/QEMUx86CPU.hpp"
	#else
		#error Active config currently not supported!
	#endif
#elif defined BUILD_T32
	#include "t32/T32Config.hpp"
	#if defined BUILD_ARM
		#include "t32/T32ArmCPU.hpp"
	#else
		#error Active config currently not supported!
	#endif
#elif defined BUILD_PANDA
	#include "panda/PandaConfig.hpp"
	#include "panda/PandaArmCPU.hpp"
#else
	#error SAL Config Target not defined
#endif

#endif // __CONCRETE_CPU_HPP__
