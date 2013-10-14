#ifndef __GEM5_ARM_CPU_HPP__
#define __GEM5_ARM_CPU_HPP__

#include "../arm/ArmArchitecture.hpp"
#include "../arm/ArmCPUState.hpp"

// gem5 forward declarations:
class System;

namespace fail {

/**
 * \class Gem5ArmCPU
 *
 * \c Gem5ArmCPU is the concrete CPU implementation for the gem5 ARM simulator. It
 * implements the CPU interfaces \c ArmArchitecture and \c ArmCPUState.
 * \c ArmArchitecture refers to architectural information (e.g. register \a count)
 * while \c ArmCPUState encapsulates the CPU state (e.g. register \a content).
 */
class Gem5ArmCPU : public ArmArchitecture, public ArmCPUState {
private:
	unsigned int m_Id; //!< the unique ID of this CPU
	System* m_System; //!< the gem5 system object
public:
	/**
	 * Creates a new gem5 CPU for ARM based targets.
	 * @param id the unique ID of the CPU to be created (the first CPU0 has ID 0)
	 * @param system the gem5 system object
	 */
	Gem5ArmCPU(unsigned int id, System* system) : m_Id(id), m_System(system) { }
	/**
	 * Retrieves the register content from the current gem5 CPU.
	 * @param reg the destination register whose content should be retrieved
	 * @return the content of register \c reg
	 */
	regdata_t getRegisterContent(const Register* reg) const;
	/**
	 * Sets the register content for the  \a current gem5 CPU.
	 * @param reg the (initialized) register object whose content should be set
	 * @param value the new content of the register \c reg
	 */
	void setRegisterContent(const Register* reg, regdata_t value);
	/**
	 * Retrieves the current instruction pointer (IP aka program counter, PC for short)
	 * for the current CPU \c this.
	 * @return the current instruction ptr address
	 */
	address_t getInstructionPointer() const { return getRegisterContent(getRegister(RI_IP)); }
	/**
	 * Retrieves the current stack pointer for the current CPU \c this.
	 * @return the current stack ptr address
	 */
	address_t getStackPointer() const { return getRegisterContent(getRegister(RI_SP)); }
	/**
	 * Retrieves the link register (return address when a function returns) for
	 * the current CPU \c this. See
	 * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0211h/ch02s08s01.html
	 * for further information.
	 * @return the current link register address
	 */
	address_t getLinkRegister() const { return getRegisterContent(getRegister(RI_LR)); }
	/**
	 * Returns the ID of the current CPU.
	 * @return the unique ID of \c this CPU object
	 */
	unsigned int getId() const { return m_Id; }
};

typedef Gem5ArmCPU ConcreteCPU; //!< the concrete CPU type for ARM + gem5

} // end-of-namespace: fail

#endif // __GEM5_ARM_CPU_HPP__
