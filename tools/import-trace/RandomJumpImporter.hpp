#ifndef __RANDOM_JUMP_IMPORTER_H__
#define __RANDOM_JUMP_IMPORTER_H__

#include <vector>
#include "util/CommandLine.hpp"
#include "Importer.hpp"

#if defined(BUILD_CAPSTONE_DISASSEMBLER)
#include "util/capstonedisassembler/CapstoneDisassembler.hpp"
#elif defined(BUILD_LLVM_DISASSEMBLER)
#include "util/llvmdisassembler/LLVMDisassembler.hpp"
#endif

class RandomJumpImporter : public Importer {
#if defined(BUILD_CAPSTONE_DISASSEMBLER)
	bool binary = false;
	std::unique_ptr<fail::CapstoneDisassembler> disas;
#elif defined(BUILD_LLVM_DISASSEMBLER)
	llvm::object::Binary *binary = 0;
	std::unique_ptr<fail::LLVMDisassembler> disas;
#endif

	fail::CommandLine::option_handle FROM, TO;

	fail::MemoryMap *m_mm_from, *m_mm_to;
	std::vector<fail::guest_address_t> m_jump_to_addresses;
public:
	RandomJumpImporter() : m_mm_from(0), m_mm_to(0) {}
	/**
	 * Callback function that can be used to add command line options
	 * to the campaign
	 */
	virtual bool cb_commandline_init();

protected:
	virtual bool handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
								 Trace_Event &ev);
	virtual bool handle_mem_event(fail::simtime_t curtime, instruction_count_t instr,
								  Trace_Event &ev) {
		/* ignore on purpose */
		return true;
	}

	virtual void open_unused_ec_intervals() {
		/* empty, Memory Map has a different meaning in this importer */
	}

	void getAliases(std::deque<std::string> *aliases) {
		aliases->push_back("RandomJumpImporter");
	}
};

#endif
