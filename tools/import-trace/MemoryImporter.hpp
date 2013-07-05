#ifndef __MEMORY_IMPORTER_H__
#define __MEMORY_IMPORTER_H__

#include "Importer.hpp"
#include "util/CommandLine.hpp"

class MemoryImporter : public Importer {

public:

	virtual bool handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
								 Trace_Event &ev);
	virtual bool handle_mem_event(fail::simtime_t curtime, instruction_count_t instr,
								  Trace_Event &ev);
};

#endif
