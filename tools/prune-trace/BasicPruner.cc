#include <sstream>
#include "BasicPruner.hpp"
#include "util/Logger.hpp"
static fail::Logger LOG ("BasicPruner");


bool BasicPruner::prune_all() {
	std::stringstream ss;

	ss << "INSERT INTO fsppilot (known_outcome, variant_id, instr1, instr2, data_address, fspmethod_id) "
		  "SELECT 0, variant_id, instr1, instr2, data_address, " << m_method_id << " "
		  "FROM trace "
		  "WHERE variant_id = " << m_variant_id << " AND accesstype = 'R'";
	if (!db->query(ss.str().c_str())) return false; 
	ss.str("");

	int rows = db->affected_rows();
	// single entry for known outcome (write access)
	ss << "INSERT INTO fsppilot (known_outcome, variant_id, instr1, instr2, data_address, fspmethod_id) "
		  "SELECT 1, variant_id, instr1, instr2, data_address, " << m_method_id << " "
		  "FROM trace "
		  "WHERE variant_id = " << m_variant_id << " AND accesstype = 'W' "
		  "LIMIT 1";
	if (!db->query(ss.str().c_str())) return false;
	ss.str("");
	rows += db->affected_rows();

	LOG << "created " << rows << " fsppilot entries" << std::endl;

	/* When we are in basic-left mode we use the left boundary of the
	   equivalence interval. Sine the current database scheme has no
	   instr2_absolute, we set this to NULL in the basic-left mode. */
	std::string injection_instr = this->use_instr1 ? "t.instr1" : "t.instr2";
	std::string injection_instr_absolute = this->use_instr1 ? "NULL" : "t.instr2_absolute";

	ss << "INSERT INTO fspgroup (variant_id, injection_instr, injection_instr_absolute, "
	   << "                      data_address, fspmethod_id, pilot_id) "
	   << "SELECT p.variant_id, " << injection_instr << ", " << injection_instr_absolute << ", p.data_address, p.fspmethod_id, p.id "
	   << "FROM fsppilot p "
	   << " JOIN trace t ON t.variant_id = p.variant_id AND t.instr2 = p.instr2"
	   << "            AND t.data_address = p.data_address "
	   << "WHERE known_outcome = 0 AND p.fspmethod_id = " << m_method_id << " AND p.variant_id = " << m_variant_id;

	if (!db->query(ss.str().c_str())) return false;
	ss.str("");

	rows = db->affected_rows();
	ss << "INSERT INTO fspgroup (variant_id, injection_instr, injection_instr_absolute, data_address, fspmethod_id, pilot_id) "
		"SELECT t.variant_id, "<< injection_instr << ", " << injection_instr_absolute <<", t.data_address, p.fspmethod_id, p.id "
		"FROM trace t "
		"JOIN fsppilot p "
		"ON t.variant_id = p.variant_id AND p.fspmethod_id = " << m_method_id << " AND p.known_outcome = 1 "
		"WHERE t.variant_id = " << m_variant_id << " AND t.accesstype = 'W'";
	if (!db->query(ss.str().c_str())) return false;
	ss.str("");
	rows += db->affected_rows();

	LOG << "created " << rows << " fspgroup entries" << std::endl;

	return true;
}
