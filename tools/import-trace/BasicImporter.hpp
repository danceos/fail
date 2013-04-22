#ifndef __BASIC_IMPORTER_H__
#define __BASIC_IMPORTER_H__

#include "Importer.hpp"
#include "util/CommandLine.hpp"

class BasicImporter : public Importer {
public:
	virtual bool create_database();
	bool add_trace_event(instruction_count_t begin,
						 instruction_count_t end,
						 fail::simtime_t time_begin,
						 fail::simtime_t time_end,
						 const Trace_Event &event,
						 bool is_fake = false);
};

#endif
