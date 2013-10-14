#ifndef __ARMMEMORYINSTRUCITON_HPP__
#define __ARMMEMORYINSTRUCITON_HPP__

#include "../MemoryInstruction.hpp"
#include "util/Disassembler.hpp"
#include "util/ElfReader.hpp"
#include "ArmDisassembler.hpp"

namespace fail {

class ArmMemoryInstructionAnalyzer : public MemoryInstructionAnalyzer {
	fail::ElfReader m_elf;
	fail::Disassembler m_dis;

	address_t findPrevious(address_t addr);
	void evaluate(arm_instruction & inst, MemoryInstruction& result);
	bool eval_ca9(address_t address, MemoryInstruction& result);
	bool eval_cm3(address_t address, MemoryInstruction& result);

	public:

	ArmMemoryInstructionAnalyzer() {
		m_dis.init();
	};

	bool eval(address_t opcode, MemoryInstruction & result);
};

} //end of namespace fail

#endif //  __ARMMEMORYINSTRUCITON_HPP__
