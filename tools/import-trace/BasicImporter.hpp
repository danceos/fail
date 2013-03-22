#ifndef __BASIC_IMPORTER_H__
#define __BASIC_IMPORTER_H__

#include "Importer.hpp"

class BasicImporter : public Importer {
public:
	virtual bool create_database();
	bool add_trace_event(instruction_count_t begin,
						 instruction_count_t end,
						 const Trace_Event &event,
						 bool is_fake = false);
};

#endif
