#ifndef __CPU_STATE_HPP__
#define __CPU_STATE_HPP__

#include <cstring>
#include <vector>

#include "Register.hpp"

namespace fail {

/**
 * \class CPUState
 * This is the base class for the CPU state without any architecture specific additions. It contains
 * pure virtual functions for e.g. register access and have to be overridden in the backend
 * implementation.
 */
class CPUState {
public:
	/**
	 * Gets the content of the passed Register.
	 * @param reg the register to get the content from
	 */
	virtual regdata_t getRegisterContent(const Register* reg) const = 0;
	/**
	 * Writes the passed value into the given register.
	 * @param reg the register that should be written to
	 * @param value the value that should be written into the register
	 */
	virtual void setRegisterContent(const Register* reg, regdata_t value) = 0;
	/**
	 * Returns the current instruction pointer.
	 * @return the current eip
	 */
	virtual address_t getInstructionPointer() const = 0;
	/**
	 * Returns the top address of the stack.
	 * @return the starting address of the stack
	 */
	virtual address_t getStackPointer() const = 0;
	/**
	 * Check whether the interrupt should be suppressed.
	 * @param interruptNum the interrupt-type id
	 * @return \c true if the interrupt is suppressed, \c false oterwise
	 */
	bool isSuppressedInterrupt(unsigned interruptNum) const;
	/**
	 * Add a Interrupt to the list of suppressed.
	 * @param interruptNum the interrupt-type id
	 * @return \c true if sucessfully added, \c false otherwise (already
	 *         existing)
	 */
	bool addSuppressedInterrupt(unsigned interruptNum);
	/**
	 * Remove a Interrupt from the list of suppressed.
	 * @param interruptNum the interrupt-type id
	 * @return \c true if sucessfully removed, \c false otherwise (not found)
	 */
	bool removeSuppressedInterrupt(unsigned interruptNum);
protected:
	std::vector<unsigned> m_SuppressedInterrupts;
};

// FIXME (see CPU.cc): Weird, homeless global variable
extern int interrupt_to_fire;

} // end-of-namespace: fail

#endif // __CPU_STATE_HPP__
