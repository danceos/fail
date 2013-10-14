#ifndef __DISASSEMBLER_HPP
#define __DISASSEMBLER_HPP

#include <string>
#include "Logger.hpp"
#include "sal/SALConfig.hpp"
#include <map>
#include <ostream>

namespace fail {

struct DISASSEMBLER {
	//! Inform about failed disassembly
	static const std::string FAILED;
};

/**
 * @class Instruction
 * @brief An Instruction represents an disassembled opcode
 */
struct Instruction {
	address_t address; //!< The instruction address
	regdata_t opcode;  //!< The opcode itself
	std::string instruction; //!< The disassembled instruction
	std::string comment;     //!< Comment (rest of line after ; )
	Instruction(address_t address = ADDR_INV, regdata_t opcode = 0, const std::string& instr = DISASSEMBLER::FAILED, const std::string& comment = "")
		: address(address), opcode(opcode), instruction(instr), comment(comment) { };
};
//<! This allows to print an Instruction via Logger or cout
std::ostream& operator <<(std::ostream & os, const fail::Instruction & i);

class Disassembler {

public:
	/**
	 * Constructor.
	 */
	Disassembler();

	/**
	 * Get disassembler instruction
	 * @param address The instruction address
	 * @return The according disassembled instruction if found, else DISASSEMBLER::FAILED
	 */
	const Instruction & disassemble(address_t address) const;

	/**
	 * Test if there is an instruction at a given address
	 * @param address The address to test
	 * @return true if found, else false
	 */
	bool hasInstructionAt(address_t address) const {
		return m_code.find(address) != m_code.end();;
	};

	/**
	 * Evaluate new ELF file
	 * @param elfpath Path to ELF file.
	 * @return Number of disassembled lines.
	 */
	int init(const char* elfpath);

	/**
	 * Evaluate new ELF file from env variable $FAIL_ELF_PATH
	 * @return Number of disassembled lines.
	 * @note The path is guessed from a FAIL_ELF_PATH environment variable
	 */
	int init(void);

private:
	Logger m_log;
	typedef std::map<address_t, Instruction> InstructionMap_t;
	InstructionMap_t m_code;
	void evaluate(const std::string &);
};
} // end of namespace

#endif // DISASSEMBLER_HPP
