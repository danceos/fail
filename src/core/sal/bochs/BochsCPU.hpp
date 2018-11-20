#ifndef __BOCHS_CPU_HPP__
#define __BOCHS_CPU_HPP__

#include "../x86/X86Architecture.hpp"
#include "../x86/X86CPUState.hpp"

#include "bochs.h"
#include "cpu/cpu.h"

namespace fail {

/**
 * \class BochsCPU
 *
 * \c BochsCPU is the concrete CPU implementation for the Bochs x86 simulator. It
 * implements the CPU interfaces \c X86Architecture and \c X86CPUState.
 * \c X86Architecture refers to architectural information (e.g. register \a count)
 * while \c X86CPUState encapsulates the CPU state (e.g. register \a content).
 *
 */
class BochsCPU : public X86Architecture, public X86CPUState {
private:
	unsigned int m_Id; //!< the numeric CPU identifier (ID)
public:
	/**
	 * Initializes the Bochs CPU with the provided \c id.
	 * @param id the CPU identifier (the 1st CPU is CPU0 -> id = 0, and so forth)
	 */
	BochsCPU(unsigned int id) : m_Id(id) { }
	/**
	 * Virtual Destructor is required.
	 */
	virtual ~BochsCPU() { }
		/**
	 * Retrieves the content of the register \c reg.
	 * @param reg the register pointer of interest (cannot be \c NULL)
	 * @return the content of the register \c reg
	 */
	regdata_t getRegisterContent(const Register* reg) const;
	/**
	 * Sets the content of the register \c reg to \c value.
	 * @param reg the destination register object pointer (cannot be \c NULL)
	 * @param value the new value to assign
	 */
	void setRegisterContent(const Register* reg, regdata_t value);
	/**
	 * Returns the current instruction pointer (aka program counter).
	 * @return the current (e)ip register content
	 */
	address_t getInstructionPointer() const { return getRegisterContent(getRegister(RID_PC)); }
	/**
	 * Returns the current stack pointer.
	 * @return the current (e)sp register content
	 */
	address_t getStackPointer() const { return getRegisterContent(getRegister(RID_CSP)); }
	/**
	 * Returns the current base pointer.
	 * @return the current (e)bp register content
	 */
	address_t getBasePointer() const { return getRegisterContent(getRegister(RID_CBP)); }
	/**
	 * Returns the current (E)FLAGS.
	 * @return the current (E)FLAGS processor register content
	 */
	regdata_t getFlagsRegister() const { return getRegisterContent(getRegister(RID_FLAGS)); }
	/**
	 * Returns \c true if the corresponding flag is set, or \c false
	 * otherwise.
	 */
	bool getCarryFlag() const             { return BX_CPU(m_Id)->get_CF();   }
	bool getParityFlag() const            { return BX_CPU(m_Id)->get_PF();   }
	bool getZeroFlag() const              { return BX_CPU(m_Id)->get_ZF();   }
	bool getSignFlag() const              { return BX_CPU(m_Id)->get_SF();   }
	bool getOverflowFlag() const          { return BX_CPU(m_Id)->get_OF();   }
	bool getTrapFlag() const              { return BX_CPU(m_Id)->get_TF();   }
	bool getInterruptFlag() const         { return BX_CPU(m_Id)->get_IF();   }
	bool getDirectionFlag() const         { return BX_CPU(m_Id)->get_DF();   }
	unsigned getIOPrivilegeLevel() const  { return BX_CPU(m_Id)->get_IOPL(); }
	bool getNestedTaskFlag() const        { return BX_CPU(m_Id)->get_NT();   }
	bool getResumeFlag() const            { return BX_CPU(m_Id)->get_RF();   }
	bool getVMFlag() const                { return BX_CPU(m_Id)->get_VM();   }
	bool getAlignmentCheckFlag() const    { return BX_CPU(m_Id)->get_AC();   }
	bool getVInterruptFlag() const        { return BX_CPU(m_Id)->get_VIF();  }
	bool getVInterruptPendingFlag() const { return BX_CPU(m_Id)->get_VIP();  }
	bool getIdentificationFlag() const    { return BX_CPU(m_Id)->get_ID();   }
	/**
	 * Sets/resets various status FLAGS.
	 */
	void setCarryFlag(bool bit)             { BX_CPU(m_Id)->set_CF(bit);   }
	void setParityFlag(bool bit)            { BX_CPU(m_Id)->set_PF(bit);   }
	void setZeroFlag(bool bit)              { BX_CPU(m_Id)->set_ZF(bit);   }
	void setSignFlag(bool bit)              { BX_CPU(m_Id)->set_SF(bit);   }
	void setOverflowFlag(bool bit)          { BX_CPU(m_Id)->set_OF(bit);   }
	void setTrapFlag(bool bit)              { BX_CPU(m_Id)->set_TF(bit);   }
	void setInterruptFlag(bool bit)         { BX_CPU(m_Id)->set_IF(bit);   }
	void setDirectionFlag(bool bit)         { BX_CPU(m_Id)->set_DF(bit);   }
	void setIOPrivilegeLevel(unsigned lvl)  { BX_CPU(m_Id)->set_IOPL(lvl); }
	void setNestedTaskFlag(bool bit)        { BX_CPU(m_Id)->set_NT(bit);   }
	void setResumeFlag(bool bit)            { BX_CPU(m_Id)->set_RF(bit);   }
	void setVMFlag(bool bit)                { BX_CPU(m_Id)->set_VM(bit);   }
	void setAlignmentCheckFlag(bool bit)    { BX_CPU(m_Id)->set_AC(bit);   }
	void setVInterruptFlag(bool bit)        { BX_CPU(m_Id)->set_VIF(bit);  }
	void setVInterruptPendingFlag(bool bit) { BX_CPU(m_Id)->set_VIP(bit);  }
	void setIdentificationFlag(bool bit)    { BX_CPU(m_Id)->set_ID(bit);   }
	/**
	 * Returns the current id of this CPU.
	 * @return the current id
	 */
	unsigned int getId() const { return m_Id; }
};

typedef BochsCPU ConcreteCPU; //!< the concrete BochsCPU type

} // end-of-namespace: fail

#endif // __BOCHS_CPU_HPP__
