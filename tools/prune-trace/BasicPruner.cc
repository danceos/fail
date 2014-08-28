#include <sstream>
#include "BasicPruner.hpp"
#include "util/Logger.hpp"
static fail::Logger LOG ("BasicPruner");


bool BasicPruner::prune_all() {
	std::stringstream ss;

	/* When we are in basic-left mode we use the left boundary of the
	   equivalence interval. Since the current database scheme has no
	   instr1_absolute, we set this to NULL in the basic-left mode. */
	// FIXME "basic-left mode" doesn't make any sense; injections at instr1 and
	// at instr2 are completely equivalent.
	std::string injection_instr = this->use_instr1 ? "instr1" : "instr2";
	std::string injection_instr_absolute = this->use_instr1 ? "instr1_absolute" : "instr2_absolute";

	ss << "INSERT INTO fsppilot (known_outcome, variant_id, instr2, injection_instr, injection_instr_absolute, data_address, data_width, fspmethod_id) "
		  "SELECT 0, variant_id, instr2, " << injection_instr << ", " << injection_instr_absolute << ", "
		  "  data_address, width, " << m_method_id << " "
		  "FROM trace "
		  "WHERE variant_id IN (" << m_variants_sql << ") AND accesstype = 'R'";
	if (!db->query(ss.str().c_str())) return false;
	ss.str("");

	int rows = db->affected_rows();

	// for each variant:
	for (std::vector<fail::Database::Variant>::const_iterator it = m_variants.begin();
		it != m_variants.end(); ++it) {
		// single entry for known outcome (write access)
		ss << "INSERT INTO fsppilot (known_outcome, variant_id, instr2, injection_instr, injection_instr_absolute, data_address, data_width, fspmethod_id) "
			  "SELECT 1, variant_id, instr2, " << injection_instr << ", " << injection_instr_absolute << ", "
			  "  data_address, width, " << m_method_id << " "
			  "FROM trace "
			  "WHERE variant_id = " << it->id << " AND accesstype = 'W' "
			  "ORDER BY instr2 ASC "
			  "LIMIT 1";
		if (!db->query(ss.str().c_str())) return false;
		ss.str("");
		rows += db->affected_rows();
	}

	LOG << "created " << rows << " fsppilot entries" << std::endl;

	ss << "INSERT INTO fspgroup (variant_id, instr2, data_address, fspmethod_id, pilot_id) "
	   << "SELECT STRAIGHT_JOIN p.variant_id, p.instr2, p.data_address, p.fspmethod_id, p.id "
	   << "FROM fsppilot p "
	   << "JOIN trace t ON t.variant_id = p.variant_id AND t.instr2 = p.instr2"
	   << "            AND t.data_address = p.data_address "
	   << "WHERE known_outcome = 0 AND p.fspmethod_id = " << m_method_id << " "
	   << "AND p.variant_id IN (" << m_variants_sql << ")";

	if (!db->query(ss.str().c_str())) return false;
	ss.str("");

	rows = db->affected_rows();
	ss << "INSERT INTO fspgroup (variant_id, instr2, data_address, fspmethod_id, pilot_id) "
		"SELECT STRAIGHT_JOIN t.variant_id, t.instr2, t.data_address, p.fspmethod_id, p.id "
		"FROM fsppilot p "
		"JOIN trace t "
		"ON t.variant_id = p.variant_id AND p.fspmethod_id = " << m_method_id << " AND p.known_outcome = 1 "
		"WHERE t.variant_id IN (" << m_variants_sql << ") AND t.accesstype = 'W'";
	if (!db->query(ss.str().c_str())) return false;
	ss.str("");
	rows += db->affected_rows();

	LOG << "created " << rows << " fspgroup entries" << std::endl;

	return true;
}
