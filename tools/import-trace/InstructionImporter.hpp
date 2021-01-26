#ifndef __INSTRUCTION_IMPORTER_H__
#define __INSTRUCTION_IMPORTER_H__

#include "Importer.hpp"

class InstructionImporter : public Importer {
	std::unique_ptr<Disassembler> m_disassembler;
	Disassembler::InstrMap* m_instr_map;

protected:
	virtual bool cb_initialize();

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
