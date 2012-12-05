#ifndef __ARM_CPU_STATE_HPP__
  #define __ARM_CPU_STATE_HPP__

#include "../CPU.hpp"
#include "../CPUState.hpp"

namespace fail {

class ArmCPUState : public CPUState {
public:
	virtual regdata_t getRegisterContent(Register* reg) = 0;

	virtual address_t getInstructionPointer() = 0;
	virtual address_t getStackPointer() = 0;
	/**
	 * Returns the current Link Register.
	 * @return the current lr
	 */
	virtual address_t getLinkRegister() = 0;
};

// TODO: Enum for misc registers

} // end-of-namespace: fail

#endif // __ARM_CPU_STATE_HPP__
