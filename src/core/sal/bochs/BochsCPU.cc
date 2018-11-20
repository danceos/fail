#include "BochsCPU.hpp"
#include "../SALConfig.hpp"

#include <cassert>

namespace fail {

regdata_t BochsCPU::getRegisterContent(const Register* reg) const
{
	assert(reg != NULL && "FATAL ERROR: reg-ptr cannot be NULL!");
	// TODO: BX_CPU(0) *always* correct?

	if (reg->getId() == RID_FLAGS) { // EFLAGS register?
		return static_cast<regdata_t>(BX_CPU(m_Id)->read_eflags());
	}

	// untested
	if (reg->getId() == RID_CR0) { // CR0 register?
		return static_cast<regdata_t>(BX_CPU(m_Id)->read_CR0());
	}

	// untested
	if (reg->getId() == RID_CR2) { // CR2 register?
		return static_cast<regdata_t>(BX_CPU(m_Id)->cr2);
	}

	if (reg->getId() == RID_CR3) { // CR3 register?
		return static_cast<regdata_t>(BX_CPU(m_Id)->cr3);
	}

	// untested
	if (reg->getId() == RID_CR4) { // CR4 register?
		return static_cast<regdata_t>(BX_CPU(m_Id)->read_CR4());
	}

	// untested
	if (reg->getId() == RID_CS) { // CS register?
		return static_cast<regdata_t>(BX_CPU(m_Id)->sregs[BX_SEG_REG_CS].selector.value);
	}

	// untested
	if (reg->getId() == RID_DS) { // DS register?
		return static_cast<regdata_t>(BX_CPU(m_Id)->sregs[BX_SEG_REG_DS].selector.value);
	}

	// untested
	if (reg->getId() == RID_ES) { // ES register?
		return static_cast<regdata_t>(BX_CPU(m_Id)->sregs[BX_SEG_REG_ES].selector.value);
	}

	// untested
	if (reg->getId() == RID_FS) { // FS register?
		return static_cast<regdata_t>(BX_CPU(m_Id)->sregs[BX_SEG_REG_FS].selector.value);
	}

	// untested
	if (reg->getId() == RID_GS) { // GS register?
		return static_cast<regdata_t>(BX_CPU(m_Id)->sregs[BX_SEG_REG_GS].selector.value);
	}

	// untested
	if (reg->getId() == RID_SS) { // SS register?
		return static_cast<regdata_t>(BX_CPU(m_Id)->sregs[BX_SEG_REG_SS].selector.value);
	}

#ifdef SIM_SUPPORT_64
	if (reg->getId() == RID_PC) // program counter?
		return static_cast<regdata_t>(BX_CPU(m_Id)->gen_reg[BX_64BIT_REG_RIP].rrx);
	else // 64 bit general purpose registers
		return static_cast<regdata_t>(BX_CPU(m_Id)->gen_reg[reg->getId()].rrx);
#else // 32 bit mode
	if (reg->getId() == RID_PC)
		return static_cast<regdata_t>(BX_CPU(m_Id)->gen_reg[BX_32BIT_REG_EIP].dword.erx);
	else // 32 bit general purpose registers
		return static_cast<regdata_t>(BX_CPU(m_Id)->gen_reg[reg->getId()].dword.erx);
#endif // SIM_SUPPORT_64
}

void BochsCPU::setRegisterContent(const Register* reg, regdata_t value)
{
	assert(reg != NULL && "FATAL ERROR: reg-ptr cannot be NULL!");
	// TODO: BX_CPU(0) *always* correct?

	if (reg->getId() == RID_FLAGS) { // EFLAGS register?
	  #ifdef SIM_SUPPORT_64
		// We are in 64 bit mode: Just assign the lower 32 bits!
		regdata_t regdata = getRegisterContent(reg);
		BX_CPU(m_Id)->writeEFlags((regdata & 0xFFFFFFFF00000000ULL) | (value & 0xFFFFFFFFULL),
								0xffffffff);
	  #else
		BX_CPU(m_Id)->writeEFlags(value, 0xffffffff);
	  #endif
		BX_CPU(m_Id)->force_flags();
		return;
	}

	#ifndef __puma
	// untested
	if (reg->getId() == RID_CR0) { // CR0 register?
		BX_CPU(m_Id)->SetCR0(value);
		return;
	}

	// untested
	if (reg->getId() == RID_CR2) { // CR2 register?
		BX_CPU(m_Id)->cr2 = value;
		return;
	}

	if (reg->getId() == RID_CR3) { // CR3 register?
		BX_CPU(m_Id)->SetCR3(value);
		return;
	}

	// untested
	if (reg->getId() == RID_CR4) { // CR4 register?
		BX_CPU(m_Id)->SetCR4(value);
		return;
	}

	// untested
	if (reg->getId() == RID_CS) { // CS register?
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_CS], value);
		return;
	}

	// untested
	if (reg->getId() == RID_DS) { // DS register?
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_DS], value);
		return;
	}

	// untested
	if (reg->getId() == RID_ES) { // ES register?
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_ES], value);
		return;
	}

	// untested
	if (reg->getId() == RID_FS) { // FS register?
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_FS], value);
		return;
	}

	// untested
	if (reg->getId() == RID_GS) { // GS register?
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_GS], value);
		return;
	}

	// untested
	if (reg->getId() == RID_SS) { // SS register?
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_SS], value);
		return;
	}
	#endif

	regdata_t* pData;
#ifdef SIM_SUPPORT_64
	if (reg->getId() == RID_PC) // program counter?
		pData = &(BX_CPU(m_Id)->gen_reg[BX_64BIT_REG_RIP].rrx);
	else // 64 bit general purpose registers
		pData = &(BX_CPU(m_Id)->gen_reg[reg->getId()].rrx);
#else // 32 bit mode
	if (reg->getId() == RID_PC)
		pData = &(BX_CPU(m_Id)->gen_reg[BX_32BIT_REG_EIP].dword.erx);
	else // 32 bit general purpose registers
		pData = &(BX_CPU(m_Id)->gen_reg[reg->getId()].dword.erx);
#endif // SIM_SUPPORT_64
	*pData = value;
}

} // end-of-namespace: fail
