#ifndef __REGISTER_IMPORTER_H__
#define __REGISTER_IMPORTER_H__

#include <set>
#include "util/CommandLine.hpp"
#include "Importer.hpp"
#include "sal/faultspace/RegisterArea.hpp"



class RegisterImporter : public Importer {
	// Importer Options
	fail::CommandLine::option_handle NO_GP, FLAGS, IP, NO_SPLIT;

	std::set<unsigned> m_register_ids;

	fail::RegisterArea *m_area;


	std::unique_ptr<Disassembler> m_disassembler;
	Disassembler::InstrMap* m_instr_map;
	RegisterTranslator* m_translator;

	bool m_import_ip;
	fail::RegisterView m_ip_register;

	// Data structures for recording failed {LLVM,Capstone} -> FAIL*
	// register mappings, including occurrence counts in the trace (to
	// give an estimate on the impact) and instruction addresses (for
	// debugging purposes).
	struct RegNotFound {
		uint64_t count = 0;
		std::set<fail::guest_address_t> address;
	};
	std::map<Disassembler::register_t, RegNotFound> m_regnotfound;

	bool addRegisterTrace(fail::simtime_t curtime, instruction_count_t instr,
						  Trace_Event &ev,
						  fail::RegisterView info,
						  char access_type);


public:
	RegisterImporter() : m_instr_map(0), m_translator(0),  m_import_ip(false) {
		m_area = dynamic_cast<fail::RegisterArea *>(&m_fsp.get_area("register"));
		assert(m_area != nullptr && "RegisterImporter failed to get a RegisterArea from the fault space description");
	}
	/**
	 * Callback function that can be used to add command line options
	 * to the cmd interface
	 */
	virtual bool cb_commandline_init() override;

	/**
	 * Callback function that can be used to initialize the importer
	 * before the trace is consumed.
	 */
	virtual bool cb_initialize() override;

protected:
	virtual bool handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
								 Trace_Event &ev) override;
	virtual bool handle_mem_event(fail::simtime_t curtime, instruction_count_t instr,
								  Trace_Event &ev) override {
		/* ignore on purpose */
		return true;
	}

	virtual bool trace_end_reached() override;

	virtual void open_unused_ec_intervals() override {
		/* empty, Memory Map has a different meaning in this importer */
	}

	void getAliases(std::deque<std::string> *aliases) override{
		aliases->push_back("RegisterImporter");
		aliases->push_back("regs");
	}

};

#endif
