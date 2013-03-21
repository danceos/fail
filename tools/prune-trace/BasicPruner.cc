#include <sstream>
#include "BasicPruner.hpp"
#include "util/Logger.hpp"
static fail::Logger log ("BasicPruner");


bool BasicPruner::prune_all() {
	std::stringstream ss;

	ss << "INSERT INTO fsppilot (known_outcome, variant_id, instr2, data_address, fspmethod_id) "
	      "SELECT 0, variant_id, instr2, data_address, " << m_method_id << " "
	      "FROM trace "
	      "WHERE variant_id = " << m_variant_id << " AND accesstype = 'R'";
	if (!db->query(ss.str().c_str())) return false; 
	ss.str("");

	int rows = db->affected_rows();
	// single entry for known outcome (write access)
	ss << "INSERT INTO fsppilot (known_outcome, variant_id, instr2, data_address, fspmethod_id) "
	      "SELECT 1, variant_id, instr2, data_address, " << m_method_id << " "
	      "FROM trace "
	      "WHERE variant_id = " << m_variant_id << " AND accesstype = 'W' "
	      "LIMIT 1";
	if (!db->query(ss.str().c_str())) return false;
	ss.str("");
	rows += db->affected_rows();

	log << "created " << rows << " fsppilot entries" << std::endl;

	ss << "INSERT INTO fspgroup (variant_id, instr2, data_address, fspmethod_id, pilot_id) "
	      "SELECT variant_id, instr2, data_address, fspmethod_id, id "
	      "FROM fsppilot "
	      "WHERE known_outcome = 0 AND fspmethod_id = " << m_method_id << " AND variant_id = " << m_variant_id;
	if (!db->query(ss.str().c_str())) return false;
	ss.str("");
	rows = db->affected_rows();
	ss << "INSERT INTO fspgroup (variant_id, instr2, data_address, fspmethod_id, pilot_id) "
	      "SELECT t.variant_id, t.instr2, t.data_address, p.fspmethod_id, p.id "
	      "FROM trace t "
	      "JOIN fsppilot p "
	      "ON t.variant_id = p.variant_id AND p.fspmethod_id = " << m_method_id << " AND p.known_outcome = 1 "
	      "WHERE t.variant_id = " << m_variant_id << " AND t.accesstype = 'W'";
	if (!db->query(ss.str().c_str())) return false;
	ss.str("");
	rows += db->affected_rows();

	log << "created " << rows << " fspgroup entries" << std::endl;

	return true;
}
