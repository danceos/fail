#ifndef __UDIS86_HPP__
  #define __UDIS86_HPP__

#include <udis86.h>
#include "sal/bochs/BochsRegister.hpp"

/**
 * \class Udis86
 *
 * \brief Class to disassemble instructions
 *
 * This class disassembles a stream of machine code instruction
 * by instruction.
 * It provides a (thin) wrapper around the C API of UDIS86.
 */
class Udis86
{
private:
	ud_t ud_obj; //<! the ud object of udis86
	unsigned char *udis_instr; //<! the instruction buffer for UDIs86
	size_t udis_instr_size; //<! the size of the instruction buffer
public:
	Udis86(unsigned char const *instr, size_t size, fail::address_t ip);
	~Udis86();
	/**
	 * retrieves the private ud structure of udis86
	 * @returns a reference pointer to a ud_t variable
	 */
	inline ud_t const &getCurrentState() const { return ud_obj; }
	/**
	 * Tries to decode the next instruction from the given buffer.
	 * @returns \c true if a new instruction could be retrieved, \c false if the object has expired
	 */
	bool fetchNextInstruction();
	/**
	 * Returns the FailBochs equivalent to a UDIS86 GPR identifier.
	 * Attention: this only returns either 32-bit or 64-bit registers, no general IDs
	 * @param udisReg the udis86 GPR ID
	 * @returns the FailBochs GPR ID, usable with the BochsRegisterManager class
	 */
	static fail::GPRegisterId udisGPRToFailBochsGPR(ud_type_t udisReg);
};

#endif // __UDIS86_HPP__
