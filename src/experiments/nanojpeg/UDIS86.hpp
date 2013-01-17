#ifndef __UDIS86_HPP__
  #define __UDIS86_HPP__

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
	 * sets the current IP
	 * @param ip the current IP
	 */
	void setIP(fail::address_t ip) { ud_set_pc(&ud_obj, ip); }
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
	inline ud_t &getCurrentState() { return ud_obj; }
	/**
	 * Tries to decode the next instruction from the given buffer.
	 * @returns \c true if a new instruction could be retrieved, \c false if the object has expired
	 */
	inline bool fetchNextInstruction() { return (ud_disassemble(&ud_obj) > 0); }
};

#endif // __UDIS86_HPP__
