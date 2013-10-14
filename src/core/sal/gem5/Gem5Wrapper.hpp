#ifndef __GEM5_WRAPPER_HPP__
#define __GEM5_WRAPPER_HPP__

#include "../Register.hpp"
#include "../SALConfig.hpp"

// gem5 forward declarations:
class System;

namespace fail {

// Register-/Memory-related:
regdata_t GetRegisterContent(System* sys, unsigned int id, size_t idx);
void SetRegisterContent(System* sys, unsigned int id, size_t idx, regdata_t value);
void WriteMemory(System* sys, guest_address_t addr, size_t cnt, void const *src);
void ReadMemory(System* sys, guest_address_t addr, size_t cnt, void *dest);
size_t GetPoolSize(System* sys);

// Controller-related:
unsigned int GetCPUId(System* sys, int context);
System* GetSystemObject();
int GetNumberOfContexts(System* sys);

} // end-of-namespace: fail

#endif // __GEM5_WRAPPER_HPP__
