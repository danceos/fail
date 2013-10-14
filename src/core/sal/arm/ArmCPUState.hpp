#ifndef __ARM_CPU_STATE_HPP__
#define __ARM_CPU_STATE_HPP__

#include "../CPU.hpp"
#include "../CPUState.hpp"

namespace fail {

/**
 * \class ArmCPUState
 * This class represents the current state of a ARM based CPU. A final CPU class
 * need to implement \c ArmCPUState and \c ArmArchitecture.
 */
class ArmCPUState : public CPUState {
public:
	/**
	 * Returns the current Link Register.
	 * @return the current lr
	 */
	virtual address_t getLinkRegister() const = 0;
	virtual ~ArmCPUState() { }
};

// TODO: Enum for misc registers

} // end-of-namespace: fail

#endif // __ARM_CPU_STATE_HPP__
