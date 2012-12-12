#ifndef __BOCHS_REGISTER_HPP__
  #define __BOCHS_REGISTER_HPP__

#include "../Register.hpp"
#include "BochsRegisterIDs.hpp"

#include "bochs.h"

#include <iostream>
#include <cassert>

namespace fail {

/**
 * \class BochsRegister
 * Bochs-specific implementation of x86 registers.
 */
class BochsRegister : public Register {
protected:
	regdata_t* m_pData;
public:
	/**
	 * Constructs a new register object.
	 * @param id the global unique id
	 * @param width width of the register (8, 16, 32 or 64 bit should suffice)
	 * @param link pointer to bochs interal register memory
	 * @param t type of the register
	 */
	BochsRegister(unsigned int id, regwidth_t width, regdata_t* link, RegisterType t)
		: Register(id, t, width), m_pData(link) { }
	/**
	 * Retrieves the data of the register.
	 * @return the current register data
	 */
	regdata_t getData() { return *m_pData; }
	/**
	 * Sets the content of the register.
	 * @param data the new register data to be written
	 */
	void setData(regdata_t data) { *m_pData = data; }
};

/**
 *  \class BxGPReg
 * Bochs-specific implementation of x86 general purpose (GP) registers.
 */
class BxGPReg : public BochsRegister {
public:
	/**
	 * Constructs a new general purpose register.
	 * @param id the global unique id
	 * @param width width of the register (8, 16, 32 or 64 bit should
	 *        suffice)
	 * @param link pointer to bochs interal register memory
	 */
	BxGPReg(unsigned int id, regwidth_t width, regdata_t* link)
		: BochsRegister(id, width, link, RT_GP) { }
};

/**
 *  \class BxPCReg
 * Bochs-specific implementation of the x86 program counter register.
 */
class BxPCReg : public BochsRegister {
public:
	/**
	 * Constructs a new program counter register.
	 * @param id the global unique id
	 * @param width width of the register (8, 16, 32 or 64 bit should
	 *        suffice)
	 * @param link pointer to bochs internal register memory
	 */
	BxPCReg(unsigned int id, regwidth_t width, regdata_t* link)
		: BochsRegister(id, width, link, RT_PC) { }
};

/**
 * \class BxFlagsReg
 * Bochs-specific implementation of the FLAGS status register.
 */
class BxFlagsReg : public BochsRegister {
public:
	/**
	 * Constructs a new FLAGS status register. The refenced FLAGS are
	 * allocated as follows:
	 * --------------------------------------------------
	 * 31|30|29|28| 27|26|25|24| 23|22|21|20| 19|18|17|16
	 * ==|==|=====| ==|==|==|==| ==|==|==|==| ==|==|==|==
	 *  0| 0| 0| 0|  0| 0| 0| 0|  0| 0|ID|VP| VF|AC|VM|RF
	 *
	 * 15|14|13|12| 11|10| 9| 8|  7| 6| 5| 4|  3| 2| 1| 0
	 * ==|==|=====| ==|==|==|==| ==|==|==|==| ==|==|==|==
	 *  0|NT| IOPL| OF|DF|IF|TF| SF|ZF| 0|AF|  0|PF| 1|CF
	 * --------------------------------------------------
	 * @param id the global unique id
	 * @param link pointer to bochs internal register memory
	 */
	BxFlagsReg(unsigned int id, regdata_t* link)
		: BochsRegister(id, 32, link, RT_ST) { }

	/**
	 * Returns \c true if the corresponding flag is set, or \c false
	 * otherwise.
	 */
	bool getCarryFlag() const             { return BX_CPU(0)->get_CF();   }
	bool getParityFlag() const            { return BX_CPU(0)->get_PF();   }
	bool getZeroFlag() const              { return BX_CPU(0)->get_ZF();   }
	bool getSignFlag() const              { return BX_CPU(0)->get_SF();   }
	bool getOverflowFlag() const          { return BX_CPU(0)->get_OF();   }
	bool getTrapFlag() const              { return BX_CPU(0)->get_TF();   }
	bool getInterruptFlag() const         { return BX_CPU(0)->get_IF();   }
	bool getDirectionFlag() const         { return BX_CPU(0)->get_DF();   }
	unsigned getIOPrivilegeLevel() const  { return BX_CPU(0)->get_IOPL(); }
	bool getNestedTaskFlag() const        { return BX_CPU(0)->get_NT();   }
	bool getResumeFlag() const            { return BX_CPU(0)->get_RF();   }
	bool getVMFlag() const                { return BX_CPU(0)->get_VM();   }
	bool getAlignmentCheckFlag() const    { return BX_CPU(0)->get_AC();   }
	bool getVInterruptFlag() const        { return BX_CPU(0)->get_VIF();  }
	bool getVInterruptPendingFlag() const { return BX_CPU(0)->get_VIP();  }
	bool getIdentificationFlag() const    { return BX_CPU(0)->get_ID();   }

	/**
	 * Sets/resets various status FLAGS.
	 */
	void setCarryFlag(bool bit)             { BX_CPU(0)->set_CF(bit);   }
	void setParityFlag(bool bit)            { BX_CPU(0)->set_PF(bit);   }
	void setZeroFlag(bool bit)              { BX_CPU(0)->set_ZF(bit);   }
	void setSignFlag(bool bit)              { BX_CPU(0)->set_SF(bit);   }
	void setOverflowFlag(bool bit)          { BX_CPU(0)->set_OF(bit);   }
	void setTrapFlag(bool bit)              { BX_CPU(0)->set_TF(bit);   }
	void setInterruptFlag(bool bit)         { BX_CPU(0)->set_IF(bit);   }
	void setDirectionFlag(bool bit)         { BX_CPU(0)->set_DF(bit);   }
	void setIOPrivilegeLevel(unsigned lvl)  { BX_CPU(0)->set_IOPL(lvl); }
	void setNestedTaskFlag(bool bit)        { BX_CPU(0)->set_NT(bit);   }
	void setResumeFlag(bool bit)            { BX_CPU(0)->set_RF(bit);   }
	void setVMFlag(bool bit)                { BX_CPU(0)->set_VM(bit);   }
	void setAlignmentCheckFlag(bool bit)    { BX_CPU(0)->set_AC(bit);   }
	void setVInterruptFlag(bool bit)        { BX_CPU(0)->set_VIF(bit);  }
	void setVInterruptPendingFlag(bool bit) { BX_CPU(0)->set_VIP(bit);  }
	void setIdentificationFlag(bool bit)    { BX_CPU(0)->set_ID(bit);   }

	/**
	 * Sets the content of the status register.
	 * @param data the new register data to be written; note that only the
	 *        32 lower bits are used (bits 32-63 are ignored in 64 bit mode)
	 */
	void setData(regdata_t data)
	{
	  #ifdef BX_SUPPORT_X86_64
		// We are in 64 bit mode: Just assign the lower 32 bits!
		(*m_pData) = ((*m_pData) & 0xFFFFFFFF00000000ULL) | (data & 0xFFFFFFFFULL);
	  #else
		*m_pData = data;
	  #endif
	}
};

/**
 * \class BochsRegisterManager
 * Bochs-specific implementation of the RegisterManager.
 */
class BochsRegisterManager : public RegisterManager {
public:
	/**
	 * Returns the current instruction pointer.
	 * @return the current eip
	 */
	address_t getInstructionPointer()
	{
		return static_cast<address_t>(getSetOfType(RT_PC)->first()->getData());
	}
	/**
	 * Returns the top address of the stack.
	 * @return the starting address of the stack
	 */
	address_t getStackPointer()
	{
	  #if BX_SUPPORT_X86_64
		return static_cast<address_t>(getRegister(RID_RSP)->getData());
	  #else
		return static_cast<address_t>(getRegister(RID_ESP)->getData());
	  #endif
	}
	/**
	 * Retrieves the base ptr (holding the address of the
	 * current stack frame).
	 * @return the base pointer
	 */
	address_t getBasePointer()
	{
	  #if BX_SUPPORT_X86_64
		return static_cast<address_t>(getRegister(RID_RBP)->getData());
	  #else
		return static_cast<address_t>(getRegister(RID_EBP)->getData());
	  #endif
	}
};

} // end-of-namespace: fail

#endif // __BOCHS_REGISTER_HPP__
