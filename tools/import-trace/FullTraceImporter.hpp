#ifndef __FULL_TRACE_IMPORTER_H__
#define __FULL_TRACE_IMPORTER_H__

#include "Importer.hpp"
#include "util/CommandLine.hpp"

/**
	The FullTraceImporter imports every dynamic ip-event from the trace into the fulltrace database table.
*/
class FullTraceImporter : public Importer {

protected:
	virtual bool handle_ip_event(fail::simtime_t curtime, instruction_count_t instr,
				Trace_Event &ev) override;
	virtual bool handle_mem_event(fail::simtime_t curtime, instruction_count_t instr,
				Trace_Event &ev) override;
	virtual bool create_database() override;
	virtual bool clear_database() override;

	virtual bool trace_end_reached() override;

public:
	virtual void getAliases(std::deque<std::string> *aliases) override {
		aliases->push_back("FullTraceImporter");
	}
};

#endif
