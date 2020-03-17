#ifndef __ADVANCED_MEMORY_IMPORTER_H__
#define __ADVANCED_MEMORY_IMPORTER_H__

#include <vector>
#include <deque>
#include "MemoryImporter.hpp"

#if defined(BUILD_CAPSTONE_DISASSEMBLER)
#include "util/capstonedisassembler/CapstoneDisassembler.hpp"
#elif defined(BUILD_LLVM_DISASSEMBLER)
#include "util/llvmdisassembler/LLVMDisassembler.hpp"
#endif

/**
 * A MemoryImporter that additionally imports Relyzer-style conditional branch
 * history, instruction opcodes, and a virtual duration = time2 - time1 + 1
 * column (MariaDB 5.2+ only!) for fault-space pruning purposes.
 *
 * Initially this was implemented by directly passing through trace events to
 * the MemoryImporter, keeping a record of conditional jumps and opcodes, and
 * UPDATEing all inserted rows in a second pass when the MemoryImporter is
 * finished.
 *
 * Unfortunately, UPDATE is very slow, and keeping all information in memory
 * till the end doesn't scale indefinitely.  Therefore the implementation now
 * delays passing memory access events upwards to the MemoryImporter only until
 * enough branch history is aggregated, and taps into Importer's database
 * operations with a set of new virtual functions that are called downwards.
 */
class AdvancedMemoryImporter : public MemoryImporter {
#if defined(BUILD_CAPSTONE_DISASSEMBLER)
	bool isDisassembled = false;
	std::unique_ptr<fail::CapstoneDisassembler> disas;
#elif defined(BUILD_LLVM_DISASSEMBLER)
	llvm::object::Binary *binary = 0;
	std::unique_ptr<fail::LLVMDisassembler> disas;
#endif
	bool m_last_was_conditional_branch;
	fail::guest_address_t m_ip_jump_not_taken;
	std::vector<bool> branches_taken;
	struct DelayedTraceEntry {
		fail::simtime_t curtime;
		instruction_count_t instr;
		Trace_Event ev;
		unsigned opcode;
		unsigned branches_before;
	};
	std::deque<DelayedTraceEntry> delayed_entries;
	static const unsigned BRANCH_WINDOW_SIZE = 16; //!< increasing this requires changing the underlying data types

	unsigned m_cur_branchmask;

	void insert_delayed_entries(bool finalizing);

public:
	AdvancedMemoryImporter() : m_last_was_conditional_branch(false),
		m_ip_jump_not_taken(0), m_cur_branchmask(0) {}

protected:
	virtual std::string database_additional_columns();
	virtual void database_insert_columns(std::string& sql, unsigned& num_columns);
	virtual bool database_insert_data(Trace_Event &ev, std::stringstream& value_sql, unsigned num_columns, bool is_fake);
	virtual bool handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
		Trace_Event &ev);
	virtual bool handle_mem_event(fail::simtime_t curtime, instruction_count_t instr,
		Trace_Event &ev);
	virtual bool trace_end_reached();

	void getAliases(std::deque<std::string> *aliases) {
		aliases->push_back("AdvancedMemoryImporter");
	}
};

#endif
