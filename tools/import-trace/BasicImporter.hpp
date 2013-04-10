#ifndef __BASIC_IMPORTER_H__
#define __BASIC_IMPORTER_H__

#include "Importer.hpp"
#include "util/CommandLine.hpp"

class BasicImporter : public Importer {
public:
	virtual bool create_database();
	virtual bool add_trace_event(margin_info_t &begin, margin_info_t &end,
								 const Trace_Event &event, bool is_fake = false);
};

#endif
