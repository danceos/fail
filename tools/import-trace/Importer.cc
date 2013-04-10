#include <sstream>
#include <iostream>
#include "Importer.hpp"
#include "util/Logger.hpp"

using namespace fail;

static Logger LOG("Importer");

bool Importer::init(const std::string &variant, const std::string &benchmark, Database *db) {
	this->db = db;
	m_variant_id = db->get_variant_id(variant, benchmark);
	if (!m_variant_id) {
		return false;
	}
	LOG << "Importing to variant " << variant << "/" << benchmark
		<< " (ID: " << m_variant_id << ")" << std::endl;
	return true;
}

bool Importer::clear_database() {
	std::stringstream ss;
	ss << "DELETE FROM trace WHERE variant_id = " << m_variant_id;

	bool ret = db->query(ss.str().c_str()) == 0 ? false : true;
	LOG << "deleted " << db->affected_rows() << " rows from trace table" << std::endl;
	return ret;
}

bool Importer::copy_to_database(fail::ProtoIStream &ps) {
	unsigned row_count = 0, row_count_fake = 0;
	// map for keeping one "open" EC for every address
	// (maps injection data address =>
	// dyn. instruction count / time information for equivalence class left margin)
	AddrLastaccessMap open_ecs;

	// time the trace started/ended
	// For now we just use the min/max occuring timestamp; for "sparse" traces
	// (e.g., only mem accesses, only a subset of the address space) it might
	// be a good idea to store this explicitly in the trace file, though.
	simtime_t time_trace_start = 0, curtime = 0;

	// instruction counter within trace
	instruction_count_t instr = 0;

	Trace_Event ev;

	while (ps.getNext(&ev)) {
		if (ev.has_time_delta()) {
			// record trace start
			// it suffices to do this once, the events come in sorted
			if (time_trace_start == 0) {
				time_trace_start = ev.time_delta();
				LOG << "trace start time: " << time_trace_start << std::endl;
			}
			// curtime also always holds the max time, provided we only get
			// nonnegative deltas
			assert(ev.time_delta() >= 0);
			curtime += ev.time_delta();
		}

		// instruction events just get counted
		if (!ev.has_memaddr()) {
			// new instruction
			// sanity check for overflow
			if (instr == (1LL << (sizeof(instr)*8)) - 1) {
				LOG << "error: instruction_count_t overflow, aborting at instr=" << instr << std::endl;
				return false;
			}
			instr++;
			continue;
		}

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
			margin_info_t right_margin;
			margin_info_t left_margin = open_ecs[data_address];
			// defaulting to 0 is not such a good idea, memory reads at the
			// beginning of the trace would get an unnaturally high weight:
			if (left_margin.time == 0) {
				left_margin.time = time_trace_start;
			}
			right_margin.time = curtime;
			right_margin.dyninstr = instr; // !< The current instruction
			right_margin.ip = ev.ip();

			// skip zero-sized intervals: these can occur when an instruction
			// accesses a memory location more than once (e.g., INC, CMPXCHG)
			// FIXME: look at timing instead?
			if (left_margin.dyninstr > right_margin.dyninstr) {
				continue;
			}

			ev.set_width(1);
			ev.set_memaddr(data_address);

			// we now have an interval-terminating R/W event to the memaddr
			// we're currently looking at; the EC is defined by
			// data_address, dynamic instruction start/end, the absolute PC at
			// the end, and time start/end
			if (!add_trace_event(left_margin, right_margin, ev)) {
				LOG << "add_trace_event failed" << std::endl;
				return false;
			}
			row_count ++;
			if (row_count % 10000 == 0) {
				LOG << "Inserted " << row_count << " trace events into the database" << std::endl;
			}

			// next interval must start at next instruction; the aforementioned
			// skipping mechanism wouldn't work otherwise
			open_ecs[data_address].dyninstr = instr + 1;
			open_ecs[data_address].time     = curtime  + 1;
			// FIXME: This should be the next IP, not the current, or?
			open_ecs[data_address].ip       = ev.ip();
		}
	}

	// Close all open intervals (right end of the fault-space) with fake trace
	// event.  This ensures we have a rectangular fault space (important for,
	// e.g., calculating the total SDC rate), and unknown memory accesses after
	// the end of the trace are properly taken into account: Either with a
	// "don't care" (a synthetic memory write at the right margin), or a "care"
	// (synthetic read), the latter resulting in more experiments to be done.
	for (AddrLastaccessMap::iterator lastuse_it = open_ecs.begin();
		lastuse_it != open_ecs.end(); ++lastuse_it) {

		Trace_Event fake_ev;
		fake_ev.set_memaddr(lastuse_it->first);
		fake_ev.set_width(1);
		fake_ev.set_accesstype(m_faultspace_rightmargin == 'R' ? fake_ev.READ : fake_ev.WRITE);

		margin_info_t left_margin, right_margin;
		left_margin = lastuse_it->second;

		// Why -1?	In most cases it does not make sense to inject before the
		// very last instruction, as we won't execute it anymore.  This *only*
		// makes sense if we also inject into parts of the result vector.  This
		// is not the case in this experiment, and with -1 we'll get a result
		// comparable to the non-pruned campaign.
		right_margin.dyninstr = instr - 1;
		right_margin.time     = curtime; // -1?
		right_margin.ip       = ev.ip(); // The last event in the log.
// #else
//		// EcosKernelTestCampaign only variant: fault space ends with the last FI experiment
//		FIXME probably implement this with cmdline parameter FAULTSPACE_CUTOFF
//		right_margin.dyninstr = instr_rightmost;
// #endif

		// zero-sized?	skip.
		// FIXME: look at timing instead?
		if (left_margin.dyninstr > right_margin.dyninstr) {
			continue;
		}

		if (!add_trace_event(left_margin, right_margin, fake_ev, true)) {
			LOG << "add_trace_event failed" << std::endl;
			return false;
		}
		++row_count_fake;
	}

	LOG << "trace duration: " << (curtime - time_trace_start) << " ticks" << std::endl;
	LOG << "Inserted " << row_count << " trace events (+" << row_count_fake
	    << " fake events) into the database" << std::endl;

	// sanity checks
	if (m_sanitychecks) {
		std::stringstream ss;
		bool all_ok = true;
		MYSQL_RES *res;

		// PC-based fault space rectangular, covered, and non-overlapping?
		// (same for timing-based fault space?)

		LOG << "Sanity check: EC overlaps ..." << std::flush;
		ss <<
			"SELECT t1.variant_id\n" //, v.variant, v.benchmark, t1.instr1, t1.instr2, t1.data_address, t2.instr1, t2.instr2
			"FROM trace t1\n"
			"JOIN variant v\n"
			"  ON v.id = t1.variant_id\n"
			"JOIN trace t2\n"
			"  ON t1.variant_id = t2.variant_id AND t1.data_address = t2.data_address\n"
			" AND (t1.instr1 != t2.instr1 OR t2.instr2 != t2.instr2)\n"
			" AND ((t1.instr1 >= t2.instr1 AND t1.instr1 <= t2.instr2)\n"
			"  OR (t1.instr2 >= t2.instr1 AND t1.instr2 <= t2.instr2)\n"
			"  OR (t1.instr1 < t2.instr1 AND t1.instr2 > t2.instr2))\n"
			"WHERE t1.variant_id = " << m_variant_id;

		res = db->query(ss.str().c_str(), true);
		ss.str("");
		ss.clear();
		if (res && mysql_num_rows(res) == 0) {
			std::cout << " OK" << std::endl;
		} else {
			std::cout << " FAILED: not all ECs are disjoint" << std::endl;
			// TODO: show a list of overlapping ECs?
			all_ok = false;
		}

		LOG << "Sanity check: FS row sum = total width ..." << std::flush;
		ss <<
			"SELECT t.variant_id, t.data_address,\n"
			" (SELECT (MAX(t2.instr2) - MIN(t2.instr1) + 1)\n"
			"  FROM trace t2\n"
			"  WHERE t2.variant_id = t.variant_id)\n"
			" -\n"
			" (SELECT SUM(t3.instr2 - t3.instr1 + 1)\n"
			"  FROM trace t3\n"
			"  WHERE t3.variant_id = t.variant_id AND t3.data_address = t.data_address)\n"
			" AS diff\n"
			"FROM trace t\n"
			"WHERE t.variant_id = " << m_variant_id << "\n"
			"GROUP BY t.variant_id, t.data_address\n"
			"HAVING diff != 0\n"
			"ORDER BY t.data_address\n";

		res = db->query(ss.str().c_str(), true);
		ss.str("");
		ss.clear();
		if (res && mysql_num_rows(res) == 0) {
			std::cout << " OK" << std::endl;
		} else {
			std::cout << " FAILED: MAX(instr2)-MIN(instr1)+1 == SUM(instr2-instr1+1) not true for all fault-space rows" << std::endl;
			// TODO: show a list of failing data_addresses?
			all_ok = false;
		}

		LOG << "Sanity check: Global min/max = FS row local min/max ..." << std::flush;
		ss <<
			"SELECT t.variant_id, local.data_address, global.min_instr, global.max_instr, local.min_instr, local.max_instr\n"
			"FROM trace t\n"
			"JOIN\n"
			" (SELECT t2.variant_id, MIN(instr1) AS min_instr, MAX(instr2) AS max_instr\n"
			"  FROM trace t2\n"
			"  GROUP BY t2.variant_id) AS global\n"
			" ON global.variant_id = t.variant_id\n"
			"JOIN\n"
			" (SELECT variant_id, data_address, MIN(instr1) AS min_instr, MAX(instr2) AS max_instr\n"
			"  FROM trace t3\n"
			"  GROUP BY t3.variant_id, t3.data_address) AS local\n"
			" ON local.variant_id = t.variant_id\n"
			"AND (local.min_instr != global.min_instr\n"
			"  OR local.max_instr != global.max_instr)\n"
			"WHERE t.variant_id = " << m_variant_id << "\n"
			"GROUP BY t.variant_id, local.data_address\n";

		res = db->query(ss.str().c_str(), true);
		ss.str("");
		ss.clear();
		if (res && mysql_num_rows(res) == 0) {
			std::cout << " OK" << std::endl;
		} else {
			std::cout << " FAILED: global MIN(instr1)/MAX(instr2) != row-local MIN(instr1)/MAX(instr2)" << std::endl;
			// TODO: show a list of failing data_addresses and global min/max?
			all_ok = false;
		}

		if (!all_ok) {
			return false;
		}
	}

	return true;
}
