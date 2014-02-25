#ifndef __INSTRUCTION_IMPORTER_H__
#define __INSTRUCTION_IMPORTER_H__

#include "Importer.hpp"

#include "util/llvmdisassembler/LLVMDisassembler.hpp"

class InstructionImporter : public Importer {
	llvm::OwningPtr<llvm::object::Binary> binary;
	llvm::OwningPtr<fail::LLVMDisassembler> disas;

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
