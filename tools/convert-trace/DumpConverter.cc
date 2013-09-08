#include <sstream>
#include <string>
#include "util/Logger.hpp"
#include "comm/TracePlugin.pb.h"
#include "DumpConverter.hpp"

using namespace fail;
using std::stringstream;
using std::endl;
using std::hex;
using std::dec;

static Logger LOG("DumpConverter", true);

// TODO: convert extended trace information, too
bool DumpConverter::convert()
{
	char buf[2048], dummy[1024];
	int64_t prev_cycle = 0, cycle;

	while (m_input.getline(buf, sizeof(buf))) {
		Trace_Event ev;
		// IP 1035c9 t=0
		// MEM R 123abc width 4 IP 1035c9 t=1
		// MEM W 123abc width 4 IP 1035c9 t=1

		std::stringstream ss(buf);
		std::string ev_type;
		ss >> ev_type;

		if (ev_type == "MEM") {
			std::string accesstype;
			uint64_t data_address;
			uint32_t data_width;
			ss >> accesstype >> hex >> data_address >> dummy >> dec >> data_width >> ev_type;
			if (!ss) {
				LOG << "input mismatch (1), input = " << buf << endl;
				continue;
			}
			ev.set_memaddr(data_address);
			ev.set_width(data_width);
			ev.set_accesstype(accesstype == "R" ? ev.READ : ev.WRITE);
		}
		assert(ev_type == "IP");

		uint64_t instr_address;
		ss >> hex >> instr_address >> dummy;
		if (!ss || dummy[0] != 't' || dummy[1] != '=') {
			LOG << "input mismatch (2), input = " << buf << endl;
			continue;
		}
		ev.set_ip(instr_address);
		ss.clear();
		ss.str(dummy + 2);
		ss >> dec >> cycle;
		if (!ss) {
			LOG << "input mismatch (3), input = " << buf << endl;
			continue;
		}

		if (cycle - prev_cycle > 0) {
			ev.set_time_delta(cycle - prev_cycle);
			prev_cycle = cycle;
		} else if (cycle - prev_cycle < 0) {
			LOG << "ignoring time discontinuity " << dec << prev_cycle << " -> " << cycle << endl;
			prev_cycle = cycle;
		}

		m_ps.writeMessage(&ev);
	}

	return true;
}
