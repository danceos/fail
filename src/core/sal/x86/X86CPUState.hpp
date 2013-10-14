#ifndef __X86_CPU_STATE_HPP__
#define __X86_CPU_STATE_HPP__

#include "../CPU.hpp"
#include "../CPUState.hpp"

namespace fail {

/**
 * \class X86CPUState
 * This class represents the current state of a x86 based CPU. A final CPU class
 * implemention need to implement \c X86CPUState and \c X86Architecture.
 */
class X86CPUState : public CPUState {
public:
	/**
	 * Returns the current content of the base pointer register.
	 * @return the current (e)bp
	 */
	virtual address_t getBasePointer() const = 0;
	/**
	 * Returns the current (E)FLAGS.
	 * @return the current (E)FLAGS processor register content
	 */
	virtual regdata_t getFlagsRegister() const = 0;

	/**
	 * Returns \c true if the corresponding flag is set, or \c false
	 * otherwise.
	 */
	virtual bool getCarryFlag() const = 0;
	virtual bool getParityFlag() const = 0;
	virtual bool getZeroFlag() const = 0;
	virtual bool getSignFlag() const = 0;
	virtual bool getOverflowFlag() const = 0;
	virtual bool getTrapFlag() const = 0;
	virtual bool getInterruptFlag() const = 0;
	virtual bool getDirectionFlag() const = 0;
	virtual unsigned getIOPrivilegeLevel() const = 0;
	virtual bool getNestedTaskFlag() const = 0;
	virtual bool getResumeFlag() const = 0;
	virtual bool getVMFlag() const = 0;
	virtual bool getAlignmentCheckFlag() const = 0;
	virtual bool getVInterruptFlag() const = 0;
	virtual bool getVInterruptPendingFlag() const = 0;
	virtual bool getIdentificationFlag() const = 0;
	/**
	 * Sets/resets various status FLAGS.
	 */
	virtual void setCarryFlag(bool bit) = 0;
	virtual void setParityFlag(bool bit) = 0;
	virtual void setZeroFlag(bool bit) = 0;
	virtual void setSignFlag(bool bit) = 0;
	virtual void setOverflowFlag(bool bit) = 0;
	virtual void setTrapFlag(bool bit) = 0;
	virtual void setInterruptFlag(bool bit) = 0;
	virtual void setDirectionFlag(bool bit) = 0;
	virtual void setIOPrivilegeLevel(unsigned lvl) = 0;
	virtual void setNestedTaskFlag(bool bit) = 0;
	virtual void setResumeFlag(bool bit) = 0;
	virtual void setVMFlag(bool bit) = 0;
	virtual void setAlignmentCheckFlag(bool bit) = 0;
	virtual void setVInterruptFlag(bool bit) = 0;
	virtual void setVInterruptPendingFlag(bool bit) = 0;
	virtual void setIdentificationFlag(bool bit) = 0;
};

} // end-of-namespace: fail

#endif // __X86_CPU_STATE_HPP__
