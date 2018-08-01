#include <string>
#include <sstream>

#include "util/Logger.hpp"
#include "util/Database.hpp"
#include "FullTraceImporter.hpp"

using namespace fail;
static fail::Logger LOG("FullTraceImporter");

Database *db;

bool FullTraceImporter::handle_ip_event(simtime_t curtime, instruction_count_t instr,
					Trace_Event &ev) {

	margin_info_t right_margin;
	right_margin.time = curtime;
	right_margin.dyninstr = instr; // !< The current instruction
	right_margin.ip = ev.ip();

	// pass through potentially available extended trace information
	if (!add_trace_event(right_margin, right_margin, ev)) {
		LOG << "add_trace_event failed" << std::endl;
		return false;
	}

	return true;
}

bool FullTraceImporter::handle_mem_event(simtime_t curtime, instruction_count_t instr,
					Trace_Event &ev) {
	return true;
}

bool FullTraceImporter::create_database() {
	std::stringstream create_statement;
	create_statement << "CREATE TABLE IF NOT EXISTS fulltrace ("
			"   variant_id int(11) NOT NULL,"
			"   instr int(10) unsigned NOT NULL,"
			"   instr_absolute int(10) unsigned DEFAULT NULL,"
			"       INDEX(instr),"
			"   PRIMARY KEY (variant_id,instr)"
			") engine=MyISAM ";
	return db->query(create_statement.str().c_str());
}

bool FullTraceImporter::clear_database() {
	std::stringstream ss;
	ss << "DELETE FROM fulltrace WHERE variant_id = " << m_variant_id;

	bool ret = db->query(ss.str().c_str()) == 0 ? false : true;
	LOG << "deleted " << db->affected_rows() << " rows from fulltrace table" << std::endl;
	return ret;
}

bool FullTraceImporter::add_trace_event(margin_info_t &begin, margin_info_t &end,
					Trace_Event &event, bool is_fake) {
	// Is event a fake mem-event?
	if (is_fake) {
		return true;
	}

	// INSERT group entry
	std::stringstream sql;
	sql << "(" << m_variant_id << "," << end.dyninstr << "," << end.ip << ")";

	bool ret =
		db->insert_multiple("INSERT INTO fulltrace (variant_id, instr, instr_absolute) VALUES ", sql.str().c_str());
	m_row_count++;
	if (m_row_count % 10000 == 0 && ret) {
		LOG << "Inserted " << std::dec << m_row_count << " trace events into the database" << std::endl;
	}

	return ret;
}

bool FullTraceImporter::trace_end_reached() {
	return db->insert_multiple();
}
