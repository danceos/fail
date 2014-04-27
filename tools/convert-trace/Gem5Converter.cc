#include <sstream>
#include <string>
#include "util/Logger.hpp"
#include "comm/TracePlugin.pb.h"
#include "Gem5Converter.hpp"

using namespace fail;
using std::stringstream;
using std::endl;
using std::hex;
using std::dec;

static Logger LOG("Gem5Converter", true);

bool Gem5Converter::convert()
{
	char buf[2048], dummy[2048];
	int64_t prev_cycle = 0, cycle;
	uint64_t cur_ip = 0;
	bool ifetch_seen = false;

	while (m_input.getline(buf, sizeof(buf))) {
		Trace_Event ev;
		// reset ifetch_seen in case we're getting multiple concatenated gem5 traces
		if (strstr(buf, "http://gem5.org")) {
			ifetch_seen = false;
			continue;
		}
		if (!strstr(buf, "system.physmem") || strstr(buf, "access wrote")) {
			continue;
		}
		//    5000: system.physmem: access wrote 4 bytes to address f7ee8
		//    5000: system.physmem: Write of size 4 on address 0xf7ee8 data 0x61190
		//    6000: system.physmem: IFetch of size 4 on address 0xd58 data 0xe59f000c
		//    6000: system.physmem: Read of size 4 on address 0xd6c data 0x8e2c
		//  161000: system.physmem: 00000000  4c 69 6e 75 78 00 00 00  00 00 00 00 00 00 00 00   Linux

		std::stringstream ss(buf);
		std::string access_type;
		unsigned access_width;
		uint64_t access_address;
		ss >> dec >> cycle >> dummy >> dummy >> access_type
		   >> dummy >> dummy >> dec >> access_width
		   >> dummy >> dummy >> hex >> access_address;
		if (!ss) {
			if (ifetch_seen && access_type.c_str()[0] != '0') {
				LOG << "input mismatch, input = " << buf << endl;
			}
			continue;
		}

		if (cycle - prev_cycle > 0) {
			ev.set_time_delta(cycle - prev_cycle);
			prev_cycle = cycle;
		} else if (cycle - prev_cycle < 0) {
			LOG << "ignoring time discontinuity " << dec << prev_cycle << " -> " << cycle << endl;
			prev_cycle = cycle;
		}

		if (access_type == "IFetch") {
			ifetch_seen = true;
			cur_ip = access_address;
			ev.set_ip(cur_ip);
			//LOG << "ip = " << hex << cur_ip << endl;

		} else {
			bool is_read;
			if (access_type == "Read") {
				is_read = true;
			} else if (access_type == "Write") {
				is_read = false;
			} else if (access_type.c_str()[0] == '0') {
				continue;
			} else {
				LOG << "input mismatch, input = " << buf << endl;
				continue;
			}

			if (!ifetch_seen) {
				// skip all accesses before the first ifetch
				continue;
			}
			ev.set_ip(cur_ip);
			ev.set_memaddr(access_address);
			ev.set_width(access_width);
			ev.set_accesstype(is_read ? ev.READ : ev.WRITE);
			//cout << "ip = " << hex << cur_ip << " mem " << access_address << " " << access_width << " " << (is_read ? 'R' : 'W') << endl;
		}
		m_ps.writeMessage(&ev);
	}

	return true;
}
