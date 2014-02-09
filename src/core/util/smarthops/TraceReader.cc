#include "TraceReader.hpp"

#include <iostream>
#include <fstream>
#include <string>

using std::endl;
using fail::ProtoIStream;

namespace fail {

TraceReader::~TraceReader()
{
	delete ps;
	delete normal_stream;
	delete gz_stream;
}

std::istream& openStream(const char *input_file,
	std::ifstream& normal_stream, igzstream& gz_stream, Logger &out_stream) {
	normal_stream.open(input_file);
	if (!normal_stream) {
		out_stream << "FATAL ERROR: couldn't open " << input_file << endl;
		exit(-1);
	}
	unsigned char b1, b2;
	normal_stream >> b1 >> b2;

	if (b1 == 0x1f && b2 == 0x8b) {
		normal_stream.close();
		gz_stream.open(input_file);
		if (!gz_stream) {
			out_stream << "couldn't open " << input_file << endl;
			exit(-1);
		}
		return gz_stream;
	}

	normal_stream.seekg(0);

	return normal_stream;
}

bool TraceReader::openTraceFile(const char *filename, unsigned int num_inst)
{
	normal_stream = new std::ifstream();
	gz_stream = new igzstream();
	ps = new fail::ProtoIStream(&openStream(filename, *normal_stream, *gz_stream, m_log));

	m_max_num_inst = num_inst;

	return true;
}

bool TraceReader::getNextTraceEvents(trace_pos_t& trace_pos,
	std::vector<trace_event_tuple_t >& trace_events)
{
	// Stop after fixed number of instructions, if given as command line argument
	if ((m_max_num_inst > 0) && (m_current_position > m_max_num_inst)) {
		return false;
	}

	trace_pos = m_current_position;

	// Delivered trace_events vector does not have to be
	// empty, so it must be cleared
	trace_events.clear();

	if (m_current_position == 1) {
		if (!ps->getNext(&ev)) {
			return false;
		}
		ev_avail = true;
	}

	if (!ev_avail) {
		return false;
	}

	trace_events.push_back(trace_event_tuple_t(ev.ip(), ACCESS_NONE));
	ev_avail = false;

	// read possible memory accesses and the next instruction
	while (ps->getNext(&ev)) {
		if (!ev.has_memaddr()) {
			ev_avail = true;
			break;
		}

		// Add a trace_event for every byte in memory access.
		// This breaks down the calculations to multiple
		// memory accesses of length 1. No more complexity
		// is needed in hop calculations.
		if (ev.has_width()) {
			for (unsigned int i = 0; i < ev.width(); i++) {
				trace_events.push_back(
					trace_event_tuple_t(ev.memaddr() + i,
					ev.accesstype() == ev.READ ? ACCESS_READ : ACCESS_WRITE));
			}
		}
	}
	m_current_position++;
	return true;
}

} // end of namespace
