#ifndef __OVPPLATFORM_HPP__
#define __OVPPLATFORM_HPP__

#include <string>

/**
 * \class OVPPlatform
 * OVPPlatform is the layer/interface which connects SAL and the OVP platform.
 */
class OVPPlatform {

public:
	/**
	 * The current OVP platform has to set a pointer to itself so that
	 * OVPPlatform can execute OVP functions using the correct platform
	 * @param ovpcpu void ptr to the OVPCpu object
	 */
	void setCpu(void *);

	/**
	 * Set value to a register
	 * @param link Pointer to the OVP register
	 * @param val Value the register is set to
	 */
	void setRegisterData(void *, unsigned int);

	/**
	 * Retrieves value from a register
	 * @param link Pointer to the OVP register
	 * @return value of the register
	 */
	unsigned int getRegisterData(void *);

	/**
	 * Get the program counter
	 * @return current program counter
	 */
	uint32_t getPC();

	/**
	 * Set the program counter
	 */
	void setPC(uint32_t);

	/**
	 * Get the stack pointer
	 * @return current stack pointer
	 */
	uint32_t getSP();

};

#endif
