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

	// FPU
	case RID_FSW:
		return BX_CPU(m_Id)->the_i387.swd;
	case RID_FCW:
		return BX_CPU(m_Id)->the_i387.cwd;
	case RID_FTW:
		return BX_CPU(m_Id)->the_i387.twd;
	case RID_FPR0_LO:
	case RID_FPR1_LO:
	case RID_FPR2_LO:
	case RID_FPR3_LO:
	case RID_FPR4_LO:
	case RID_FPR5_LO:
	case RID_FPR6_LO:
	case RID_FPR7_LO:
		return BX_CPU(m_Id)->the_i387.st_space[(reg->getId() - RID_FPR0_LO) / 2].fraction;
	case RID_FPR0_HI:
	case RID_FPR1_HI:
	case RID_FPR2_HI:
	case RID_FPR3_HI:
	case RID_FPR4_HI:
	case RID_FPR5_HI:
	case RID_FPR6_HI:
	case RID_FPR7_HI:
		return BX_CPU(m_Id)->the_i387.st_space[(reg->getId() - RID_FPR0_HI) / 2].exp;

	// vector units
	case RID_XMM0_LO:
	case RID_XMM1_LO:
	case RID_XMM2_LO:
	case RID_XMM3_LO:
	case RID_XMM4_LO:
	case RID_XMM5_LO:
	case RID_XMM6_LO:
	case RID_XMM7_LO:
#ifdef SIM_SUPPORT_64
	case RID_XMM8_LO:
	case RID_XMM9_LO:
	case RID_XMM10_LO:
	case RID_XMM11_LO:
	case RID_XMM12_LO:
	case RID_XMM13_LO:
	case RID_XMM14_LO:
	case RID_XMM15_LO:
#endif
		return BX_CPU(m_Id)->xmm[(reg->getId() - RID_XMM0_LO) / 2].xmm_s64[0];
	case RID_XMM0_HI:
	case RID_XMM1_HI:
	case RID_XMM2_HI:
	case RID_XMM3_HI:
	case RID_XMM4_HI:
	case RID_XMM5_HI:
	case RID_XMM6_HI:
	case RID_XMM7_HI:
#ifdef SIM_SUPPORT_64
	case RID_XMM8_HI:
	case RID_XMM9_HI:
	case RID_XMM10_HI:
	case RID_XMM11_HI:
	case RID_XMM12_HI:
	case RID_XMM13_HI:
	case RID_XMM14_HI:
	case RID_XMM15_HI:
#endif
		return BX_CPU(m_Id)->xmm[(reg->getId() - RID_XMM0_HI) / 2].xmm_s64[1];
	case RID_MXCSR:
		return BX_CPU(m_Id)->mxcsr.mxcsr;

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
		break;
	#ifndef __puma
	case RID_CR0:
		// untested
		BX_CPU(m_Id)->SetCR0(value);
		break;
	case RID_CR2:
		// untested
		BX_CPU(m_Id)->cr2 = value;
		break;
	case RID_CR3:
		BX_CPU(m_Id)->SetCR3(value);
		break;
	case RID_CR4:
		// untested
		BX_CPU(m_Id)->SetCR4(value);
		break;
	case RID_CS:
		// untested
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_CS], value);
		break;
	case RID_DS:
		// untested
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_DS], value);
		break;
	case RID_ES:
		// untested
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_ES], value);
		break;
	case RID_FS:
		// untested
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_FS], value);
		break;
	case RID_GS:
		// untested
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_GS], value);
		break;
	case RID_SS:
		// untested
		BX_CPU(m_Id)->load_seg_reg(&BX_CPU(m_Id)->sregs[BX_SEG_REG_SS], value);
		break;
	#endif

	// FPU
	case RID_FSW:
		BX_CPU(m_Id)->the_i387.swd = value;
		break;
	case RID_FCW:
		BX_CPU(m_Id)->the_i387.cwd = value;
		break;
	case RID_FTW:
		BX_CPU(m_Id)->the_i387.twd = value;
		break;
	case RID_FPR0_LO:
	case RID_FPR1_LO:
	case RID_FPR2_LO:
	case RID_FPR3_LO:
	case RID_FPR4_LO:
	case RID_FPR5_LO:
	case RID_FPR6_LO:
	case RID_FPR7_LO:
		BX_CPU(m_Id)->the_i387.st_space[(reg->getId() - RID_FPR0_LO) / 2].fraction = value;
		break;
	case RID_FPR0_HI:
	case RID_FPR1_HI:
	case RID_FPR2_HI:
	case RID_FPR3_HI:
	case RID_FPR4_HI:
	case RID_FPR5_HI:
	case RID_FPR6_HI:
	case RID_FPR7_HI:
		BX_CPU(m_Id)->the_i387.st_space[(reg->getId() - RID_FPR0_HI) / 2].exp = value;
		break;

	// vector units
	case RID_XMM0_LO:
	case RID_XMM1_LO:
	case RID_XMM2_LO:
	case RID_XMM3_LO:
	case RID_XMM4_LO:
	case RID_XMM5_LO:
	case RID_XMM6_LO:
	case RID_XMM7_LO:
#ifdef SIM_SUPPORT_64
	case RID_XMM8_LO:
	case RID_XMM9_LO:
	case RID_XMM10_LO:
	case RID_XMM11_LO:
	case RID_XMM12_LO:
	case RID_XMM13_LO:
	case RID_XMM14_LO:
	case RID_XMM15_LO:
#endif
		BX_CPU(m_Id)->xmm[(reg->getId() - RID_XMM0_LO) / 2].xmm_s64[0] = value;
		break;
	case RID_XMM0_HI:
	case RID_XMM1_HI:
	case RID_XMM2_HI:
	case RID_XMM3_HI:
	case RID_XMM4_HI:
	case RID_XMM5_HI:
	case RID_XMM6_HI:
	case RID_XMM7_HI:
#ifdef SIM_SUPPORT_64
	case RID_XMM8_HI:
	case RID_XMM9_HI:
	case RID_XMM10_HI:
	case RID_XMM11_HI:
	case RID_XMM12_HI:
	case RID_XMM13_HI:
	case RID_XMM14_HI:
	case RID_XMM15_HI:
#endif
		BX_CPU(m_Id)->xmm[(reg->getId() - RID_XMM0_HI) / 2].xmm_s64[1] = value;
		break;
	case RID_MXCSR:
		BX_CPU(m_Id)->mxcsr.mxcsr = value;
		break;

#ifdef SIM_SUPPORT_64
	case RID_PC: // program counter
		BX_CPU(m_Id)->gen_reg[BX_64BIT_REG_RIP].rrx = value;
		break;
	default: // 64 bit general purpose registers
		BX_CPU(m_Id)->gen_reg[reg->getId()].rrx = value;
		break;
#else // 32 bit mode
	case RID_PC: // program counter
		BX_CPU(m_Id)->gen_reg[BX_32BIT_REG_EIP].dword.erx = value;
		break;
	default: // 32 bit general purpose registers
		BX_CPU(m_Id)->gen_reg[reg->getId()].dword.erx = value;
		break;
#endif // SIM_SUPPORT_64
	}
}

} // end-of-namespace: fail
