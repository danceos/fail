#ifndef __T32_REGISTER_HPP__
  #define __T32_REGISTER_HPP__

#include "../Register.hpp"

#include <iostream>
#include <cassert>

namespace fail {

/**
 * \class T32Register
 * T32-specific implementation of ?? registers.  TODO.
 */
class T32Register : public Register {
public:
	T32Register(unsigned int id, regwidth_t width, regdata_t* link, RegisterType t)
		: Register(id, t, width) { }
	regdata_t getData() { return 0; /* TODO */ }
	void setData(regdata_t data) { /* TODO */ }
};

/**
 * \class T32RegisterManager
 * T32-specific implementation of the RegisterManager.  TODO.
 */
class T32RegisterManager : public RegisterManager {
public:
	address_t getInstructionPointer()
	{
		return static_cast<address_t>(0); /* TODO */
	}
	address_t getStackPointer()
	{
		return static_cast<address_t>(0); /* TODO */
	}
	address_t getBasePointer()
	{
		return static_cast<address_t>(0); /* TODO */
	}
};

} // end-of-namespace: fail

#endif // __T32_REGISTER_HPP__
