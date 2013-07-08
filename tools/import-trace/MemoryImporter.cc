#include "util/Logger.hpp"
#include "MemoryImporter.hpp"

using namespace fail;
static fail::Logger LOG("MemoryImporter");


bool MemoryImporter::handle_ip_event(simtime_t curtime, instruction_count_t instr, Trace_Event &ev) {
	return true;
}

bool MemoryImporter::handle_mem_event(simtime_t curtime, instruction_count_t instr,
									  Trace_Event &ev) {
	address_t from = ev.memaddr(), to = ev.memaddr() + ev.width();
	// Iterate over all accessed bytes
	// FIXME Keep complete trace information (access width)?
	//   advantages: may be used for pruning strategies, complete value would be visible; less DB entries
	//   disadvantages: may need splitting when width varies, lots of special case handling
	//   Probably implement this in a separate importer when necessary.
	for (address_t data_address = from; data_address < to; ++data_address) {
		// skip events outside a possibly supplied memory map
		if (m_mm && !m_mm->isMatching(data_address)) {
			continue;
		}
		margin_info_t left_margin = getOpenEC(data_address);
		margin_info_t right_margin;
		right_margin.time = curtime;
		right_margin.dyninstr = instr; // !< The current instruction
		right_margin.ip = ev.ip();

		// skip zero-sized intervals: these can occur when an instruction
		// accesses a memory location more than once (e.g., INC, CMPXCHG)
		// FIXME: look at timing instead?
		if (left_margin.dyninstr > right_margin.dyninstr) {
			continue;
		}

		// we now have an interval-terminating R/W event to the memaddr
		// we're currently looking at; the EC is defined by
		// data_address, dynamic instruction start/end, the absolute PC at
		// the end, and time start/end

		// pass through potentially available extended trace information
		ev.set_memaddr(data_address);
		ev.set_width(1); // exactly one byte
		if (!add_trace_event(left_margin, right_margin, ev)) {
			LOG << "add_trace_event failed" << std::endl;
			return false;
		}

		// next interval must start at next instruction; the aforementioned
		// skipping mechanism wouldn't work otherwise
		newOpenEC(data_address, curtime + 1, instr + 1, ev.ip());
	}
	return true;
}
