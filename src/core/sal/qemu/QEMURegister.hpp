#ifndef __QEMU_REGISTER_HPP__
#define __QEMU_REGISTER_HPP__

#include "../Register.hpp"

#include <iostream>
#include <cassert>

namespace fail {

/**
 * \class QEMURegister
 * QEMU-specific implementation of x86 registers.  TODO.
 */
class QEMURegister : public Register {
public:
	QEMURegister(unsigned int id, regwidth_t width, regdata_t* link, RegisterType t)
		: Register(id, t, width) { }
	regdata_t getData() { return 0; /* TODO */ }
	void setData(regdata_t data) { /* TODO */ }
};

/**
 * \class QEMURegisterManager
 * QEMU-specific implementation of the RegisterManager.  TODO.
 */
class QEMURegisterManager : public RegisterManager {
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

#endif // __QEMU_REGISTER_HPP__
