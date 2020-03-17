#ifndef __REGISTER_IMPORTER_H__
#define __REGISTER_IMPORTER_H__

#include <set>
#include "util/CommandLine.hpp"
#include "Importer.hpp"

#if defined(BUILD_CAPSTONE_DISASSEMBLER)
#include "util/capstonedisassembler/CapstoneDisassembler.hpp"
#elif defined(BUILD_LLVM_DISASSEMBLER)
#include "util/llvmdisassembler/LLVMDisassembler.hpp"
#endif

class RegisterImporter : public Importer {
#if defined(BUILD_CAPSTONE_DISASSEMBLER)
	bool isDisassembled = false;
	std::unique_ptr<fail::CapstoneDisassembler> disas;
	fail::CapstoneToFailTranslator *m_ctof = 0;

	bool addRegisterTrace(fail::simtime_t curtime, instruction_count_t instr,
						  Trace_Event &ev,
						  const fail::CapstoneToFailTranslator::reginfo_t &info,
						  char access_type);

	fail::CommandLine::option_handle NO_GP, FLAGS, IP, NO_SPLIT;
	bool do_gp, do_flags, do_ip, do_split_registers;

	std::set<unsigned> m_register_ids;
	unsigned m_ip_register_id;

	// Data structures for recording failed LLVM -> FAIL* register mappings,
	// including occurrence counts in the trace (to give an estimate on the
	// impact) and instruction addresses (for debugging purposes).
	struct RegNotFound {
		uint64_t count = 0;
		std::set<fail::guest_address_t> address;
	};
	std::map<fail::CapstoneDisassembler::register_t, RegNotFound> m_regnotfound;
#elif defined(BUILD_LLVM_DISASSEMBLER)
	llvm::object::Binary *binary = 0;
	std::unique_ptr<fail::LLVMDisassembler> disas;
	fail::LLVMtoFailTranslator *m_ltof = 0;

	bool addRegisterTrace(fail::simtime_t curtime, instruction_count_t instr,
						  Trace_Event &ev,
						  const fail::LLVMtoFailTranslator::reginfo_t &info,
						  char access_type);

	fail::CommandLine::option_handle NO_GP, FLAGS, IP, NO_SPLIT;
	bool do_gp, do_flags, do_ip, do_split_registers;

	std::set<unsigned> m_register_ids;
	unsigned m_ip_register_id;

	// Data structures for recording failed LLVM -> FAIL* register mappings,
	// including occurrence counts in the trace (to give an estimate on the
	// impact) and instruction addresses (for debugging purposes).
	struct RegNotFound {
		uint64_t count = 0;
		std::set<fail::guest_address_t> address;
	};
	std::map<fail::LLVMDisassembler::register_t, RegNotFound> m_regnotfound;
#endif


public:
	RegisterImporter() : Importer(), do_gp(true), do_flags(false), do_ip(false),
						 do_split_registers(true), m_ip_register_id(0) {}
	/**
	 * Callback function that can be used to add command line options
	 * to the cmd interface
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

	virtual bool trace_end_reached();

	virtual void open_unused_ec_intervals() {
		/* empty, Memory Map has a different meaning in this importer */
	}

	void getAliases(std::deque<std::string> *aliases) {
		aliases->push_back("RegisterImporter");
		aliases->push_back("regs");
	}

};

#endif
