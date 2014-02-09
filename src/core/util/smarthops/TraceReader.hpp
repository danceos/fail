#ifndef __TRACE_READER_HPP
#define __TRACE_READER_HPP

#include <vector>
#include <iostream>

#include "sal/SALConfig.hpp"

#include "../ProtoStream.hpp"
#include "comm/TracePlugin.pb.h"

#include "../gzstream/gzstream.h"

#include "util/Logger.hpp"

namespace fail {

typedef enum {
	ACCESS_NONE,
	ACCESS_READ,
	ACCESS_WRITE,
	ACCESS_READORWRITE,	// some architectures can't distinguish read and write WPs in general (e.g. x86)
	ACCESS_CHECKPOINT,
} mem_access_type_e;

typedef uint32_t trace_pos_t;
typedef std::pair<address_t, mem_access_type_e> trace_event_tuple_t;

class TraceReader {
public:
	TraceReader() : m_current_position(1),
					ps(0),
					normal_stream(0),
					gz_stream(0),
					m_max_num_inst(0),
					ev_avail(false),
					m_log("TraceReader", false) {}

	~TraceReader();

	// Returns ACCESS_NONE in mem_access if current instruction does not access memory
	bool getNextTraceEvents(trace_pos_t& trace_pos,
		std::vector<trace_event_tuple_t >& trace_events);

	bool openTraceFile(const char *filename, unsigned int num_inst = 0);
private:
	unsigned int m_current_position;
	ProtoIStream* ps;
	std::ifstream *normal_stream;
	igzstream *gz_stream;
	unsigned int m_max_num_inst;
	Trace_Event ev;
	bool ev_avail;

	Logger m_log;
};

} // end of namespace

#endif //__TRACE_HPP
