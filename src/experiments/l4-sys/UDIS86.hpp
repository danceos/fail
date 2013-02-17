#ifndef __L4SYS_UDIS86_HPP__
  #define __L4SYS_UDIS86_HPP__

#include <udis86.h>
#include "sal/bochs/BochsController.hpp"

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
	/**
	 * creates a new Uds86 object
	 * @param ip the current instruction pointer of the simulator
	 */
	Udis86(fail::address_t ip);
	~Udis86() { free(udis_instr); }
	/**
	 * sets a new input buffer
	 * @param instr the encoded instruction
	 * @param size the size of the instruction
	 */
	void setInputBuffer(unsigned char const *instr, size_t size);
	/**
	 * retrieves the private ud structure of udis86
	 * @returns a reference pointer to a ud_t variable
	 */
	inline ud_t const &getCurrentState() const { return ud_obj; }
	/**
	 * Tries to decode the next instruction from the given buffer.
	 * @returns \c true if a new instruction could be retrieved, \c false if the object has expired
	 */
	inline bool fetchNextInstruction() { return (ud_disassemble(&ud_obj) > 0); }
	/**
	 * Returns the FailBochs equivalent to a UDIS86 GPR identifier.
	 * Attention: this only returns either 32-bit or 64-bit registers, no general IDs
	 * @param udisReg the udis86 GPR ID
	 * @returns the FailBochs GPR ID, usable with the BochsRegisterManager class
	 */
	static inline fail::GPRegisterId udisGPRToFailBochsGPR(ud_type_t udisReg)
	{
		#define REG_CASE(REG) case UD_R_##REG: return fail::RID_##REG
			switch (udisReg) {
		#if BX_SUPPORT_X86_64 // 64 bit register id's:
			REG_CASE(RAX);
			REG_CASE(RCX);
			REG_CASE(RDX);
			REG_CASE(RBX);
			REG_CASE(RSP);
			REG_CASE(RBP);
			REG_CASE(RSI);
			REG_CASE(RDI);
			REG_CASE(R8);
			REG_CASE(R9);
			REG_CASE(R10);
			REG_CASE(R11);
			REG_CASE(R12);
			REG_CASE(R13);
			REG_CASE(R14);
			REG_CASE(R15);
		#else
			REG_CASE(EAX);
			REG_CASE(ECX);
			REG_CASE(EDX);
			REG_CASE(EBX);
			REG_CASE(ESP);
			REG_CASE(EBP);
			REG_CASE(ESI);
			REG_CASE(EDI);
		#endif
			default:
				return fail::RID_LAST_GP_ID;
			}
		#undef REG_CASE
	}
};

#endif // __L4SYS_UDIS86_HPP__
