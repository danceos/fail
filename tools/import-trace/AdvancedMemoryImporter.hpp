#ifndef __ADVANCED_MEMORY_IMPORTER_H__
#define __ADVANCED_MEMORY_IMPORTER_H__

#include <vector>
#include "MemoryImporter.hpp"

#include "util/llvmdisassembler/LLVMDisassembler.hpp"

/**
 * A MemoryImporter that additionally imports Relyzer-style conditional branch
 * history, instruction opcodes, and a virtual duration = time2 - time1 + 1
 * column (MariaDB 5.2+ only!) for fault-space pruning purposes.
 */
class AdvancedMemoryImporter : public MemoryImporter {
	llvm::OwningPtr<llvm::object::Binary> binary;
	llvm::OwningPtr<fail::LLVMDisassembler> disas;
	bool m_last_was_conditional_branch;
	fail::guest_address_t m_ip_jump_not_taken;
	std::vector<bool> branches_taken;
	struct TraceEntry {
		unsigned instr2;
		uint64_t data_address;
		unsigned data_width;
		unsigned opcode;
		unsigned branches_before;
	};
	std::vector<TraceEntry> update_entries;

public:
	AdvancedMemoryImporter() : m_last_was_conditional_branch(false) {}
	virtual std::string database_additional_columns();
	virtual bool handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
		Trace_Event &ev);
	virtual bool handle_mem_event(fail::simtime_t curtime, instruction_count_t instr,
		Trace_Event &ev);
	virtual bool finalize();
};

#endif
