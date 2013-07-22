#ifndef __INSTRUCTION_IMPORTER_H__
#define __INSTRUCTION_IMPORTER_H__

#include "Importer.hpp"

#ifndef __puma
#include "util/llvmdisassembler/LLVMDisassembler.hpp"
#endif


class InstructionImporter : public Importer {
#ifndef __puma
	llvm::OwningPtr<llvm::object::Binary> binary;
	llvm::OwningPtr<fail::LLVMDisassembler> disas;
#endif

public:
	virtual bool handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
								 const Trace_Event &ev);
	virtual bool handle_mem_event(fail::simtime_t curtime, instruction_count_t instr,
								  const Trace_Event &ev) {
		/* ignore on purpose */
		return true;
	}
};

#endif
