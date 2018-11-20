#include "BochsCPU.hpp"
#include "../SALConfig.hpp"

#include <cassert>

namespace fail {

regdata_t BochsCPU::getRegisterContent(const Register* reg) const
{
	assert(reg != NULL && "FATAL ERROR: reg-ptr cannot be NULL!");

	switch (reg->getId()) {
	case RID_FLAGS: // EFLAGS/RFLAGS
		return static_cast<regdata_t>(BX_CPU(m_Id)->read_eflags());
	case RID_CR0:
		// untested
		return static_cast<regdata_t>(BX_CPU(m_Id)->read_CR0());
	case RID_CR2:
		// untested
		return static_cast<regdata_t>(BX_CPU(m_Id)->cr2);
	case RID_CR3:
		return static_cast<regdata_t>(BX_CPU(m_Id)->cr3);
	case RID_CR4:
		// untested
		return static_cast<regdata_t>(BX_CPU(m_Id)->read_CR4());
	case RID_CS:
		// untested
		return static_cast<regdata_t>(BX_CPU(m_Id)->sregs[BX_SEG_REG_CS].selector.value);
	case RID_DS:
		// untested
		return static_cast<regdata_t>(BX_CPU(m_Id)->sregs[BX_SEG_REG_DS].selector.value);
	case RID_ES:
		// untested
		return static_cast<regdata_t>(BX_CPU(m_Id)->sregs[BX_SEG_REG_ES].selector.value);
	case RID_FS:
		// untested
		return static_cast<regdata_t>(BX_CPU(m_Id)->sregs[BX_SEG_REG_FS].selector.value);
	case RID_GS:
		// untested
		return static_cast<regdata_t>(BX_CPU(m_Id)->sregs[BX_SEG_REG_GS].selector.value);
	case RID_SS:
		// untested
		return static_cast<regdata_t>(BX_CPU(m_Id)->sregs[BX_SEG_REG_SS].selector.value);
#ifdef SIM_SUPPORT_64
	case RID_PC: // program counter
		return static_cast<regdata_t>(BX_CPU(m_Id)->gen_reg[BX_64BIT_REG_RIP].rrx);
	default: // 64 bit general purpose registers
		return static_cast<regdata_t>(BX_CPU(m_Id)->gen_reg[reg->getId()].rrx);
#else // 32 bit mode
	case RID_PC: // program counter
		return static_cast<regdata_t>(BX_CPU(m_Id)->gen_reg[BX_32BIT_REG_EIP].dword.erx);
	default: // 32 bit general purpose registers
		return static_cast<regdata_t>(BX_CPU(m_Id)->gen_reg[reg->getId()].dword.erx);
#endif // SIM_SUPPORT_64
	}
}

void BochsCPU::setRegisterContent(const Register* reg, regdata_t value)
{
	assert(reg != NULL && "FATAL ERROR: reg-ptr cannot be NULL!");

	regdata_t* pData;
	switch (reg->getId()) {
	case RID_FLAGS: // EFLAGS/RFLAGS
	  #ifdef SIM_SUPPORT_64
		{
		// We are in 64 bit mode: Just assign the lower 32 bits!
		regdata_t regdata = getRegisterContent(reg);
		BX_CPU(m_Id)->writeEFlags((regdata & 0xFFFFFFFF00000000ULL) | (value & 0xFFFFFFFFULL),
								0xffffffff);
		}
	  #else
		BX_CPU(m_Id)->writeEFlags(value, 0xffffffff);
	  #endif
		BX_CPU(m_Id)->force_flags();
		return;
	#ifndef __puma
	case RID_CR0:
		// untested
		BX_CPU(m_Id)->SetCR0(value);
		return;
	case RID_CR2:
		// untested
		BX_CPU(m_Id)->cr2 = value;
		return;
	case RID_CR3:
		BX_CPU(m_Id)->SetCR3(value);
		return;
	case RID_CR4:
		// untested
		BX_CPU(m_Id)->SetCR4(value);
		return;
	case RID_CS:
		// untested
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_CS], value);
		return;
	case RID_DS:
		// untested
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_DS], value);
		return;
	case RID_ES:
		// untested
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_ES], value);
		return;
	case RID_FS:
		// untested
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_FS], value);
		return;
	case RID_GS:
		// untested
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_GS], value);
		return;
	case RID_SS:
		// untested
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_SS], value);
		return;
	#endif

#ifdef SIM_SUPPORT_64
	case RID_PC: // program counter
		pData = &(BX_CPU(m_Id)->gen_reg[BX_64BIT_REG_RIP].rrx);
		break;
	default: // 64 bit general purpose registers
		pData = &(BX_CPU(m_Id)->gen_reg[reg->getId()].rrx);
		break;
#else // 32 bit mode
	case RID_PC: // program counter
		pData = &(BX_CPU(m_Id)->gen_reg[BX_32BIT_REG_EIP].dword.erx);
		break;
	default: // 32 bit general purpose registers
		pData = &(BX_CPU(m_Id)->gen_reg[reg->getId()].dword.erx);
		break;
#endif // SIM_SUPPORT_64
	}
	*pData = value;
}

} // end-of-namespace: fail
