#ifndef __OVP_REGISTER_HPP__
  #define __OVP_REGISTER_HPP__

#include "../Register.hpp"
#include "ovp/OVPPlatform.hpp"
//#include "ovp/OVPStatusRegister.hpp"

extern OVPPlatform ovpplatform;

namespace fail {

/**
 * \class OVPRegister
 * OVP-specific implementation of x86 registers.
 */
class OVPRegister : public Register {
private:
	void *reg;
public:
	/**
	 * Constructs a new register object.
	 * @param id the unique id of this register (simulator specific)
	 * @param width width of the register (8, 16, 32 or 64 bit should suffice)
	 * @param link pointer to bochs internal register memory
	 * @param t type of the register
	 */
	OVPRegister(unsigned int id, regwidth_t width, void* link, RegisterType t)
		: Register(id, t, width) { reg = link; }
	/**
	 * Retrieves the data of the register.
	 * @return the current register data
	 */
	regdata_t getData() { return ovpplatform.getRegisterData(reg); }
	/**
	 * Sets the content of the register.
	 * @param data the new register data to be written
	 */
	void setData(regdata_t data) { ovpplatform.setRegisterData(reg, data); }
};

/**
 * \class OVPRegister
 * OVP-specific implementation of the RegisterManager.
 */
class OVPRegisterManager : public RegisterManager {
public:
	/**
	 * Returns the current instruction pointer.
	 * @return the current eip
	 */
	virtual address_t getInstructionPointer()
	{
		return ovpplatform.getPC();
	}
	/**
	 * Retruns the top address of the stack.
	 * @return the starting address of the stack
	 */
	virtual address_t getStackPointer()
	{
		return ovpplatform.getSP();
	}
	/**
	 * Retrieves the base ptr (holding the address of the
	 * current stack frame).
	 * @return the base pointer
	 */
	virtual address_t getBasePointer()
	{
		return 0;
	}
};

} // end-of-namespace: fail

#endif // __OVP_REGISTER_HPP__
