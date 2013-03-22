#include <sstream>
#include <iostream>
#include "Importer.hpp"
#include "util/Logger.hpp"

using namespace fail;

extern Logger log;

void Importer::init(const std::string &variant, const std::string &benchmark, Database *db) {
	this->db = db;
	m_variant_id = db->get_variant_id(variant, benchmark);
	log << "Importing to variant " << variant << "/" << benchmark
		<< " (ID: " << m_variant_id << ")" << std::endl;
}

bool Importer::clear_database() {
	std::stringstream ss;
	ss << "DELETE FROM trace WHERE variant_id = " << m_variant_id;

	bool ret = db->query(ss.str().c_str()) == 0 ? false : true;
	log << "delted " << db->affected_rows() << " rows from trace table" << std::endl;
	return ret;
}

bool Importer::copy_to_database(fail::ProtoIStream &ps) {
	unsigned row_count = 0;
	// map for keeping one "open" EC for every address
	// (maps injection data address => equivalence class)
	AddrLastaccessMap open_ecs;

	// instruction counter within trace
	unsigned instr = 0;

	// "rightmost" instr where we did a FI experiment
	unsigned instr_rightmost = 0;

	Trace_Event ev;

	while (ps.getNext(&ev)) {
		// instruction events just get counted
		if (!ev.has_memaddr()) {
			// new instruction
			instr++;
			continue;
		}

		address_t from = ev.memaddr(), to = ev.memaddr() + ev.width();
		// Iterate over all accessed bytes
		for (address_t data_address = from; data_address < to; ++data_address) {

			int instr1 = open_ecs[data_address]; // defaults to 0 if nonexistent
			int instr2 = instr; // the current instruction

			// skip zero-sized intervals: these can occur when an instruction
			// accesses a memory location more than once (e.g., INC, CMPXCHG)
			if (instr1 > instr2) {
				continue;
			}

			ev.set_width(1);
			ev.set_memaddr(data_address);

			// we now have an interval-terminating R/W event to the memaddr
			// we're currently looking at; the EC is defined by
			// data_address [instr1, instr2] (instr_absolute)
			if (!add_trace_event(instr1, instr2, ev)) {
				log << "add_trace_event failed" << std::endl;
				return false;
			}
			row_count ++;
			if (row_count % 1000 == 0) {
				log << "Imported " << row_count << " traces into the database" << std::endl;
			}

			if (ev.accesstype() == ev.READ) {
				// FIXME this is broken: we must abort after the rightmost R
				// and must not allow Ws to find their way into the known
				// results
				instr_rightmost = instr2;
			}

			// next interval must start at next instruction; the aforementioned
			// skipping mechanism wouldn't work otherwise
			//lastuse_it->second = instr2 + 1;
			open_ecs[data_address] = instr2 + 1;
		}
	}

	log << "Inserted " << row_count << " traces into the database" << std::endl;

	// FIXME
	// close all open intervals (right end of the fault-space) with fake trace event
//		for (AddrLastaccessMap::iterator lastuse_it = open_ecs.begin();
//			 lastuse_it != open_ecs.end(); ++lastuse_it) {
//			address_t data_address = lastuse_it->first;
//			int instr1 = lastuse_it->second;

// #if 0
//			// Why -1?	In most cases it does not make sense to inject before the
//			// very last instruction, as we won't execute it anymore.  This *only*
//			// makes sense if we also inject into parts of the result vector.  This
//			// is not the case in this experiment, and with -1 we'll get a result
//			// comparable to the non-pruned campaign.
//			int instr2 = instr - 1;
// #else
//			// EcosKernelTestCampaign only variant: fault space ends with the last FI experiment
//			int instr2 = instr_rightmost;
// #endif
//			// zero-sized?	skip.
//			if (instr1 > instr2) {
//				continue;
//			}

//			add_trace_event(variant_id, instr1, instr2, 
//				data_address, 1, 'W', 0,
//				0, 0, 0, 0, 0, true);
//		}

//		fsp.fini();

	return true;
}

