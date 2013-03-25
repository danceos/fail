#include "BochsCPU.hpp"
#include "../SALConfig.hpp"

#include <cassert>

namespace fail {

regdata_t BochsCPU::getRegisterContent(Register* reg) const
{
	assert(reg != NULL && "FATAL ERROR: reg-ptr cannot be NULL!");
	// TODO: BX_CPU(0) *always* correct?

	if (reg->getId() == RID_FLAGS) // EFLAGS register?
		return static_cast<regdata_t>(BX_CPU(id)->eflags);

  #ifdef SIM_SUPPORT_64
	if (reg->getId() == RID_PC) // program counter?
		return static_cast<regdata_t>(BX_CPU(id)->gen_reg[BX_64BIT_REG_RIP].rrx);
	else // 64 bit general purpose registers
		return static_cast<regdata_t>(BX_CPU(id)->gen_reg[reg->getId()].rrx);
  #else // 32 bit mode
	if (reg->getId() == RID_PC)
		return static_cast<regdata_t>(BX_CPU(id)->gen_reg[BX_32BIT_REG_EIP].dword.erx);
	else // 32 bit general purpose registers
		return static_cast<regdata_t>(BX_CPU(id)->gen_reg[reg->getId()].dword.erx);
  #endif // SIM_SUPPORT_64
}

void BochsCPU::setRegisterContent(Register* reg, regdata_t value)
{
	assert(reg != NULL && "FATAL ERROR: reg-ptr cannot be NULL!");
	// TODO: BX_CPU(0) *always* correct?

	if (reg->getId() == RID_FLAGS) { // EFLAGS register?
		regdata_t* pData = reinterpret_cast<regdata_t*>(&(BX_CPU(id)->eflags));
	  #ifdef SIM_SUPPORT_64
		// We are in 64 bit mode: Just assign the lower 32 bits!
		*pData = ((*pData) & 0xFFFFFFFF00000000ULL) | (value & 0xFFFFFFFFULL);
	  #else
		*pData = value;
	  #endif
		return;
	}

	regdata_t* pData;
  #ifdef SIM_SUPPORT_64
	if (reg->getId() == RID_PC) // program counter?
		pData = &(BX_CPU(id)->gen_reg[BX_64BIT_REG_RIP].rrx);
	else // 64 bit general purpose registers
		pData = &(BX_CPU(id)->gen_reg[reg->getId()].rrx);
  #else // 32 bit mode
	if (reg->getId() == RID_PC)
		pData = &(BX_CPU(id)->gen_reg[BX_32BIT_REG_EIP].dword.erx);
	else // 32 bit general purpose registers
		pData = &(BX_CPU(id)->gen_reg[reg->getId()].dword.erx);
  #endif // SIM_SUPPORT_64
	*pData = value;
}

} // end-of-namespace: fail
