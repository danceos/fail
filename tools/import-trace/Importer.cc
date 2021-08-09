#include <sstream>
#include <iostream>
#include <utility> // std::pair
#include "Importer.hpp"
#include "util/Logger.hpp"
#include "sal/faultspace/MemoryArea.hpp"

using namespace fail;

static Logger LOG("Importer");

bool Importer::init(const std::string &variant, const std::string &benchmark, Database *db) {
	this->db = db;
	m_variant_id = db->get_variant_id(variant, benchmark);
	if (!m_variant_id) {
		return false;
	}
	m_extended_trace_regs =
		m_arch.getRegisterSetOfType(RT_TRACE);
	LOG << "Importing to variant " << variant << "/" << benchmark
		<< " (ID: " << m_variant_id << ")" << std::endl;

	if (!cb_initialize())
		return false;

	return true;
}

bool Importer::create_database() {
	std::stringstream create_statement;
	create_statement << "CREATE TABLE IF NOT EXISTS trace ("
		"	variant_id int(11) NOT NULL,"
		"	instr1 int(10) unsigned NOT NULL,"
		"	instr1_absolute int(10) unsigned DEFAULT NULL,"
		"	instr2 int(10) unsigned NOT NULL,"
		"	instr2_absolute int(10) unsigned DEFAULT NULL,"
		"	time1 bigint(10) unsigned NOT NULL,"
		"	time2 bigint(10) unsigned NOT NULL,"
		"	data_address bigint(10) unsigned NOT NULL,"
		"	data_mask tinyint(3) unsigned NOT NULL,"
		"	accesstype enum('R','W') NOT NULL,";
	if (m_extended_trace) {
		create_statement << "   data_value int(10) unsigned NULL,";
		for (UniformRegisterSet::iterator it = m_extended_trace_regs->begin();
			it != m_extended_trace_regs->end(); ++it) {
			create_statement << " r" << (*it)->getId() << " int(10) unsigned NULL,";
			create_statement << " r" << (*it)->getId() << "_deref int(10) unsigned NULL,";
		}
	}
	create_statement << database_additional_columns();
	create_statement <<
		"	PRIMARY KEY (variant_id,data_address,instr2,data_mask)"
		") engine=MyISAM ";
	return db->query(create_statement.str().c_str());
}


bool Importer::clear_database() {
	std::stringstream ss;
	ss << "DELETE FROM trace WHERE variant_id = " << m_variant_id;

	bool ret = db->query(ss.str().c_str()) == 0 ? false : true;
	LOG << "deleted " << std::dec << db->affected_rows() << " rows from trace table" << std::endl;
	return ret;
}

bool Importer::sanitycheck(std::string check_name, std::string fail_msg, std::string sql)
{
	LOG << "Sanity check: " << check_name << " ..." << std::flush << std::endl;
	MYSQL_RES *res = db->query(sql.c_str(), true);

	if (res && mysql_num_rows(res) == 0) {
		LOG << " OK" << std::endl;
		return true;
	} else {
		LOG << " FAILED: " <<  std::endl << fail_msg << std::endl;
		LOG << " ERROR MSG: " << std::endl;
		MYSQL_ROW row;
		int num_fields = mysql_num_fields(res);
		while((row = mysql_fetch_row(res))) {
			std::stringstream this_row;
			for(int i = 0; i < num_fields;  ++i) {
				this_row << " " << row[i];
			}
			LOG << this_row.str() << std::endl;
		}
		return false;
	}
}

bool Importer::copy_to_database(ProtoIStream &ps) {
	// For now we just use the min/max occuring timestamp; for "sparse" traces
	// (e.g., only mem accesses, only a subset of the address space) it might
	// be a good idea to store this explicitly in the trace file, though.
	simtime_t curtime = 0;

	// instruction counter within trace
	instruction_count_t instr = 0;
	// instruction counter new memory access events belong to
	instruction_count_t instr_memaccess = 0;

	// the currently processed event
	Trace_Event ev;

	while (ps.getNext(&ev)) {
		if (ev.has_time_delta()) {
			// record trace start
			// it suffices to do this once, the events come in sorted
			if (m_time_trace_start == 0) {
				m_time_trace_start = ev.time_delta();
				LOG << "trace start time: " << m_time_trace_start << std::endl;
			}
			// curtime also always holds the max time, provided we only get
			// nonnegative deltas
			assert(ev.time_delta() >= 0);
			curtime += ev.time_delta();
		}

		// instruction event?
		if (!ev.has_memaddr()) {
			// new instruction
			// sanity check for overflow
			if (instr == (1LL << (sizeof(instr)*8)) - 1) {
				LOG << "error: instruction_count_t overflow, aborting at instr=" << instr << std::endl;
				return false;
			}

			/* Another instruction was executed, handle it in the
			   subclass */
			if (!handle_ip_event(curtime, instr, ev)) {
				LOG << "error: handle_ip_event() failed at instr=" << instr << std::endl;
				return false;
			}

			// all subsequent mem access events belong to this dynamic instr
			instr_memaccess = instr;
			instr++;
		} else {
			if (!handle_mem_event(curtime, instr_memaccess, ev)) {
				LOG << "error: handle_mem_event() failed at instr=" << instr_memaccess << std::endl;
				return false;
			}
		}
	}

	if (!trace_end_reached()) {
		LOG << "trace_end_reached() failed" << std::endl;
		return false;
	}

	// Why -1?	In most cases it does not make sense to inject before the
	// very last instruction, as we won't execute it anymore.  This *only*
	// makes sense if we also inject into parts of the result vector.  This
	// is not the case in this experiment, and with -1 we'll get a result
	// comparable to the non-pruned campaign.
	m_last_time = curtime;
	m_last_instr = instr - 1; // - 1?
	m_last_ip = ev.ip();      // The last event in the log

	/* Signal that the trace was completely imported */
	LOG << "trace duration: " << std::dec << (curtime - m_time_trace_start) << " ticks" << std::endl;
	LOG << "Inserted " << m_row_count << " real trace events into the database" << std::endl;

	if (m_cover_memorymap) {
		/* All addresses that were specified in the memory map get an open
		   EC */
		open_unused_ec_intervals();
	}

	/* Close all open EC intervals */
	if (!close_ec_intervals()) {
		return false;
	}

	// flush cache before sanity checks
	db->insert_multiple();

	// sanity checks
	if (m_sanitychecks) {
		std::stringstream ss;
		bool all_ok = true;

		// PC- and timing-based fault space non-overlapping, covered, and
		// rectangular?

		// non-overlapping (instr1/2)?
		ss <<
			"SELECT t1.variant_id,HEX(t1.data_address),t1.instr1,t1.instr2\n"
			"FROM trace t1\n"
			"JOIN variant v\n"
			"  ON v.id = t1.variant_id\n"
			"JOIN trace t2\n"
			"  ON t1.variant_id = t2.variant_id AND t1.data_address = t2.data_address AND t1.data_mask = t2.data_mask\n"
			" AND (t1.instr1 != t2.instr1 OR t2.instr2 != t2.instr2)\n"
			" AND ((t1.instr1 >= t2.instr1 AND t1.instr1 <= t2.instr2)\n"
			"  OR (t1.instr2 >= t2.instr1 AND t1.instr2 <= t2.instr2)\n"
			"  OR (t1.instr1 < t2.instr1 AND t1.instr2 > t2.instr2))\n"
			"WHERE t1.variant_id = " << m_variant_id;
		if (!sanitycheck("EC overlaps (instr1/2)",
			"not all ECs are disjoint", ss.str())) {
			all_ok = false;
		}
		ss.str("");

		// non-overlapping (time1/2)?
		ss <<
			"SELECT t1.variant_id, HEX(t1.data_address), t1.instr1, t1.instr2\n"
			"FROM trace t1\n"
			"JOIN variant v\n"
			"  ON v.id = t1.variant_id\n"
			"JOIN trace t2\n"
			"  ON t1.variant_id = t2.variant_id AND t1.data_address = t2.data_address AND t1.data_mask = t2.data_mask\n"
			" AND (t1.time1 != t2.time1 OR t2.time2 != t2.time2)\n"
			" AND ((t1.time1 >= t2.time1 AND t1.time1 <= t2.time2)\n"
			"  OR (t1.time2 >= t2.time1 AND t1.time2 <= t2.time2)\n"
			"  OR (t1.time1 < t2.time1 AND t1.time2 > t2.time2))\n"
			"WHERE t1.variant_id = " << m_variant_id;
		if (!sanitycheck("EC overlaps (time1/2)",
			"not all ECs are disjoint", ss.str())) {
			all_ok = false;
		}
		ss.str("");

		// covered (instr1/2)?
		ss <<
			"SELECT t.variant_id, HEX(t.data_address),\n"
			" (SELECT (MAX(t2.instr2) - MIN(t2.instr1) + 1)\n"
			"  FROM trace t2\n"
			"  WHERE t2.variant_id = t.variant_id)\n"
			" -\n"
			" (SELECT SUM(t3.instr2 - t3.instr1 + 1)\n"
			"  FROM trace t3\n"
			"  WHERE t3.variant_id = t.variant_id AND t3.data_address = t.data_address AND t3.data_mask = t.data_mask)\n"
			" AS diff\n"
			"FROM trace t\n"
			"WHERE t.variant_id = " << m_variant_id << "\n"
			"GROUP BY t.variant_id, t.data_address,t.data_mask\n"
			"HAVING diff != 0\n"
			"ORDER BY t.data_address\n";
		if (!sanitycheck("FS row sum = total width (instr1/2)",
			"MAX(instr2)-MIN(instr1)+1 == SUM(instr2-instr1+1) not true for all fault-space rows",
			ss.str())) {
			all_ok = false;
		}
		ss.str("");

		// covered (time1/2)?
		ss <<
			"SELECT t.variant_id, t.data_address,\n"
			" (SELECT (MAX(t2.time2) - MIN(t2.time1) + 1)\n"
			"  FROM trace t2\n"
			"  WHERE t2.variant_id = t.variant_id)\n"
			" -\n"
			" (SELECT SUM(t3.time2 - t3.time1 + 1)\n"
			"  FROM trace t3\n"
			"  WHERE t3.variant_id = t.variant_id AND t3.data_address = t.data_address AND t3.data_mask = t.data_mask)\n"
			" AS diff\n"
			"FROM trace t\n"
			"WHERE t.variant_id = " << m_variant_id << "\n"
			"GROUP BY t.variant_id, t.data_address, t.data_mask\n"
			"HAVING diff != 0\n"
			"ORDER BY t.data_address\n";
		if (!sanitycheck("FS row sum = total width (time1/2)",
			"MAX(time2)-MIN(time1)+1 == SUM(time2-time1+1) not true for all fault-space rows",
			ss.str())) {
			all_ok = false;
		}
		ss.str("");

		// rectangular (instr1/2)?
		ss <<
			"SELECT local.data_address, global.min_instr, global.max_instr, local.min_instr, local.max_instr\n"
			"FROM\n"
			" (SELECT MIN(instr1) AS min_instr, MAX(instr2) AS max_instr\n"
			"  FROM trace t2\n"
			"  WHERE variant_id = " << m_variant_id << "\n"
			"  GROUP BY t2.variant_id) AS global\n"
			"JOIN\n"
			" (SELECT data_address, MIN(instr1) AS min_instr, MAX(instr2) AS max_instr\n"
			"  FROM trace t3\n"
			"  WHERE variant_id = " << m_variant_id << "\n"
			"  GROUP BY t3.variant_id, t3.data_address, t3.data_mask) AS local\n"
			" ON (local.min_instr != global.min_instr\n"
			"  OR local.max_instr != global.max_instr)";
		if (!sanitycheck("Global min/max = FS row local min/max (instr1/2)",
			"global MIN(instr1)/MAX(instr2) != row-local MIN(instr1)/MAX(instr2)",
			ss.str())) {
			all_ok = false;
		}
		ss.str("");

		// rectangular (time1/2)?
		ss <<
			"SELECT local.data_address, global.min_time, global.max_time, local.min_time, local.max_time\n"
			"FROM\n"
			" (SELECT MIN(time1) AS min_time, MAX(time2) AS max_time\n"
			"  FROM trace t2\n"
			"  WHERE variant_id = " << m_variant_id << "\n"
			"  GROUP BY t2.variant_id) AS global\n"
			"JOIN\n"
			" (SELECT data_address, MIN(time1) AS min_time, MAX(time2) AS max_time\n"
			"  FROM trace t3\n"
			"  WHERE variant_id = " << m_variant_id << "\n"
			"  GROUP BY t3.variant_id, t3.data_address,t3.data_mask) AS local\n"
			" ON (local.min_time != global.min_time\n"
			"  OR local.max_time != global.max_time)";
		if (!sanitycheck("Global min/max = FS row local min/max (time1/2)",
			"global MIN(time1)/MAX(time2) != row-local MIN(time1)/MAX(time2)",
			ss.str())) {
			all_ok = false;
		}
		ss.str("");

		if (!all_ok) {
			return false;
		}
	}

	return true;
}

using margin_info_t = Importer::margin_info_t;
std::ostream& operator<<(std::ostream& os, margin_info_t& v) {
	os << std::hex << std::showbase;
	os << "{ instr=" << v.dyninstr << ", ip="  << v.ip
	   << ", mask=" << std::hex << (int)v.mask << ", syn=" << v.synthetic << "}";
	return os << std::dec << std::noshowbase;
}

std::vector<margin_info_t>
Importer::getLeftMargins(const FaultSpaceElement& fsp_elem,
						 simtime_t time, instruction_count_t dyninstr,
						 guest_address_t ip, uint8_t mask) {
	std::vector<margin_info_t> results;

	if(m_open_ecs.count(fsp_elem) == 0) {
		/* If this fault space element has never been before we must
		 * create a synthetic fallback margin, which contains all
		 * bits and will used, if no event inside the trace has ever touched
		 * a specific bit.
		 */
		m_open_ecs[fsp_elem].emplace_back(0, 0, m_time_trace_start, 0xFF, true);
	}

	// For a given FSE, we get the stack of all open left margins.
	auto& left_margins = m_open_ecs[fsp_elem];

	/* Within this stack, we go from the newsest to the oldest left
	   margin and look for all margins that match our mask.

	   While searching, we :
	    (1) delete the matching bits from the left margins
		(2) we delete the left margin, if it matched.
	 */

	assert(left_margins.size() > 0
		   && "For a touched FSE, at least one margin must exist");

	uint8_t found_bits = 0;
	for(auto it = left_margins.rbegin(); it != left_margins.rend(); ++it) {
		margin_info_t& margin = *it;
		uint8_t overlap = margin.mask & mask;

		// Sanity Check: There is at most one margin_info_t that matches our mask.
		assert((overlap & found_bits) == 0
			   && "Multiple margin_info_t matched our access mask");
		found_bits |= overlap;

		if (overlap) {
			// create left margin which contains the bits of margin
			// that are closed by this interval.
			results.emplace_back(margin, overlap);

			margin.mask ^= overlap; // delete bits

			if (margin.mask == 0) {
				left_margins.erase(std::next(it).base());
			}
		}
	}

	// Create a new margin for the current instruction, with the
	// requested access mask.
	left_margins.emplace_back(dyninstr+1, ip, time+1, mask);

	return results;
}

bool Importer::add_faultspace_element(simtime_t curtime, instruction_count_t instr,
									   const FaultSpaceElement &element,
									   uint8_t mask, char access_type, Trace_Event& origin) {
	LOG << std::hex << std::showbase
		<< "[IP=" << origin.ip() << "] "
		<< " working on element: " << element
		<< " with access: " << access_type
		<< " and mask "     << (int)mask << std::endl;

	auto left_margins = getLeftMargins(element,curtime,instr,origin.ip(),mask);

	for(auto& left_margin : left_margins) {
		margin_info_t right_margin(instr, origin.ip(), curtime, left_margin.mask);

		// skip zero-sized intervals: these can occur when an
		// instruction accesses a fault location more than once. This
		// for example happens, if memory is read and written (e.g.,
		// INC, CMPXCHG) or if --ip is given and the instruction also reads EIP
		if(left_margin.time > right_margin.time) {
			continue;
		}

		// pass through potentially available extended trace information
		fsp_address_t data_address = element.get_address();
		if(!add_trace_row(left_margin, right_margin, data_address, access_type, origin)) {
			LOG << "add_trace_row failed" << std::endl;
			return false;
		}
	}

	return true;
}

bool Importer::add_trace_row(margin_info_t& begin, margin_info_t &end,
							 const fsp_address_t data_address,
							 char access_type,
							 Trace_Event& origin,
							 bool known_outcome) {
	if (!m_import_write_ecs && access_type == 'W') {
		return true;
	}

	// insert extended trace info if configured and available
	bool extended = m_extended_trace && origin.has_trace_ext();

	std::string *insert_sql;
	unsigned *columns;
	static std::string insert_sql_basic;
	static unsigned columns_basic = 0;
	static std::string insert_sql_extended;
	static unsigned columns_extended = 0;
	insert_sql = extended ? &insert_sql_extended : &insert_sql_basic;
	columns = extended ? &columns_extended : &columns_basic;

	static unsigned num_additional_columns = 0;
	LOG << "add interval:" <<  begin << " -> " << end << std::endl;

	if (!insert_sql->size()) {
		std::stringstream sql;
		sql << "INSERT INTO trace (variant_id, instr1, instr1_absolute, instr2, instr2_absolute, time1, time2, "
			   "data_address, data_mask, accesstype";
		*columns = 10;
		if (extended) {
			sql << ", data_value";
			(*columns)++;
			for (UniformRegisterSet::iterator it = m_extended_trace_regs->begin();
				it != m_extended_trace_regs->end(); ++it) {
				sql << ", r" << (*it)->getId() << ", r" << (*it)->getId() << "_deref";
			}
		}

		// Ask specialized importers whether they want to INSERT additional
		// columns.
		std::string additional_columns;
		database_insert_columns(additional_columns, num_additional_columns);
		sql << additional_columns;

		sql << ") VALUES ";

		*insert_sql = sql.str();
	}

	// extended trace:
	// retrieve register / register-dereferenced values
	// map: register ID --> (value available? , value)
	std::map<int, std::pair<bool, uint32_t> > register_values;
	std::map<int, std::pair<bool, uint32_t> > register_values_deref;
	unsigned data_value = 0;
	const Trace_Event_Extended& ev_ext = origin.trace_ext();
	if (extended) {
		data_value = ev_ext.data();
		for (int i = 0; i < ev_ext.registers_size(); i++) {
			const Trace_Event_Extended_Registers& reg = ev_ext.registers(i);
			register_values[reg.id()] = std::pair<bool, uint32_t>(reg.has_value(), reg.value());
			register_values_deref[reg.id()] = std::pair<bool, uint32_t>(reg.has_value_deref(), reg.value_deref());
		}
	}

	unsigned mask = begin.mask;
	assert(begin.mask == end.mask);

	std::stringstream value_sql;
	value_sql << "("
		<< m_variant_id << ","
		<< begin.dyninstr << ",";
	if (begin.ip == 0 || known_outcome) {
		value_sql << "NULL,";
	} else {
		value_sql << begin.ip << ",";
	}
	value_sql << end.dyninstr << ",";
	if (end.ip == 0 || known_outcome) {
		value_sql << "NULL,";
	} else {
		value_sql << end.ip << ",";
	}
	value_sql << begin.time << ","
		<< end.time << ","
		<< data_address << ","
		<< mask << ","
		<< "'" << access_type << "',";

	if (extended) {
		value_sql << data_value << ",";

		unsigned i = 0;
		for (UniformRegisterSet::iterator it = m_extended_trace_regs->begin();
			it != m_extended_trace_regs->end(); ++it, ++i) {
			std::map<int, std::pair<bool, uint32_t> >::const_iterator val_it;

			val_it = register_values.find((*it)->getId());
			if (val_it != register_values.end() && val_it->second.first) {
				value_sql << val_it->second.second << ",";
			} else {
				value_sql << "NULL,";
			}

			val_it = register_values_deref.find((*it)->getId());
			if (val_it != register_values_deref.end() && val_it->second.first) {
				value_sql << val_it->second.second << ",";
			} else {
				value_sql << "NULL,";
			}
		}
	}

	// Ask specialized importers what concrete data they want to INSERT.
	if (num_additional_columns &&
		!database_insert_data(origin, value_sql, num_additional_columns, known_outcome)) {
		return false;
	}

	// replace trailing ",[^,]*$" with ")"
	std::string value_sql_str = value_sql.str();
	size_t comma_pos = value_sql_str.find_last_of(',');
	if (comma_pos == std::string::npos) {
		// should never happen
		comma_pos = value_sql_str.length();
		LOG << "internal error: no comma found in SQL value list" << std::endl;
	}
	value_sql_str.resize(comma_pos + 1);
	value_sql_str[comma_pos] = ')';

	if (!db->insert_multiple(insert_sql->c_str(), value_sql_str.c_str())) {
		LOG << "Database::insert_multiple() failed" << std::endl;
		LOG << "IP: " << std::hex << end.ip << std::endl;
		return false;
	}

	m_row_count ++;
	if (m_row_count % 10000 == 0) {
		LOG << "Inserted " << std::dec << m_row_count << " trace events into the database" << std::endl;
	}

	return true;
}

void Importer::open_unused_ec_intervals() {
	// Create an open EC for every entry in the memory map we didn't encounter
	// in the trace.  If we don't, non-accessed rows in the fault space will be
	// missing later (e.g., unused parts of the stack).  Without a memory map,
	// we just don't know the extents of our fault space; just guessing by
	// using the minimum and maximum addresses is not a good idea, we might
	// have large holes in the fault space.

	if (m_mm) {
		auto filter = [] (address_t a) -> bool { return true; };
		auto area = dynamic_cast<MemoryArea*>(&m_fsp.get_area("ram"));
		for (MemoryMap::iterator it = m_mm->begin(); it != m_mm->end(); ++it) {
			for (auto &element : area->translate(*it, *it, filter)) { // exactly one element
				if (m_open_ecs.count(element) == 0) {
					m_open_ecs[element].emplace_back(0, 0, m_time_trace_start, 0xFF, true);
				}
			}
		}
	}
}

bool Importer::close_ec_intervals() {
	unsigned row_count_real = m_row_count;

	// Close all open intervals (right end of the fault-space) with fake trace
	// event.  This ensures we have a rectangular fault space (important for,
	// e.g., calculating the total SDC rate), and unknown memory accesses after
	// the end of the trace are properly taken into account: Either with a
	// "don't care" (a synthetic memory write at the right margin), or a "care"
	// (synthetic read), the latter resulting in more experiments to be done.
	for (AddrLastaccessMap::iterator lastuse_it = m_open_ecs.begin();
		lastuse_it != m_open_ecs.end(); ++lastuse_it) {
		// iterate over each left_margin and close with a similarily
		// masked right margin unless the left margin is synthetic.
		std::vector<margin_info_t>& margins = lastuse_it->second;
		for(auto& left_margin: margins) {
			// don't close synthetic intervals
			// FIXME: this should probably be a function.
			if(left_margin.synthetic)
				continue;
			margin_info_t right_margin(m_last_instr, m_last_ip, m_last_time, left_margin.mask);
			// zero-sized?	skip.
			// FIXME: look at timing instead?
			if (left_margin.dyninstr > right_margin.dyninstr) {
				continue;
			}

			Trace_Event t;
			fsp_address_t data_address = lastuse_it->first.get_address();
			if (!add_trace_row(left_margin, right_margin, data_address, m_faultspace_rightmargin, t)) {
				LOG << "add_trace_row failed" << std::endl;
				return false;
			}
		}
	}

	LOG << "Inserted " << std::dec << (m_row_count - row_count_real) << " fake trace events into the database" << std::endl;
	return true;
}
