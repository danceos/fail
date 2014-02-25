#ifndef __FULL_TRACE_IMPORTER_H__
#define __FULL_TRACE_IMPORTER_H__

#include "Importer.hpp"
#include "util/CommandLine.hpp"

/**
	The FullTraceImporter imports every dynamic ip-event from the trace into the database.
*/
class FullTraceImporter : public Importer {

protected:
	virtual bool handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
				Trace_Event &ev);
	virtual bool handle_mem_event(fail::simtime_t curtime, instruction_count_t instr,
				Trace_Event &ev);
	virtual bool create_database();
	virtual bool clear_database();
	using Importer::add_trace_event;
	virtual bool add_trace_event(margin_info_t &begin, margin_info_t &end,
				Trace_Event &event, bool is_fake = false);
	virtual bool trace_end_reached();

public:
	void getAliases(std::deque<std::string> *aliases) {
		aliases->push_back("FullTraceImporter");
	}
};

#endif
