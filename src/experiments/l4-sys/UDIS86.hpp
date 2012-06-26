#if 0
		// temporarily disabled to make the code in the repository compile - will soon be fixed
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
public:
	Udis86(const unsigned char *instr, size_t size);
	/**
	 * retrieves the private ud structure of udis86
	 * @returns a reference pointer to a ud_t variable
	 */
	inline const ud_t &getCurrentState() const { return ud_obj; }
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
#endif
