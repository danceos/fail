#ifndef __INSTRUCTION_IMPORTER_H__
#define __INSTRUCTION_IMPORTER_H__

#include "Importer.hpp"

#if defined(BUILD_CAPSTONE_DISASSEMBLER)
#include "util/capstonedisassembler/CapstoneDisassembler.hpp"
#elif defined(BUILD_LLVM_DISASSEMBLER)
#include "util/llvmdisassembler/LLVMDisassembler.hpp"
#endif

class InstructionImporter : public Importer {
#if defined(BUILD_CAPSTONE_DISASSEMBLER)
	bool isDisassembled = false;
	std::unique_ptr<fail::CapstoneDisassembler> disas;
#elif defined(BUILD_LLVM_DISASSEMBLER)
	llvm::object::Binary *binary = 0;
	std::unique_ptr<fail::LLVMDisassembler> disas;
#endif

protected:
	virtual bool handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
								 Trace_Event &ev);
	virtual bool handle_mem_event(fail::simtime_t curtime, instruction_count_t instr,
								  Trace_Event &ev) {
		/* ignore on purpose */
		return true;
	}

	void getAliases(std::deque<std::string> *aliases) {
		aliases->push_back("InstructionImporter");
		aliases->push_back("code");
	}
};

#endif
