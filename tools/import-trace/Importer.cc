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
	m_extended_trace_regs =
		m_arch.getRegisterSetOfType(RT_TRACE);
	LOG << "Importing to variant " << variant << "/" << benchmark
		<< " (ID: " << m_variant_id << ")" << std::endl;
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
		"	data_address int(10) unsigned NOT NULL,"
		"	width tinyint(3) unsigned NOT NULL,"
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
		"	PRIMARY KEY (variant_id,data_address,instr2)"
		") engine=MyISAM ";
	return db->query(create_statement.str().c_str());
}


bool Importer::clear_database() {
	std::stringstream ss;
	ss << "DELETE FROM trace WHERE variant_id = " << m_variant_id;

	bool ret = db->query(ss.str().c_str()) == 0 ? false : true;
	LOG << "deleted " << db->affected_rows() << " rows from trace table" << std::endl;
	return ret;
}

bool Importer::copy_to_database(fail::ProtoIStream &ps) {
	// For now we just use the min/max occuring timestamp; for "sparse" traces
	// (e.g., only mem accesses, only a subset of the address space) it might
	// be a good idea to store this explicitly in the trace file, though.
	simtime_t curtime = 0;

	// instruction counter within trace
	instruction_count_t instr = 0;

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

		// instruction events just get counted
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

			instr++;
		} else {
			if (!handle_mem_event(curtime, instr, ev)) {
				LOG << "error: handle_mem_event() failed at instr=" << instr << std::endl;
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


	/* All addresses that were specified in the memory map get an open
	   EC */
	open_unused_ec_intervals();

	/* Close all open EC intervals */
	if (!close_ec_intervals()) {
		return false;
	}


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

bool Importer::add_trace_event(margin_info_t &begin, margin_info_t &end,
							   access_info_t &access, bool is_fake) {
	Trace_Event e;
	e.set_ip(end.ip);
	e.set_memaddr(access.data_address);
	e.set_width(access.data_width);
	e.set_accesstype(access.access_type == 'R' ? e.READ : e.WRITE);
	return add_trace_event(begin, end, e, is_fake);
}

bool Importer::add_trace_event(margin_info_t &begin, margin_info_t &end,
							   Trace_Event &event, bool is_fake) {
	if (!m_import_write_ecs && event.accesstype() == event.WRITE) {
		return true;
	}

	// insert extended trace info if configured and available
	bool extended = m_extended_trace && event.has_trace_ext();

	MYSQL_STMT **stmt;
	unsigned *columns;
	static MYSQL_STMT *stmt_basic = 0;
	static unsigned columns_basic = 0;
	static MYSQL_STMT *stmt_extended = 0;
	static unsigned columns_extended = 0;
	stmt = extended ? &stmt_extended : &stmt_basic;
	columns = extended ? &columns_extended : &columns_basic;

	static unsigned num_additional_columns = 0;

	if (!*stmt) {
		std::stringstream sql;
		sql << "INSERT INTO trace (variant_id, instr1, instr1_absolute, instr2, instr2_absolute, time1, time2, "
		       "data_address, width, accesstype";
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

		sql << ") VALUES (?";
		for (unsigned i = 1;
		     i < *columns +
		         (extended ? m_extended_trace_regs->count() * 2 : 0) +
		         num_additional_columns;
		     ++i) {
			sql << ",?";
		}
		sql << ")";

		*stmt = mysql_stmt_init(db->getHandle());
		if (mysql_stmt_prepare(*stmt, sql.str().c_str(), sql.str().length())) {
			LOG << "query '" << sql.str() << "' failed: " << mysql_error(db->getHandle()) << std::endl;
			return false;
		}
	}

	// extended trace:
	// retrieve register / register-dereferenced values
	std::map<int, uint32_t> register_values;
	std::map<int, uint32_t> register_values_deref;
	unsigned data_value = 0;
	const Trace_Event_Extended& ev_ext = event.trace_ext();
	if (extended) {
		data_value = ev_ext.data();
		for (int i = 0; i < ev_ext.registers_size(); i++) {
			const Trace_Event_Extended_Registers& reg = ev_ext.registers(i);
			register_values[reg.id()] = reg.value();
			register_values_deref[reg.id()] = reg.value_deref();
		}
	}

	// C99 / g++ extension VLA to the rescue:
	MYSQL_BIND bind[*columns + m_extended_trace_regs->count() * 2 + num_additional_columns];
	my_bool fake_null = is_fake;
	my_bool null = true, not_null = false;
	long unsigned accesstype_len = 1;

	unsigned data_address = event.memaddr();
	unsigned width = event.width();
	char accesstype = event.accesstype() == event.READ ? 'R' : 'W';

	// LOG << m_variant_id  << "-" << ":" << begin.dyninstr << ":" << end.dyninstr << "-" << data_address << " " << accesstype << std::endl;

	// sizeof works fine for VLAs
	memset(bind, 0, sizeof(bind));
	for (unsigned i = 0; i < *columns; ++i) {
		bind[i].buffer_type = MYSQL_TYPE_LONG;
		bind[i].is_unsigned = 1;
		switch (i) {
		case 0: bind[i].buffer = &m_variant_id; break;
		case 1: bind[i].buffer = &begin.dyninstr; break;
		case 2: bind[i].buffer = &begin.ip;
			bind[i].is_null = begin.ip == 0 ? &null : &fake_null;
			break;
		case 3: bind[i].buffer = &end.dyninstr; break;
		case 4: bind[i].buffer = &end.ip;
			bind[i].is_null = &fake_null; break;
		case 5: bind[i].buffer = &begin.time;
			bind[i].buffer_type = MYSQL_TYPE_LONGLONG;
			break;
		case 6: bind[i].buffer = &end.time;
			bind[i].buffer_type = MYSQL_TYPE_LONGLONG;
			break;
		case 7: bind[i].buffer = &data_address; break;
		case 8: bind[i].buffer = &width; break;
		case 9: bind[i].buffer = &accesstype;
			bind[i].buffer_type = MYSQL_TYPE_STRING;
			bind[i].buffer_length = accesstype_len;
			bind[i].length = &accesstype_len;
			break;
		// only visited in extended mode:
		case 10: bind[i].buffer = &data_value;
			bind[i].is_null = ev_ext.has_data() ? &not_null : &null; break;
		}
	}
	if (extended) {
		unsigned i = 0;
		for (UniformRegisterSet::iterator it = m_extended_trace_regs->begin();
			it != m_extended_trace_regs->end(); ++it, ++i) {
			assert(*columns + i*2 + 1 < sizeof(bind)/sizeof(*bind));
			bind[*columns + i*2    ].buffer_type = MYSQL_TYPE_LONG;
			bind[*columns + i*2    ].is_unsigned = 1;
			if (register_values.count((*it)->getId())) {
				bind[*columns + i*2    ].buffer = &register_values[(*it)->getId()];
			} else {
				bind[*columns + i*2    ].buffer = &width; // arbitrary
				bind[*columns + i*2    ].is_null = &null;
			}

			bind[*columns + i*2 + 1].buffer_type = MYSQL_TYPE_LONG;
			bind[*columns + i*2 + 1].is_unsigned = 1;
			if (register_values_deref.count((*it)->getId())) {
				bind[*columns + i*2 + 1].buffer = &register_values_deref[(*it)->getId()];
			} else {
				bind[*columns + i*2 + 1].buffer = &width; // arbitrary
				bind[*columns + i*2 + 1].is_null = &null;
			}
		}
	}

	// Ask specialized importers what concrete data they want to INSERT.
	if (num_additional_columns) {
		unsigned pos = *columns + (extended ? m_extended_trace_regs->count() * 2 : 0);
		if (!database_insert_data(event, bind + pos, num_additional_columns, is_fake)) {
			return false;
		}
	}

	if (mysql_stmt_bind_param(*stmt, bind)) {
		LOG << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(*stmt) << std::endl;
		return false;
	}
	if (mysql_stmt_execute(*stmt)) {
		LOG << "mysql_stmt_execute() failed: " << mysql_stmt_error(*stmt) << std::endl;
		LOG << "IP: " << std::hex << end.ip << std::endl;
		return false;
	}

	m_row_count ++;
	if (m_row_count % 10000 == 0) {
		LOG << "Inserted " << m_row_count << " trace events into the database" << std::endl;
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
		for (MemoryMap::iterator it = m_mm->begin(); it != m_mm->end(); ++it) {
			if (m_open_ecs.count(*it) == 0) {
				newOpenEC(*it, m_time_trace_start, 0, 0);
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

		access_info_t access;
		access.access_type  = m_faultspace_rightmargin;
		access.data_address = lastuse_it->first;
		access.data_width   = 1; // One Byte

		margin_info_t left_margin, right_margin;
		left_margin = lastuse_it->second;

		right_margin.dyninstr = m_last_instr;
		right_margin.time     = m_last_time; // -1?
		right_margin.ip       = m_last_ip; // The last event in the log.
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


		if (!add_trace_event(left_margin, right_margin, access, true)) {
			LOG << "add_trace_event failed" << std::endl;
			return false;
		}
	}

	LOG << "Inserted " << (m_row_count - row_count_real) << " fake trace events into the database" << std::endl;
	return true;
}
